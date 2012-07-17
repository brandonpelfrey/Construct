#ifndef ConstructGrid_h
#define ConstructGrid_h
#include "construct/ConstructField.h"
#include "construct/ConstructDomain.h"
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

		int i = (int)floor(relative.x());
		int j = (int)floor(relative.z());
		int k = (int)floor(relative.y());
		
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
  
	void load(const char* path) { 
    FILE *f = fopen(path, "rb");
		Domain newdomain;

    if(3 != fread(newdomain.res+0, sizeof(int), 3, f) ) { }
    if(2 != fread(&newdomain.bmin, sizeof(Vec3), 1, f)) { }
    if(2 != fread(&newdomain.bmax, sizeof(Vec3), 1, f)) { }
	
		domain = newdomain;
    const unsigned int N = domain.res[0] * domain.res[1] * domain.res[2];
    delete[] data;
    data = NULL;
    data = new T[N];
    if(N != fread(data, sizeof(T), N, f)) { }
    fclose(f);
  }

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

template<typename T>
inline Field<T> writeToGrid(Field<T> field, Field<T> outside, Domain domain) {
  ConstructGrid<T> *grid = new ConstructGrid<T>(domain, outside.node);
  grid->bakeData(field.node);
  return Field<T>(grid);
}

};
#endif
