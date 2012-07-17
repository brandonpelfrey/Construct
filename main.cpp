#include <iostream>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

int main(int argc, char **argv) {
	auto I = identity() / length(identity());
	Domain D = Domain(64,64,64, Vec3(-1,-1,-1), Vec3(1,1,1));
	auto Ig = writeToGrid(I, constant(Vec3(0,0,0)), D);

	auto li = lineIntegral(I, identity(), identity() - constant(Vec3(0,0,0)), constant(1.f), constant(.1f));

  cout << I.eval(Vec3(-1,-1,-1)) << endl;
	cout << Ig.eval(Vec3(-1,-1,-1)) << endl;
  return 0;
}
