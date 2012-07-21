#ifndef ConstructCalculus_h
#define ConstructCalculus_h
#include "construct/ConstructAlgebra.h"
#include "construct/ConstructArithmetic.h"
namespace Construct {

//! Gradient Operator
template<typename T>
struct GradField : public ConstructFieldNode<typename FieldInfo<T>::GradType> {
	typedef typename ConstructFieldNode<T>::ptr Ptr;
	typedef typename FieldInfo<T>::GradType GradType;
	typedef typename FieldInfo<GradType>::GradType GradType2;
	Ptr f;
	GradField(Ptr f) : f(f) { }
	GradType eval(const Vec3& x) const { return f->grad(x); }
	GradType2 grad(const Vec3& x) const
	{ 
		throw std::logic_error("Can not analytically create second derivatives..."); 
		return FieldInfo<GradType2>::Zero(); 
	}
};
template<typename T>
inline Field<typename FieldInfo<T>::GradType> grad(Field<T> field)
{ return new GradField<T>(field.node ); }

//! Line integral Operator
template<typename T>
struct LineIntegralField : public ConstructFieldNode<T> {
	typedef typename ConstructFieldNode<T>::ptr Ptr;
	Ptr field;
	VFNodePtr start, flow;
	SFNodePtr distance, step_size;
	LineIntegralField(Ptr field, VFNodePtr start, VFNodePtr flow, SFNodePtr distance, SFNodePtr step_size)
	: field(field), start(start), flow(flow), distance(distance), step_size(step_size) { }
	T eval(const Vec3& x) const {
		real distance_travelled = static_cast<real>(0);
		real target_distance = distance->eval(x);
		T integral = FieldInfo<T>::Zero();
		real step;

		Vec3 p = start->eval(x);							 // Starting point along the integration	
		do {
			step = step_size->eval(p);           // Get the step size to use
			integral += field->eval(p) * step;   // Accumulate integrand
			const Vec3 dx = flow->eval(p) * step;// Compute new direction
			p += dx;                             // Move sample point to the new position
			target_distance -= step;             // Subtract how much farther we must go
		} while(target_distance >= static_cast<real>(0));
		
		return integral;
	}

	// TODO: Compute grad(lineIntegral) !
	typename FieldInfo<T>::GradType grad(const Vec3& x) const
	{ return FieldInfo<typename FieldInfo<T>::GradType>::Zero(); }
};
template<typename T>
inline Field<T> lineIntegral(Field<T> field, VectorField start, VectorField flow, ScalarField distance, ScalarField step) 
{ return Field<T>(new LineIntegralField<T>(field.node, start.node, flow.node, distance.node, step.node)); }

//! Divergence Operator
struct DivergenceField : public ScalarFieldNode {
	VFNodePtr field;
	DivergenceField(VFNodePtr field) : field(field) { }
	real eval(const Vec3& x) const {
		Mat3 G = field->grad(x);
		return G.trace();
	}
	Vec3 grad(const Vec3& x) const 
	{	throw std::logic_error("Can not analytically create second derivatives..."); return Vec3(0,0,0); }
};
inline ScalarField div(VectorField field)
{ return ScalarField(new DivergenceField(field.node)); }

//! Curl Operator
struct CurlField : public VectorFieldNode {
  VFNodePtr field;
  CurlField(VFNodePtr field) : field(field) { }
  Vec3 eval(const Vec3& x) const { 
    Mat3 G = field->grad(x);
    Vec3 r;
    r[0] = G(2,1) - G(1,2);
    r[1] = G(0,2) - G(2,0);
    r[2] = G(1,0) - G(0,1);
    return r;
  }
  Mat3 grad(const Vec3& x) const
	{	throw std::logic_error("Can not analytically create second derivatives..."); return Mat3::Zero(); }
};
inline VectorField curl(VectorField field)
{ return VectorField(new CurlField(field.node)); }

};
#endif
