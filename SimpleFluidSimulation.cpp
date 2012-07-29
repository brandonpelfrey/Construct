#include <iostream>
#include <cstdio>
#include <chrono>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

// Output a _very_ crude PPM down the middle slice of the volume
void render_ppm(const char *path, ScalarField field, VectorField color, Domain domain) {
	FILE *f = fopen(path, "wb");
	int W = 512;//domain.res[0] * 2;
	int H = 512;//domain.res[2] * 2;

	fprintf(f,"P6\n%d %d\n255\n",W,H);
	for(int y=H-1;y>=0;--y) {
		for(int x=0;x<W;++x) {
		
			Vec3 C(0,0,0); // Output color
			float T=1; // Transmittance
			const float ds = domain.H[2] * .5f;

      // Ray march through the volume
			for(float z=domain.bmin[2];z<=domain.bmax[2];z+=ds) {
				Vec3 X;
				X[0] = domain.bmin[0] + domain.extent[0] * (float)x / (float)(W-1);
				X[1] = domain.bmin[1] + domain.extent[1] * (float)y / (float)(H-1);
				X[2] = z;
				float rho = field.eval(X);
				if(rho <= 0) continue;
				Vec3 col = color.eval(X);

				const float dT = expf(rho * -ds);
				T *= dT;
				C += (1.f-dT) * T * col;
			}
			// Gamma adjust + convert to the [0,255] range
			const float gamma = 1.f / 2.f;
			for(int i=0;i<3;++i)
				C[i] = C[i] > 1.f ? 255 : powf(C[i], gamma) * 255;
			fprintf(f, "%c%c%c", (unsigned char)C[0], (unsigned char)C[1], (unsigned char)C[2]);
		}
	}

	fclose(f);
}

// Semi-lagrangian advection by warping space along 
// characteristic lines in the flow field
template<typename T>
Field<T> advect(Field<T> f, VectorField u, ScalarField dt) {
	return warp(f, identity() - u * dt);
}

// Signed distance function for a sphere
ScalarField sphere(Vec3 center, float radius) {
  return constant(radius) - length(identity() - constant(center));
}

int main(int argc, char **argv) {
	const unsigned int R = 64; // Resolution
  Domain domain(R, R, R, Vec3(-1,-1,-1), Vec3(1,1,1));

  auto density = mask(sphere(Vec3(0,0,0), .8f));
  auto velocity = constant(Vec3(0,0,0));
	auto dt = constant(.1f);

	for(int iter=0; iter<1000; ++iter) {
		//////////////////////////////////////////////////////////	
		// Advect density using semi-lagrangian advection		
		density = advect(density, velocity, dt);
		density = writeToGrid(density, constant(0.f), domain);

		// Advect velocity similarly
		velocity = advect(velocity, velocity, dt);
    // Add force upward, proportional to density
    velocity = velocity + dt * density * constant(Vec3(0,1,0));
    // Div-Free Projection
		velocity = divFree(velocity, constant(0.f), domain, 50);
		//////////////////////////////////////////////////////////	

		// Output results
		char path[256];
		sprintf(path, "frame.%04d.ppm", iter);
		ScalarField render_density = density ;
		VectorField render_color = density * constant(Vec3(1,1,1));
		render_ppm(path, render_density, render_color, domain);

		cout << "Finished time step " << iter+1 << endl;
	}
	return 0;
}
