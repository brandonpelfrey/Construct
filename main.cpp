#include <iostream>
#include <cstdio>
#include <chrono>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

// Output a _very_ crude PPM down the middle slice of the volume
void render_ppm(const char *path, ScalarField field, Domain domain) {
	FILE *f = fopen(path, "wb");
	float max_value = 1.f;
	int W = domain.res[0] * 2;
	int H = domain.res[2] * 2;

	fprintf(f,"P6\n%d %d\n255\n",W,H);
	//unsigned char *pix = new unsigned char[W*H*3];
	for(int y=0;y<H;++y) {
		for(int x=0;x<W;++x) {
			Vec3 X;
			X[0] = domain.bmin[0] + domain.extent[0] * (float)x / (float)(W-1);
			X[1] = domain.bmin[2] + domain.extent[1] * (float)y / (float)(H-1);
			X[2] = (domain.bmin[2] + domain.bmax[2]) * .5f;
	
			float sample = field.eval(X);
			unsigned char C = sample / max_value * 255;
			fprintf(f, "%c%c%c", C, C, C);
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

int main(int argc, char **argv) {
	const unsigned int R = 128; // Resolution
  Domain domain(R, R, R, Vec3(-1,-1,-1), Vec3(1,1,1));

	auto density = mask(sphere(Vec3(0,-.5,0), .4f));
	auto velocity = constant(Vec3(0,0,0));
	auto dt = constant(.1f);

	for(int iter=0; iter<100; ++iter) {
		auto t1 = chrono::high_resolution_clock::now();
	
		//////////////////////////////////////////////////////////	
		// Advect density using semi-lagrangian advection		
		density = sla(density, velocity, dt);	
		density = writeToGrid(density, constant(0.f), domain);

		// Advect velocity similarly
		velocity = sla(velocity, velocity, dt);
		
		// Apply bouyant force
		velocity = velocity + dt * density * constant(Vec3(0,1,0));

		// Project to divergence free (volume conserving)
		velocity = divFree(velocity, domain);
		//////////////////////////////////////////////////////////	

		// Output results
#if 1
		char path[256];
		//sprintf(path, "density.%04d.grid", iter);
		//saveGriddedField(path, density, domain);

		sprintf(path, "frame.%04d.ppm", iter);
		render_ppm(path, density, domain);
#endif

		// Output some 
		auto t2 = chrono::high_resolution_clock::now();
		cout << 
			"Finished time step " << iter+1 << " in " <<
			chrono::duration_cast<chrono::milliseconds>(t2-t1).count() << 
			" milliseconds." << endl; 
	}
	return 0;
}
