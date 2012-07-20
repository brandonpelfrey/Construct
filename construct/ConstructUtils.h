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

};
#endif
