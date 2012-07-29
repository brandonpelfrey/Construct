targets=SimpleFluidSimulation
CFLAGS=-Wall -std=c++0x -I.
CFLAGS+=-O3 -fopenmp -msse3

SimpleFluidSimulation: SimpleFluidSimulation.cpp
	g++ $(CFLAGS) $< -o $@

SimpleAlgebraTest: SimpleAlgebraTest.cpp
	g++ $(CFLAGS) $< -o $@
