#ifndef ConstructBase_H
#define ConstructBase_H
#include <cmath>
#include <Eigen/Dense>

namespace Construct {
  //! Floating point precision
  typedef float real;

  // Linear Algebra types
  typedef Eigen::Matrix<real, 3, 1> Vec3;
  typedef Eigen::Matrix<real, 3, 3> Mat3;
};

#endif
