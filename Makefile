itarget:
	g++ -Wall -std=c++0x -O4 -fopenmp -mfpmath=sse -msse4 -fexpensive-optimizations main.cpp -I. -o test
