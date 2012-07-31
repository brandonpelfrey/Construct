#ifndef ConstructTransforms_h
#define ConstructTransforms_h
#include "construct/ConstructField.h"
namespace Construct {

// Translate
template<typename T>
struct TranslateField : public ConstructFieldNode<T> {
  typedef typename ConstructFieldNode<T>::ptr Ptr;
  Ptr field;
  VFNodePtr translation;
  TranslateField(Ptr field, VFNodePtr translation)
    : field(field), translation(translation) { }
  T eval(const Vec3& x) const {
    Vec3 trans = translation->eval(x);
    return field->eval(x - trans);
  }
  typename FieldInfo<T>::GradType grad(const Vec3& x) const {
    return field->grad(translation->eval(x)) * translation->grad(x);
  }
};
template<typename T>
inline Field<T> translate(Field<T> field, VectorField translation)
{ return Field<T>(new TranslateField<T>(field.node, translation.node)); }


};
#endif
