#include <iostream>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

int main(int argc, char **argv) {
  VectorField I = identity();
  VectorField n = Vec3(1,1,1).normalized();

  VectorField z = I - dot(n,I)*n;

  return 0;
}
