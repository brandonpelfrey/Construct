#include <iostream>
#include "construct/ConstructField.h"
#include "construct/ConstructAlgebra.h"
using namespace Construct;
using namespace std;

int main(int argc, char **argv) {
  ScalarField a = 4.f, b = 7.2;
  ScalarField c = a+b;
  cout << c.eval(Vec3(0,0,0)) << endl;
  cout << c.grad(Vec3(0,0,0)) << endl;

  cout << "Vector fields..." << endl;
  VectorField d = Vec3(1,2,3), e = Vec3(3,3,-1);
  VectorField f = d-e;
  f = f * ScalarField(4.f);
  cout << f.eval(Vec3(0,0,0)) << endl;

  cout << "--------" << endl;
  VectorField I = identity();
  cout << I.eval(Vec3(1,1,1)) << endl;
  cout << I.grad(Vec3(1,1,1)) << endl;

  return 0;
}
