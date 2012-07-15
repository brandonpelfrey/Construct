#ifndef ConstructAlgebra_h
#define ConstructAlgebra_h
#include "construct/ConstructField.h"
namespace Construct {

/////////////////////////////////////

// Identity field
struct IdentityField : public VectorFieldNode {
  Vec3 eval(const Vec3& x) const { return x; }
  Mat3 grad(const Vec3& x) const { return Mat3::Identity(); }
};
inline VectorField identity()
{ return new IdentityField(); }

// Length(vectorfield) field
struct LengthField : public ScalarFieldNode {
  VFNodePtr v;
  LengthField(VFNodePtr v) : v(v) { }
  real eval(const Vec3& x) const { return v->eval(x).norm(); }
  Vec3 grad(const Vec3& x) const { 
    Vec3 ve = v->eval(x);
    Mat3 vg = v->grad(x);
    return ( vg.transpose() * ve ) / (static_cast<real>(1.e-5) + ve.norm()); 
  }
};
inline ScalarField length(VectorField v)
{ return ScalarField(new LengthField(v.node)); }

// Inner Product
struct InnerProductField : public ScalarFieldNode {
  VFNodePtr A, B;
  InnerProductField(VFNodePtr A, VFNodePtr B) : A(A), B(B) { }
  real eval(const Vec3& x) const { return A->eval(x).dot(B->eval(x)); }
  Vec3 grad(const Vec3& x) const // TODO: Check for correctness 
  { return A->grad(x).transpose()*B->eval(x) + B->grad(x).transpose()*A->eval(x); }
};
inline ScalarField dot(VectorField a, VectorField b)
{ return ScalarField(new InnerProductField(a.node,b.node)); }

// Function Composition -> f(g(x))
template<typename T>
struct WarpField : public ConstructFieldNode<T> {
  typename ConstructFieldNode<T>::ptr f; 
  VFNodePtr g;
  WarpField(typename ConstructFieldNode<T>::ptr f, VFNodePtr g) : f(f), g(g) { }
  T eval(const Vec3& x) const { return f->eval(g->eval(x)); }
  typename FieldInfo<T>::GradType grad(const Vec3& x) const
  { return f->grad(g->eval(x)) * g->grad(x); }
};
template<typename T>
Field<T> warp(Field<T> f, VectorField v)
{ return Field<T>(new WarpField<T>(f.node, v.node)); }

};
#endif
