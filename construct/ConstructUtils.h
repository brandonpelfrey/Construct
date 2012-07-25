#ifndef ConstructUtils_h
#define ConstructUtils_h
#include "construct/ConstructField.h"
namespace Construct {

struct MaskField : public ScalarFieldNode {
	SFNodePtr field;
	MaskField(SFNodePtr field) : field(field) { }
	real eval(const Vec3& x) const {
		return field->eval(x) > 0 ? static_cast<real>(1) : static_cast<real>(0);
	}

	// This is not technically differentiable...
	// Excepting the borders where there is an infinite derivative,
	// everywhere else is constant -> has a zero derivative 
	Vec3 grad(const Vec3& x) const {
		return Vec3(0,0,0);
	}
};
inline ScalarField mask(ScalarField field)
{ return ScalarField(new MaskField(field.node)); }

// Absolute value
template<typename T>
struct AbsoluteValueField : public ConstructFieldNode<T> {
	typedef typename ConstructFieldNode<T>::ptr Ptr;
	Ptr field;
	AbsoluteValueField(Ptr field) : field(field) { }
	T eval(const Vec3& x) const { return field->eval(x).cwiseAbs(); }
	typename FieldInfo<T>::GradType grad(const Vec3& x) const
	{ return FieldInfo<typename FieldInfo<T>::GradType>::Zero(); }
	// TODO: Implement grad(abs)
};
template<> real AbsoluteValueField<real>::eval(const Vec3& x) 
const { return abs(field->eval(x)); }

template<typename T> inline Field<T> abs(Field<T> field)
{ return Field<T>(new AbsoluteValueField<T>(field.node)); }

};
#endif
