#ifndef ConstructField_h
#define ConstructField_h

#include <memory>
#include "construct/ConstructBase.h"
namespace Construct {

template<typename> struct FieldInfo;
template<> struct FieldInfo<real> {
  typedef Vec3 GradType;
  static inline real Zero() { return static_cast<real>(0); }
};
template<> struct FieldInfo<Vec3> {
  typedef Mat3 GradType;
  static inline Vec3 Zero() { return Vec3::Zero(); }
};
template<> struct FieldInfo<Mat3> {
  typedef Mat3 GradType;
  static inline Mat3 Zero() { return Mat3::Zero(); }
};

//////////////////////////////////////////////////////////
// Field node types. Each is evaluatable and possibly once differentiable
template<typename T>
struct ConstructFieldNode {
  typedef std::shared_ptr<ConstructFieldNode<T> > ptr;
  virtual T eval(const Vec3& x) const = 0;
  virtual typename FieldInfo<T>::GradType grad(const Vec3& x) const 
  { return FieldInfo<typename FieldInfo<T>::GradType>::Zero(); } 
};

typedef ConstructFieldNode<float> ScalarFieldNode;
typedef ConstructFieldNode<Vec3> VectorFieldNode;
typedef ConstructFieldNode<Mat3> MatrixFieldNode;

//////////////////////////////////////////////////////////
//! Constant Fields
// Forward declared for use in Field constructors (below)
template<typename T>
struct ConstantField : public ConstructFieldNode<T> {
  typedef typename FieldInfo<T>::GradType GradType;
  T value;
  ConstantField() : value(FieldInfo<T>::Zero()) { } 
  ConstantField(const T& value) : value(value) { }
  T eval(const Vec3& x) const { return value; }
  GradType grad(const Vec3& x) const { return FieldInfo<GradType>::Zero(); }
};
typedef ConstantField<real> ConstantScalarField;
typedef ConstantField<Vec3> ConstantVectorField;
typedef ConstantField<Mat3> ConstantMatrixField;

//////////////////////////////////////////////////////////
//! The user-accessible field types: Scalar,Vector,Matrix
template<typename T>
struct Field {
  typedef typename ConstructFieldNode<T>::ptr NodePtr;
  NodePtr node;
  Field() : node(NodePtr(new ConstantField<T>())) {}
  Field(const T& value) : node(NodePtr(new ConstantField<T>(value))) { }

  Field(ConstructFieldNode<T>* node) : node(NodePtr(node)) { }
  Field(NodePtr node) : node(node) { }
 
  //! Evaluates the underlying expression tree
  T eval(const Vec3& x) const 
  { return node->eval(x); }
  typename FieldInfo<T>::GradType grad(const Vec3& x) const
  { return node->grad(x); }
};

template<> Mat3 Field<Mat3>::grad(const Vec3& x) const
{ throw std::logic_error("Can not take gradients of matrix fields in the Construct."); }

// Convenience type defs
typedef typename ConstructFieldNode<real>::ptr SFNodePtr;
typedef typename ConstructFieldNode<Vec3>::ptr VFNodePtr;
typedef typename ConstructFieldNode<Mat3>::ptr MFNodePtr;

// Types that the user actually uses
typedef Field<real> ScalarField;
typedef Field<Vec3> VectorField;
typedef Field<Mat3> MatrixField;

};
#endif
