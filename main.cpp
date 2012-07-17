#include <iostream>
#include <chrono>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

template<typename T>
Field<T> sla(Field<T> f, VectorField u, ScalarField dt) {
	return warp(f, identity() - u * dt);
}

int main(int argc, char **argv) {
	const unsigned int R = 64; // Resolution
  Domain domain(R, R, R, Vec3(-1,-1,-1), Vec3(1,1,1));

	auto density = constant(0.f);
	auto velocity = constant(Vec3(0,1,0));
	auto dt = constant(.1f);

	for(int iter=0; iter<100; ++iter) {
		auto t1 = chrono::high_resolution_clock::now();
		
		// Advect density using semi-lagrangian advection		
		density = sla(density, velocity, dt);	
		density = writeToGrid(density, constant(0.f), domain);

		auto t2 = chrono::high_resolution_clock::now();
		cout << "\rFinished time step " << iter+1 << " in " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << " milliseconds.\t\t\t"; 
		cout.flush();
	}
	cout << endl;
	
	return 0;
}
