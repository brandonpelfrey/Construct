#include <iostream>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

int main(int argc, char **argv) {
	auto I = identity() / length(identity());
	auto Z = grad(I);
	Domain D = Domain(64,64,64, Vec3(-1,-1,-1), Vec3(1,1,1));
	auto Ig = writeToGrid(I, constant(Vec3(0,0,0)), D);

  cout << Z.eval(Vec3(1,2,3)) << endl;
  return 0;
}
