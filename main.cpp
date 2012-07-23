#include <iostream>
#include <cstdio>
#include <chrono>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

// Output a _very_ crude PPM down the middle slice of the volume
void render_ppm(const char *path, ScalarField field, Domain domain) {
	FILE *f = fopen(path, "wb");
	int W = domain.res[0] * 2;
	int H = domain.res[2] * 2;

	fprintf(f,"P6\n%d %d\n255\n",W,H);
	//unsigned char *pix = new unsigned char[W*H*3];
	for(int y=H-1;y>=0;--y) {
		for(int x=0;x<W;++x) {
		
			float C=0,T=1;
			const float ds = domain.H[2] * .5f;
			for(float z=domain.bmin[2];z<=domain.bmax[2];z+=ds) {
				Vec3 X;
				X[0] = domain.bmin[0] + domain.extent[0] * (float)x / (float)(W-1);
				X[1] = domain.bmin[1] + domain.extent[1] * (float)y / (float)(H-1);
				X[2] = z;
				float rho = field.eval(X);
				if(rho <= 0) continue;
				const float dT = expf(rho * -ds);
				T *= dT;
				C += (1.f-dT) * T * 1;
			}

			// Gamma adjust + convert to the [0,255] range
			unsigned char Cc = powf(C, 1.f/1.7f) * 255;
			fprintf(f, "%c%c%c", Cc, Cc, Cc);
		}
	}

	fclose(f);
}

// Semi-lagrangian advection by warping space along 
// characteristic lines in the flow field
template<typename T>
Field<T> sla(Field<T> f, VectorField u, ScalarField dt) {
	return warp(f, identity() - u * dt);
}

// Create a field which is equal to the signed 
// disance from the surface of a sphere
// f(x) = r - |x - C|
// where r=radius, C=center of the sphere
ScalarField sphere(Vec3 center, float radius)
{ return constant(radius) - length(identity() - constant(center)); }

// Vorticity Confinement
VectorField VorticityConfinement(VectorField velocity, float epsilon, Domain domain) {
  auto C = writeToGrid(curl(velocity), constant(Vec3(0,0,0)), domain);
  auto eta = grad(length(C));
  auto N = eta / (constant(.00001f) + length(eta));
  return constant(epsilon) * cross(N,C);
}

VectorField MPA(VectorField u, Domain domain, ScalarField dt) {
	return sla(u, (u+sla(u,u,dt*.5f))*.5f, dt);
}

//
int main(int argc, char **argv) {
	const unsigned int R = 64; // Resolution
  Domain domain(R, R, R, Vec3(-1,-1,-1), Vec3(1,1,1));

	auto density = mask(sphere(Vec3(0,-.5,0), .4f)) * 2.f;
	auto velocity = constant(Vec3(0,0,0));
	auto dt = constant(.1f);

	for(int iter=0; iter<100; ++iter) {
		//////////////////////////////////////////////////////////	
		// Advect density using semi-lagrangian advection		
		density = sla(density, velocity, dt);	
		density = writeToGrid(density, constant(0.f), domain);

		// Advect velocity similarly
    VectorField force = VorticityConfinement(velocity, domain.H[0]*1.4f, domain);
    force = force + density * constant(Vec3(0,1,0));
		//velocity = sla(velocity, velocity, dt) + force * dt;
		velocity = MPA(velocity, domain, dt) + force * dt;
		velocity = divFree(velocity, domain, R);
		//////////////////////////////////////////////////////////	

		// Output results
#if 1
		char path[256];
		//sprintf(path, "density.%04d.grid", iter);
		//saveGriddedField(path, density, domain);
		sprintf(path, "frame.%04d.ppm", iter);
		render_ppm(path, density, domain);
#endif

		cout << "Finished time step " << iter+1 << endl;
	}
	return 0;
}
