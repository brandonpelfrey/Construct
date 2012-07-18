#include <iostream>
#include <chrono>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

template<typename T>
Field<T> sla(Field<T> f, VectorField u, ScalarField dt) {
	return warp(f, identity() - u * dt);
}

ScalarField sphere(Vec3 center, float radius)
{ return constant(radius) - length(identity() - constant(center)); }

int main(int argc, char **argv) {
	const unsigned int R = 64; // Resolution
  Domain domain(R, R, R, Vec3(-1,-1,-1), Vec3(1,1,1));

	auto source = sphere(Vec3(0,-.5,0), .4f);
	auto density = mask(source);
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
		velocity = velocity + dt * density * constant(Vec3(0,1,0));
		velocity = divFree(velocity, domain);
		//////////////////////////////////////////////////////////	

		auto t2 = chrono::high_resolution_clock::now();
		cout << 
			"Finished time step " << iter+1 << " in " <<
			std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << 
			" milliseconds." << endl; 
	}
	return 0;
}
