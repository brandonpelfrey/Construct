targets=SimpleFluidSimulation
CFLAGS=-Wall -std=c++0x -I.
CFLAGS+=-O3 -fopenmp -mtune=native -msse3 -ffast-math

SimpleFluidSimulation: SimpleFluidSimulation.cpp
	g++ $(CFLAGS) $< -o $@

SimpleAlgebraTest: SimpleAlgebraTest.cpp
	g++ $(CFLAGS) $< -o $@
