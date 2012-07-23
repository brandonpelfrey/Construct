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
  typename FieldInfo<T>::GradType grad(const Vec3& x) const { 
		throw std::logic_error("Can not take gradients of matrix fields in the Construct."); 
		return FieldInfo<typename FieldInfo<T>::GradType>::Zero();
	}
};
template<> Vec3 WarpField<real>::grad(const Vec3& x) const
{ return g->grad(x) * f->grad(g->eval(x)); }
template<> Mat3 WarpField<Vec3>::grad(const Vec3& x) const
{ return g->grad(x) * f->grad(g->eval(x)); }

template<typename T>
inline Field<T> warp(Field<T> f, VectorField v)
{ return Field<T>(new WarpField<T>(f.node, v.node)); }

// Cross Product
struct CrossProductField : public VectorFieldNode {
  VFNodePtr f,g;
  CrossProductField(VFNodePtr f, VFNodePtr g) : f(f), g(g) { }
  Vec3 eval(const Vec3& x) const { return f->eval(x).cross(g->eval(x)); }
  Mat3 grad(const Vec3& x) const {
    Mat3 df = f->grad(x), dg = g->grad(x), result;
    Vec3 fx = f->eval(x), gx = g->eval(x);
    result.row(0) = gx.z()*df.row(1) + fx.y()*dg.row(2) - fx.z()*dg.row(1) - gx.y()*df.row(2);
    result.row(1) = gx.x()*df.row(2) + fx.z()*dg.row(0) - fx.x()*dg.row(2) - gx.z()*df.row(0);
    result.row(2) = gx.y()*df.row(0) + fx.x()*dg.row(1) - fx.y()*dg.row(0) - gx.x()*df.row(1);
    return result;
  }
};
inline VectorField cross(VectorField f, VectorField g)
{ return VectorField(new CrossProductField(f.node, g.node)); }

// Outer Product
struct OuterProductField : public MatrixFieldNode {
  VFNodePtr f, g;
  OuterProductField(VFNodePtr f, VFNodePtr g) : f(f), g(g) { }
  Mat3 eval(const Vec3& x) const { return f->eval(x) * g->eval(x).transpose(); }
  // No grad(MatrixField) allowed
};
inline MatrixField outer_product(VectorField f, VectorField g)
{ return MatrixField(new OuterProductField(f.node, g.node)); }

// Solve 3x3 Linear Systems
struct LinearSolveField : public VectorFieldNode {
	MFNodePtr matrix;
	VFNodePtr vector;
	LinearSolveField(MFNodePtr matrix, VFNodePtr vector)
	: matrix(matrix), vector(vector) { }
	Vec3 eval(const Vec3& x) const {
		Mat3 m = matrix->eval(x);
		Vec3 v = vector->eval(x);
		Eigen::FullPivLU<Mat3> lu(m);
		if(lu.rank() < 3) return Vec3(0,0,0);
		return lu.solve(v);
	}
	Mat3 grad(const Vec3& x) const { 
		throw std::logic_error("Can not take gradients of matrix fields in the Construct."); 
		return Mat3::Zero();
	}
};
inline VectorField solve(MatrixField matrix, VectorField vector)
{ return VectorField(new LinearSolveField(matrix.node, vector.node)); }

// Matrix transpose
struct TransposeField : public MatrixFieldNode {
	MFNodePtr m;
	TransposeField(MFNodePtr m) : m(m) { }
	Mat3 eval(const Vec3& x) const { return m->eval(x).transpose(); }
	Mat3 grad(const Vec3& x) const { 
		throw std::logic_error("Can not take gradients of matrix fields in the Construct."); 
		return Mat3::Zero();
	}
};
inline MatrixField transpose(MatrixField m)
{ return MatrixField(new TransposeField(m.node)); }

};
#endif
