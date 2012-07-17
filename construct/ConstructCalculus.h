#ifndef ConstructCalculus_h
#define ConstructCalculus_h
#include "construct/ConstructAlgebra.h"
#include "construct/ConstructArithmetic.h"
namespace Construct {

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
		throw std::logic_error("Can not make create a grad(grad)..."); 
		return FieldInfo<GradType2>::Zero(); 
	}
};
template<typename T>
inline Field<typename FieldInfo<T>::GradType> grad(Field<T> field)
{ return new GradField<T>(field.node ); }

};
#endif
