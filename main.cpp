#include <iostream>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

int main(int argc, char **argv) {
	auto x = identity();
  auto v1 = constant(Vec3(1,2,3));
  auto z = outer_product(x, v1);

  cout << z.eval(Vec3(1,1,1)) << endl;

  return 0;
}
