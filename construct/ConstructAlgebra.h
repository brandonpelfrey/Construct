#ifndef ConstructAlgebra_h
#define ConstructAlgebra_h
#include "construct/ConstructField.h"
namespace Construct {

// Addition
template<typename T>
struct AdditionField : public ConstructFieldNode<T> {
 typedef typename ConstructFieldNode<T>::ptr Ptr;
 Ptr A, B;
 AdditionField(Ptr A, Ptr B) : A(A), B(B) { }
 T eval(const Vec3& x) const 
 { return A->eval(x) + B->eval(x); }
 typename FieldInfo<T>::GradType grad(const Vec3& x)
 { return A->grad(x) + B->grad(x); }
};
template<typename T>
Field<T> operator+(Field<T> A, Field<T> B)
{ return Field<T>(new AdditionField<T>(A.node,B.node)); }

// Subtraction
template<typename T>
struct SubtractionField : public ConstructFieldNode<T> {
 typedef typename ConstructFieldNode<T>::ptr Ptr;
 Ptr A, B;
 SubtractionField(Ptr A, Ptr B) : A(A), B(B) { }
 T eval(const Vec3& x) const 
 { return A->eval(x) - B->eval(x); }
 typename FieldInfo<T>::GradType grad(const Vec3& x)
 { return A->grad(x) - B->grad(x); }
};
template<typename T>
Field<T> operator-(Field<T> A, Field<T> B)
{ return Field<T>(new SubtractionField<T>(A.node,B.node)); }

// Scalar-Vector Multiply
struct SVMultField : public VectorFieldNode {
  SFNodePtr s;
  VFNodePtr v;
  SVMultField(SFNodePtr s, VFNodePtr v) : s(s), v(v) { }
  Vec3 eval(const Vec3& x) const
  { return v->eval(x) * s->eval(x); }
  Mat3 grad(const Vec3& x) const
  { return v->grad(x)*s->eval(x) + v->eval(x) * s->grad(x).transpose(); }
};
VectorField operator*(VectorField v, ScalarField s)
{ return VectorField(new SVMultField(s.node, v.node)); }

// Identity field
struct IdentityField : public VectorFieldNode {
  Vec3 eval(const Vec3& x) const { return x; }
  Mat3 grad(const Vec3& x) const { return Mat3::Identity(); }
};
inline VectorField identity()
{ return new IdentityField(); }

};
#endif
