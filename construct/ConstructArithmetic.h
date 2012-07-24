#ifndef ConstructArithmetic_h
#define ConstructArithmetic_h
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
 typename FieldInfo<T>::GradType grad(const Vec3& x) const
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
 typename FieldInfo<T>::GradType grad(const Vec3& x) const
 { return A->grad(x) - B->grad(x); }
};
template<typename T>
Field<T> operator-(Field<T> A, Field<T> B)
{ return Field<T>(new SubtractionField<T>(A.node,B.node)); }

// Multiplication
template<typename LeftType, typename RightType, typename ResultType>
struct MultiplicationField : public ConstructFieldNode<ResultType> {
  typename ConstructFieldNode<LeftType>::ptr  A;
  typename ConstructFieldNode<RightType>::ptr B;
  MultiplicationField(
    typename ConstructFieldNode<LeftType>::ptr  A,
    typename ConstructFieldNode<RightType>::ptr B) : A(A), B(B) { }
  ResultType eval(const Vec3& x) const
  { return A->eval(x) * B->eval(x); }
  typename FieldInfo<ResultType>::GradType grad(const Vec3& x) const
  { return A->grad(x) * B->eval(x) + A->eval(x) * B->grad(x); }
};
// grad( Vector * Real )
template<> Mat3 MultiplicationField<Vec3,real,Vec3>::grad(const Vec3& x) const
{ return A->grad(x) * B->eval(x) + A->eval(x) * B->grad(x).transpose(); }

// Disallow gradients of those operations on matrix fields
template<> Mat3 MultiplicationField<Mat3,Vec3,Vec3>::grad(const Vec3& x) const
{ throw std::logic_error("Can not take gradients of matrix fields in the Construct."); }
template<> Mat3 MultiplicationField<Mat3,Mat3,Mat3>::grad(const Vec3& x) const
{ throw std::logic_error("Can not take gradients of matrix fields in the Construct."); }
template<> Mat3 MultiplicationField<Mat3,real,Mat3>::grad(const Vec3& x) const
{ throw std::logic_error("Can not take gradients of matrix fields in the Construct."); }
template<> Mat3 MultiplicationField<real,Mat3,Mat3>::grad(const Vec3& x) const
{ throw std::logic_error("Can not take gradients of matrix fields in the Construct."); }

ScalarField operator*(ScalarField a, ScalarField b)
{ return ScalarField(new MultiplicationField<real,real,real>(a.node, b.node)); }
VectorField operator*(VectorField v, ScalarField s)
{ return VectorField(new MultiplicationField<Vec3,real,Vec3>(v.node, s.node)); }
VectorField operator*(ScalarField s, VectorField v)
{ return VectorField(new MultiplicationField<Vec3,real,Vec3>(v.node, s.node)); }
VectorField operator*(MatrixField m, VectorField v) 
{ return VectorField(new MultiplicationField<Mat3,Vec3,Vec3>(m.node, v.node)); }
MatrixField operator*(MatrixField a, MatrixField b) 
{ return MatrixField(new MultiplicationField<Mat3,Mat3,Mat3>(a.node, b.node)); }
MatrixField operator*(MatrixField m, ScalarField s)
{ return MatrixField(new MultiplicationField<Mat3,real,Mat3>(m.node, s.node)); }
MatrixField operator*(ScalarField s, MatrixField m)
{ return MatrixField(new MultiplicationField<real,Mat3,Mat3>(s.node, m.node)); }

// Division
// G(f/g) = Gf / g + f * G(1/g) = Gf/g + f*( -1/g^2 * g' )
// G(f/g) = Gf/g -f g'/g^2 = (f'g - fg') / g^2
template<typename T>
struct DivisionField : public ConstructFieldNode<T> {
  typename ConstructFieldNode<T>::ptr A;
  SFNodePtr B;
  DivisionField(typename ConstructFieldNode<T>::ptr A, SFNodePtr divisor) 
  : A(A), B(divisor) { }
  T eval(const Vec3& x) const
  { return A->eval(x) / B->eval(x); }
  typename FieldInfo<T>::GradType grad(const Vec3& x) const
  { 
    real div = B->eval(x);
    return (A->grad(x)*div - A->eval(x) * B->grad(x)) / (div*div);
  }
};

// TODO: Check to see if AB' should be transposed!
template<> Mat3 DivisionField<Vec3>::grad(const Vec3& x) const
{
    real div = B->eval(x);
    return (A->grad(x)*div - A->eval(x) * B->grad(x).transpose()) / (div*div);
}
template<> Mat3 DivisionField<Mat3>::grad(const Vec3& x) const
{ throw std::logic_error("Can not take gradients of matrix fields in the Construct."); }

template<typename T>
inline Field<T> operator/(Field<T> field, ScalarField divisor)
{ return Field<T>(new DivisionField<T>(field.node, divisor.node)); }


};
#endif
