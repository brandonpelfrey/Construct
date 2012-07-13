#include <iostream>
#include "construct/ConstructField.h"
using namespace Construct;

class Test : public MatrixFieldNode {
public:
  Mat3 eval(const Vec3& x) const
  { return Mat3::Identity() * .25f; }
};

int main(int argc, char **argv) {
  Test mf;
  using namespace std;
  cout << mf.eval(Vec3(0,0,0)) << endl;
  cout << "-------" << endl;
  cout << mf.grad(Vec3(0,0,0)) << endl;

  return 0;
}
