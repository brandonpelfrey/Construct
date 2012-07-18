#ifndef ConstructDomain_h
#define ConstructDomain_h
#include "construct/ConstructBase.h"
namespace Construct {
struct Domain {
	int res[3]; //! Domain resolution
	Vec3 bmin, bmax; //! AABB minimal and maximal corner positions
	Vec3 extent, H, Hinverse; //! precomputed size of the box and cell size/inverse size
	Domain() { }
	Domain(int rx, int ry, int rz, Vec3 bmin, Vec3 bmax)
	: bmin(bmin), bmax(bmax) {
		res[0] = rx; res[1] = ry; res[2] = rz;
		extent = bmax - bmin;
		H = extent.cwiseQuotient( Vec3(res[0]-1, res[1]-1, res[2]-1) );
		Hinverse = Vec3(1.0/H[0], 1.0/H[1], 1.0/H[2]);
	}

	//! Return true if lattice point is inside the grid
	bool inside(int i, int j, int k) const
	{ return i>=0 && j>=0 && k>=0 && i<res[0] && j<res[1] && k<res[2]; }
	
	//! The world position of a lattice point on the grid
	Vec3 position(int i, int j, int k) const {
		return bmin + Vec3(i,j,k).cwiseProduct(H);
	}
};
};
#endif
