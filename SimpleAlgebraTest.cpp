#include <iostream>
#include <cstdio>
#include <chrono>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

int main(int argc, char **argv) {
  const Vec3 p(1,1,1);

	// Create some simple fields
  ScalarField c = 1.f;
  VectorField x = identity();

  // Do some simple algebraic operations
  auto a1 = x / c; 
  cout << a1.eval(p).transpose() << endl;
  
  auto a2 = x / (constant(1.f) + c*c);
  cout << a2.eval(p).transpose() << endl;

  auto a3 = (c - dot(x, constant(Vec3(0,1,2)))) / (constant(1.f) - c*dot(x,x));
  cout << a3.eval(p) << endl;

  // And take derivatives of these expressions!
  auto d1 = grad(a3);
  cout << d1.eval(p) << endl;

  auto d2 = grad(x);
  cout << d2.eval(p) << endl;

  // Can not take a spatial derivative of a matrix field though!
  // (This would generate a third-order tensor, which isn't supported (yet?)
  // auto d3 = grad(constant(Mat3(0,0,0)))
  
  // Also, you can not take two derivatives analytically here...
  // auto d4 = grad(grad(1.f / dot(x,x)))

  return 0;
}
