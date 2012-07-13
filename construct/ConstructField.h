#ifndef ConstructField_h
#define ConstructField_h

#include <memory>
#include "construct/ConstructBase.h"
namespace Construct {

//////////////////////////////////////////////////////////
// Field node types. Each is evaluatable and possibly once differentiable
template<typename evalType, typename gradType>
struct ConstructFieldNode {
  virtual evalType eval(const Vec3& x) const = 0;
  virtual gradType grad(const Vec3& x) const ;
};

typedef ConstructFieldNode<float, Vec3> ScalarFieldNode;
typedef ConstructFieldNode<Vec3, Mat3> VectorFieldNode;
typedef ConstructFieldNode<Mat3, Mat3> MatrixFieldNode;

//////////////////////////////////////////////////////////
// Special case: grad(Mat3) creates rank-3 tensor which
// we don't have. As a default behavior, we will just return
// an identity matrix. This should never be overridden.
template<>
Mat3 ConstructFieldNode<Mat3,Mat3>::grad(const Vec3& x) const
{ return Mat3::Identity(); }

// Pointers to fields used in constructing expression tree
typedef std::shared_ptr<ScalarFieldNode> SFNodePtr;
typedef std::shared_ptr<VectorFieldNode> VFNodePtr;
typedef std::shared_ptr<MatrixFieldNode> MFNodePtr;

//////////////////////////////////////////////////////////
//! Constant Fields
// Forward declared for use in Field constructors (below)
template<typename evalType, typename gradType>
struct ConstantField : public ConstructFieldNode<evalType, gradType> {
  evalType value;
  ConstantField(const evalType& value) : value(value) { }
  evalType eval(const Vec3& x) const { return value; }
  gradType grad(const Vec3& x) const { return gradType::Zero(); }
};
typedef ConstantField<real,Vec3> ConstantScalarField;
typedef ConstantField<Vec3,Mat3> ConstantVectorField;
typedef ConstantField<Mat3,Mat3> ConstantMatrixField;

//////////////////////////////////////////////////////////
//! The user-accessible field types: Scalar,Vector,Matrix
struct ScalarField {
  SFNodePtr node;
  ScalarField() 
  : node(SFNodePtr(new ConstantScalarField(0))) { }
  ScalarField(real value) 
  : node(SFNodePtr(new ConstantScalarField(value))) { }
  ScalarField(SFNodePtr node)
  : node(node) { }

  real eval(const Vec3& x) const { return node->eval(x); }
  Vec3 grad(const Vec3& x) const { return node->grad(x); }
};
struct VectorField {
  VFNodePtr node;
  VectorField() 
  : node(VFNodePtr(new ConstantVectorField(Vec3::Zero()))) { }
  VectorField(const Vec3& value) 
  : node(VFNodePtr(new ConstantVectorField(value))) { }
  VectorField(VFNodePtr node)
  : node(node) { }

  Vec3 eval(const Vec3& x) const { return node->eval(x); }
  Mat3 grad(const Vec3& x) const { return node->grad(x); }
};
struct MatrixField {
  MFNodePtr node;
  MatrixField() 
  : node(MFNodePtr(new ConstantMatrixField(Mat3::Zero()))) { }
  MatrixField(const Mat3& value) 
  : node(MFNodePtr(new ConstantMatrixField(value))) { }
  MatrixField(MFNodePtr node)
  : node(node) { }

  Mat3 eval(const Vec3& x) const { return node->eval(x); }
  Mat3 grad(const Vec3& x) const { return node->grad(x); }
};

};
#endif
