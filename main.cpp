#include <iostream>
#include <stdexcept>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

int main(int argc, char **argv) {
  VectorField I = identity();
  VectorField n = Vec3(1,0,0).normalized();

  VectorField z = I*2 - dot(n,I)*n;
  VectorField u = warp(z, VectorField(Vec3(0,1,0)));
  cout << u.eval(Vec3(1,2,3)) << endl;

  MatrixField mf = (Mat3)Mat3::Identity();
  mf = mf * 4.f;
  cout << mf.eval(Vec3(8,2,1)) << endl;
  return 0;
}
