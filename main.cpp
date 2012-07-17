#include <iostream>
#include <stdexcept>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

int main(int argc, char **argv) {
	auto I = identity() / length(identity());
  cout << I.eval(Vec3(8,2,1)) << endl;
  return 0;
}
