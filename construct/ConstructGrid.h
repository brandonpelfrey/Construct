#ifndef ConstructGrid_h
#define ConstructGrid_h
#include "construct/ConstructField.h"
#include "construct/ConstructDomain.h"
#include <iostream>
namespace Construct {

template<typename T>
struct ConstructGrid : public ConstructFieldNode<T> {
	//! Domain definition for this grid
	Domain domain;

	//! The field which should be queried if something
	//! tries to access outside of this grid
	typename ConstructFieldNode<T>::ptr outside_field;
	
	//! Grid storage
	T *data;

	ConstructGrid(Domain domain, typename ConstructFieldNode<T>::ptr outside_field) 
	: domain(domain), outside_field(outside_field) { 
		data = new T[domain.res[0] * domain.res[1] * domain.res[2]];	
	}

	~ConstructGrid() { delete[] data; }

	//! 1D array index of a 3D lattice index 
	inline int index(int i, int j, int k) const 
	{	return (k * domain.res[1] + j) * domain.res[0] + i; }

	//! Return data located at lattice point (i,j,k)
	inline T get(int i, int j, int k) const {
		if(!domain.inside(i,j,k))
			return outside_field->eval(domain.position(i,j,k));
		else
			return data[ index(i,j,k) ];
	}

	inline void set(int i, int j, int k, const T& value) {
		data[index(i,j,k)] = value;
	}

	void bakeData(typename ConstructFieldNode<T>::ptr source) {
		#pragma omp parallel for
		for(int i=0;i<domain.res[0];++i)
		for(int j=0;j<domain.res[1];++j)
		for(int k=0;k<domain.res[2];++k) {
			Vec3 x = domain.position(i,j,k);
			data[index(i,j,k)] = source->eval(x);
		}
	}

	T eval(const Vec3& x) const {
		Vec3 relative = (x - domain.bmin).cwiseProduct(domain.Hinverse);

		int i = (int)floor(relative[0]);
		int j = (int)floor(relative[1]);
		int k = (int)floor(relative[2]);
		
		int i1 = i+1, j1 = j+1, k1 = k+1;
		const Vec3 w = relative - Vec3(i,j,k);
		const Vec3 w1 = Vec3(1,1,1) - w;

		return
			w1[0] * w1[1] * w1[2] * get(i ,j ,k ) +
			w[0]  * w1[1] * w1[2] * get(i1,j ,k ) +
			w1[0] * w[1]  * w1[2] * get(i ,j1,k ) +
			w[0]  * w[1]  * w1[2] * get(i1,j1,k ) +
			w1[0] * w1[1] * w[2]  * get(i ,j ,k1) +
			w[0]  * w1[1] * w[2]  * get(i1,j ,k1) +
			w1[0] * w[1]  * w[2]  * get(i ,j1,k1) +
			w[0]  * w[1]  * w[2]  * get(i1,j1,k1);
	}

	typename FieldInfo<T>::GradType grad(const Vec3& x) const {
		throw std::logic_error("Gradient of Matrix Field not supported");
		return FieldInfo<typename FieldInfo<T>::GradType>::Zero(); 
	}

	//! Divergence-Free projection. Only specialized for vector fields 
	void divFree(int iterations) { }
 
	//! Load a gridded field from disk 
	void load(const char* path) { 
    FILE *f = fopen(path, "rb");
		Domain newdomain;

    if(3 != fread(newdomain.res+0, sizeof(int), 3, f) ) { }
    if(1 != fread(&newdomain.bmin, sizeof(Vec3), 1, f)) { }
    if(1 != fread(&newdomain.bmax, sizeof(Vec3), 1, f)) { }
	
		domain = newdomain;
    const unsigned int N = domain.res[0] * domain.res[1] * domain.res[2];
    delete[] data;
    data = NULL;
    data = new T[N];
    if(N != fread(data, sizeof(T), N, f)) { }
    fclose(f);
  }

	//! Save a gridded field to disk
  void save(const char* path) {
    FILE *f = fopen(path, "wb");
    int N = domain.res[0] * domain.res[1] * domain.res[2];
    fwrite(domain.res, 3, sizeof(int), f);
    fwrite(&domain.bmin, 1, sizeof(Vec3), f);
    fwrite(&domain.bmax, 1, sizeof(Vec3), f);
    fwrite(data, N, sizeof(T), f);
    fclose(f);
  }
};

// Grid gradient operators
template<> Vec3 ConstructGrid<real>::grad(const Vec3& x) const
{ 
	const Vec3 &dx(domain.H);
	Vec3 result;
	result[0] = (eval(x + Vec3(dx[0],0,0)) - eval(x - Vec3(dx[0],0,0))) / (2 * dx[0]);
	result[1] = (eval(x + Vec3(0,dx[1],0)) - eval(x - Vec3(0,dx[1],0))) / (2 * dx[1]);
	result[2] = (eval(x + Vec3(0,0,dx[2])) - eval(x - Vec3(0,0,dx[2]))) / (2 * dx[2]);
	return result;
}
template<> Mat3 ConstructGrid<Vec3>::grad(const Vec3& x) const
{ 
	const Vec3 &dx(domain.H);
	Mat3 result;
	result.row(0) = (eval(x + Vec3(dx[0],0,0)) - eval(x - Vec3(dx[0],0,0))) / (2 * dx[0]);
	result.row(1) = (eval(x + Vec3(0,dx[1],0)) - eval(x - Vec3(0,dx[1],0))) / (2 * dx[1]);
	result.row(2) = (eval(x + Vec3(0,0,dx[2])) - eval(x - Vec3(0,0,dx[2]))) / (2 * dx[2]);
	return result;
}

// Divergence-Free Projection
// Use Helmholtz-Hodge decomposition to compute div-free component of a vector field
template<> void ConstructGrid<Vec3>::divFree(int iterations) {
	ConstructGrid<real> p(domain, constant(static_cast<real>(0)).node);
	ConstructGrid<real> pnew(domain, constant(static_cast<real>(0)).node);
	ConstructGrid<real> divergence(domain, constant(static_cast<real>(0)).node);
	p.bakeData(ScalarField(static_cast<real>(0)).node);

	// Set no flux for velocity
#pragma omp parallel for
    for(int i=0;i<domain.res[0];++i)
    for(int j=0;j<domain.res[1];++j)
    for(int k=0;k<domain.res[2];++k) {
      if(i==0 || i==domain.res[0]-1) set(i,j,k, get(i,j,k).cwiseProduct(Vec3(0,1,1)));
      if(j==0 || j==domain.res[1]-1) set(i,j,k, get(i,j,k).cwiseProduct(Vec3(1,0,1)));
      if(k==0 || k==domain.res[2]-1) set(i,j,k, get(i,j,k).cwiseProduct(Vec3(1,1,0)));
    }

	// Compute divergence of non-boundary cells
#pragma omp parallel for
    for(int i=1;i<domain.res[0]-1;++i)
    for(int j=1;j<domain.res[1]-1;++j)
    for(int k=1;k<domain.res[2]-1;++k) {
      Vec3 V = get(i,j,k);
      float D = get(i+1,j,k)[0] + get(i,j+1,k)[1] + get(i,j,k+1)[2];
      D -= V[0] + V[1] + V[2];
      divergence.set(i,j,k, D /** domain.Hinverse[0]*/ ); // ASSUMED CUBIC CELLS!
    }

	// Jacobi Iterations
	int iter;
	for(iter=0; iter<iterations; ++iter) {
#pragma omp parallel for
      for(int i=0;i<domain.res[0];++i)
      for(int j=0;j<domain.res[1];++j)
      for(int k=0;k<domain.res[2];++k) {
        if(i==0) { pnew.set(i,j,k, p.get(i+1,j,k)); continue; }
        if(j==0) { pnew.set(i,j,k, p.get(i,j+1,k)); continue; }
        if(k==0) { pnew.set(i,j,k, p.get(i,j,k+1)); continue; }
        if(i==domain.res[0]-1) { pnew.set(i,j,k, p.get(domain.res[0]-2,j,k)); continue; }
        if(j==domain.res[1]-1) { pnew.set(i,j,k, p.get(i,domain.res[1]-2,k)); continue; }
        if(k==domain.res[2]-1) { pnew.set(i,j,k, p.get(i,j,domain.res[2]-2)); continue; }

        const real h2 = 1;//domain.H[0]*domain.H[0];

        real P = -h2 * divergence.get(i,j,k);
        P += p.get(i+1,j,k);
        P += p.get(i-1,j,k);
        P += p.get(i,j+1,k);
        P += p.get(i,j-1,k);
        P += p.get(i,j,k+1);
        P += p.get(i,j,k-1);

        real newp = P / 6;
        pnew.set(i,j,k, newp);
      }

	// Assign computed values over old ones
#pragma omp parallel for
      for(int i=0;i<domain.res[0];++i)
      for(int j=0;j<domain.res[1];++j)
      for(int k=0;k<domain.res[2];++k) {
        p.set(i,j,k, pnew.get(i,j,k));
      }
	}

	// Subtract gradient of "pressure"
#pragma omp parallel for
    for(int i=1;i<domain.res[0]-1;++i)
    for(int j=1;j<domain.res[1]-1;++j)
    for(int k=1;k<domain.res[2]-1;++k) {
      Vec3 V = get(i,j,k);
      V[0] -= (p.get(i,j,k) - p.get(i-1,j,k)) ;//* domain.Hinverse[0];
      V[1] -= (p.get(i,j,k) - p.get(i,j-1,k)) ;//* domain.Hinverse[1];
      V[2] -= (p.get(i,j,k) - p.get(i,j,k-1)) ;//* domain.Hinverse[2];
      set(i,j,k,V);
    }
}

inline VectorField divFree(VectorField field, const Domain& domain, int iterations=30) {
	// TODO: build in isGridded() check and create shortcut for fields that are already grids
	// so we don't waste time writing them to a grid a second time

	ConstructGrid<Vec3> *grid = new ConstructGrid<Vec3>(domain, constant(Vec3(0,0,0)).node);
	grid->bakeData(field.node);
	grid->divFree(iterations);
	return VectorField(grid);
}


template<typename T>
inline Field<T> writeToGrid(Field<T> field, Field<T> outside, Domain domain) {
	ConstructGrid<T> *grid = new ConstructGrid<T>(domain, outside.node);
  grid->bakeData(field.node);
  return Field<T>(grid);
}

template<typename T>
inline Field<T> loadGriddedField(const char *path, Field<T> outside) {
	const Domain fake(1,1,1, Vec3(-1,-1,-1), Vec3(1,1,1));
	ConstructGrid<T> *grid = new ConstructGrid<T>(fake, outside.node);
	grid->load(path);
	return Field<T>(grid);
}

template<typename T>
inline void saveGriddedField(const char *path, Field<T> field, Domain domain) {
	// TODO: If this is a grid already (need introspection/reflection)
	//       then save: Don't write to grid twice.
	Field<T> zero(FieldInfo<T>::Zero());
	ConstructGrid<T> grid = ConstructGrid<T>(domain, zero.node);
	grid.bakeData(field.node);
	grid.save(path);
}

};
#endif
