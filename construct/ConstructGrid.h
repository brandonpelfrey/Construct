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
	
  //! Guaranteed Safe Get (For when there is no possibility of accessing outside)
  inline T gets(int i, int j, int k) const {
		return data[ index(i,j,k) ];
	}

	inline void set(int i, int j, int k, const T& value) {
		data[index(i,j,k)] = value;
	}

	void bakeData(typename ConstructFieldNode<T>::ptr source) {
		#pragma omp parallel for
		for(int j=0;j<domain.res[1];++j)
		for(int k=0;k<domain.res[2];++k) 
		for(int i=0;i<domain.res[0];++i) {
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
	void divFree(ScalarField boundary, int iterations) { }
 
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
  Vec3 D;
	Mat3 result;
	D = (eval(x + Vec3(dx[0],0,0)) - eval(x - Vec3(dx[0],0,0))) / (2 * dx[0]);
  result(0,0) = D[0]; result(0,1) = D[1]; result(0,2) = D[2];
  D = (eval(x + Vec3(0,dx[1],0)) - eval(x - Vec3(0,dx[1],0))) / (2 * dx[1]);
	result(1,0) = D[0]; result(1,1) = D[1]; result(1,2) = D[2];
  D = (eval(x + Vec3(0,0,dx[2])) - eval(x - Vec3(0,0,dx[2]))) / (2 * dx[2]);
	result(2,0) = D[0]; result(2,1) = D[1]; result(2,2) = D[2];
	return result;
}

#include <iostream>
// Divergence-Free Projection
// Use Helmholtz-Hodge decomposition to compute div-free component of a vector field
template<> void ConstructGrid<Vec3>::divFree(ScalarField boundary, int iterations) {
	ConstructGrid<real> p(domain, constant(static_cast<real>(0)).node);
	ConstructGrid<real> divergence(domain, constant(static_cast<real>(0)).node);
	p.bakeData(ScalarField(static_cast<real>(0)).node);

  ConstructGrid<real> r(domain, constant(static_cast<real>(0)).node);
  ConstructGrid<real> d(domain, constant(static_cast<real>(0)).node);
  ConstructGrid<real> q(domain, constant(static_cast<real>(0)).node);
  ConstructGrid<real> skip(domain, constant(static_cast<real>(1)).node);

  // Outside "skip" area
#pragma omp parallel for
  for(int i=0;i<domain.res[0];++i)
  for(int j=0;j<domain.res[1];++j)
  for(int k=0;k<domain.res[2];++k) {
    if(i==0 || i==domain.res[0]-1 || j==0 || j==domain.res[1]-1 || k==0 || k==domain.res[2]-1) 
      skip.set(i,j,k,1);
    else
      skip.set(i,j,k,0);

    // Add in extra boundaries
    if(boundary.eval(domain.position(i,j,k)) > 0)
      skip.set(i,j,k,1);
  }

  // TODO: Generalize this for other boundaries and conditions (Dirichlet, etc)
	// Set no flux for velocity
#pragma omp parallel for
    for(int k=0;k<domain.res[2];++k) 
    for(int j=0;j<domain.res[1];++j)
    for(int i=0;i<domain.res[0];++i) {
      if(i==0 || i==domain.res[0]-1) set(i,j,k, gets(i,j,k).cwiseProduct(Vec3(0,1,1)));
      if(j==0 || j==domain.res[1]-1) set(i,j,k, gets(i,j,k).cwiseProduct(Vec3(1,0,1)));
      if(k==0 || k==domain.res[2]-1) set(i,j,k, gets(i,j,k).cwiseProduct(Vec3(1,1,0)));
    }

	// Compute divergence of non-boundary cells
#pragma omp parallel for
    for(int k=1;k<domain.res[2]-1;++k) 
    for(int j=1;j<domain.res[1]-1;++j)
    for(int i=1;i<domain.res[0]-1;++i) {
      real D = gets(i+1,j,k)[0] + gets(i,j+1,k)[1] + gets(i,j,k+1)[2];
			D -= gets(i-1,j,k)[0] + gets(i,j-1,k)[1] + gets(i,j,k-1)[2];
			D *= .5f;
      divergence.set(i,j,k, D ); // ASSUMED CUBIC CELLS!
    }

    // Conjugate Gradient 
    // (http://en.wikipedia.org/wiki/Conjugate_gradient_method)
    // r = b - Ax
#pragma omp parallel for
    for(int k=1;k<domain.res[2]-1;++k) 
    for(int j=1;j<domain.res[1]-1;++j) 
    for(int i=1;i<domain.res[0]-1;++i) {
      real center = 0., R = 0.;
      if(skip.get(i-1,j,k)!=1) { center += 1; R += p.gets(i-1,j,k); }
      if(skip.get(i,j-1,k)!=1) { center += 1; R += p.gets(i,j-1,k); }
      if(skip.get(i,j,k-1)!=1) { center += 1; R += p.gets(i,j,k-1); }
      if(skip.get(i+1,j,k)!=1) { center += 1; R += p.gets(i+1,j,k); }
      if(skip.get(i,j+1,k)!=1) { center += 1; R += p.gets(i,j+1,k); }
      if(skip.get(i,j,k+1)!=1) { center += 1; R += p.gets(i,j,k+1); }
      
      R = -divergence.gets(i,j,k) - (center * p.gets(i,j,k) - R);
      r.set(i,j,k, skip.get(i,j,k)==1 ? 0. : R);
    }

    // d = r
#pragma omp parallel for
    for(int k=1;k<domain.res[2]-1;++k) 
    for(int j=1;j<domain.res[1]-1;++j)
    for(int i=1;i<domain.res[0]-1;++i) {
      d.set(i,j,k, r.gets(i,j,k) );
    }

    // deltaNew = transpose(r) * r
    real deltaNew = 0.;
#pragma omp parallel for reduction(+:deltaNew)
    for(int k=1;k<domain.res[2]-1;++k)
    for(int j=1;j<domain.res[1]-1;++j)
    for(int i=1;i<domain.res[0]-1;++i) {
      deltaNew += r.gets(i,j,k) * r.gets(i,j,k);
    }

    // delta = deltaNew
    const real eps = 1.e-4;
    real maxR = 1.f;
    int iter=0;
    while((iter<iterations) && (maxR > eps)) {
      // q = A d
#pragma omp parallel for
      for(int k=1;k<domain.res[2]-1;++k) 
      for(int j=1;j<domain.res[1]-1;++j)
      for(int i=1;i<domain.res[0]-1;++i) {
        real center = 0., R = 0.;
        if(skip.get(i-1,j,k)!=1) { center += 1; R += d.gets(i-1,j,k); }
        if(skip.get(i,j-1,k)!=1) { center += 1; R += d.gets(i,j-1,k); }
        if(skip.get(i,j,k-1)!=1) { center += 1; R += d.gets(i,j,k-1); }
        if(skip.get(i+1,j,k)!=1) { center += 1; R += d.gets(i+1,j,k); }
        if(skip.get(i,j+1,k)!=1) { center += 1; R += d.gets(i,j+1,k); }
        if(skip.get(i,j,k+1)!=1) { center += 1; R += d.gets(i,j,k+1); }
        
        R = (center * d.gets(i,j,k) - R);
        q.set(i,j,k, skip.get(i,j,k)==1 ? 0. : R);
      }

      // alpha = deltaNew / (d'q)
      real alpha = 0.f;
#pragma omp parallel for reduction(+:alpha)      
      for(int k=1;k<domain.res[2]-1;++k)
      for(int j=1;j<domain.res[1]-1;++j)
      for(int i=1;i<domain.res[0]-1;++i)
      {  alpha += d.gets(i,j,k) * q.gets(i,j,k); }

      if(fabs(alpha) > .0) 
        alpha = deltaNew / alpha;

      // x = x + alpha * d
#pragma omp parallel for
      for(int k=1;k<domain.res[2]-1;++k) 
      for(int j=1;j<domain.res[1]-1;++j)
      for(int i=1;i<domain.res[0]-1;++i) {
				p.set(i,j,k, p.gets(i,j,k) + alpha * d.gets(i,j,k)); 
        r.set(i,j,k, r.gets(i,j,k) - alpha * q.gets(i,j,k)); 
			}

      // r = r - alpha * q
      maxR = 0.;

//#pragma omp parallel for reduction(+:maxR)      
      for(int k=1;k<domain.res[2]-1;++k)  
      for(int j=1;j<domain.res[1]-1;++j)
      for(int i=1;i<domain.res[0]-1;++i) {
        maxR = r.gets(i,j,k) > maxR ? r.gets(i,j,k) : maxR;
      }

      real deltaOld = deltaNew;

      // deltaNew = r'r
      deltaNew = 0.;
#pragma omp parallel for reduction(+:deltaNew)      
      for(int k=1;k<domain.res[2]-1;++k)
      for(int j=1;j<domain.res[1]-1;++j)
      for(int i=1;i<domain.res[0]-1;++i) 
      { deltaNew += r.gets(i,j,k) * r.gets(i,j,k); }

      real beta = deltaNew / deltaOld;

      // d = r + beta * d
#pragma omp parallel for
      for(int k=1;k<domain.res[2]-1;++k) 
      for(int j=1;j<domain.res[1]-1;++j)
      for(int i=1;i<domain.res[0]-1;++i)
      {  d.set(i,j,k, r.get(i,j,k) + beta * d.gets(i,j,k));  }

      // Next iteration...
      ++iter;
#if 0
      if(iter%10==0)
      {
        using namespace std;
        cout << "Iteration " << iter << " -- Error: " << maxR << endl;
      }
#endif
    }

#pragma omp parallel for
    for(int k=0;k<domain.res[2];++k) 
    for(int j=0;j<domain.res[1];++j)
    for(int i=0;i<domain.res[0];++i) {
      if(i==0) p.set(i,j,k, p.gets(1,j,k));
      if(j==0) p.set(i,j,k, p.gets(i,1,k));
      if(k==0) p.set(i,j,k, p.gets(i,j,1));
      if(i==domain.res[0]-1) p.set(i,j,k, p.gets(domain.res[0]-2,j,k));
      if(j==domain.res[1]-1) p.set(i,j,k, p.gets(i,domain.res[1]-2,k));
      if(k==domain.res[2]-1) p.set(i,j,k, p.gets(i,j,domain.res[2]-2));
    }

	// Subtract gradient of "pressure"
#pragma omp parallel for
    for(int k=1;k<domain.res[2]-1;++k) 
    for(int j=1;j<domain.res[1]-1;++j)
    for(int i=1;i<domain.res[0]-1;++i) {
      Vec3 V = get(i,j,k);
      V[0] -= (p.gets(i+1,j,k) - p.gets(i-1,j,k)) * .5f; 
      V[1] -= (p.gets(i,j+1,k) - p.gets(i,j-1,k)) * .5f;
      V[2] -= (p.gets(i,j,k+1) - p.gets(i,j,k-1)) * .5f;
      set(i,j,k,V);
    }
}

inline VectorField divFree(VectorField field, ScalarField boundary, const Domain& domain, int iterations=30) {
	// TODO: build in isGridded() check and create shortcut for fields that are already grids
	// so we don't waste time writing them to a grid a second time

	ConstructGrid<Vec3> *grid = new ConstructGrid<Vec3>(domain, constant(Vec3(0,0,0)).node);
	grid->bakeData(field.node);
	grid->divFree(boundary, iterations);
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
