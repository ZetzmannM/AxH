#ifndef __MACRO_UTILS_H
#define __MACRO_UTILS_H

#define __TBR__ //To be removed, because of bad or nonexisting design f.e.
#define __NOT_IMPLEMENTED__

//<summary>Marks critical sections</summary>
#define critical 
//<summary>Marks thread_safe Sections sections</summary>
#define thread_safe 

///<summary>Epsilon for very low precision calculation</summary>
#define SHORTFLOAT_EPSILON 0.001f

///<summary>Epsilon for single precision calculation</summary>
#define FLOAT_EPSILON 0.000001f

///<summary>Epsilon for double precision calculation</summary>
#define DOUBLE_EPSILON 0.00000000001

///<summary>
/// "Integer Floatingpoint cast"
/// returns the smallest positive integer which is greater or equal to (a + e). e is the given epsilon, and is supposed to prevent 1.999999898 from becoming 1 when casted.
///</summary>
///<param name='a'>Value to be 'smoothed'</param>
///<param name='e'>To be used epsilon value</param>
#define IFC(a,e) ((std::abs(a-(int32)a) >= 1-e)?(((int32)a)+1):((int32)a))

///<summary>
///Intends to remove the effects of machine uncertainty. Round the given value to the next int if the difference is smaller than the given epsilon
///</summary>
///<param name='a'>Value to be 'smoothed'</param>
///<param name='e'>To be used epsilon value</param>
#define SMOOTH_VALUE(a,e) ((std::abs(std::round(a)-a)<e)?(std::round(a)):(a))
#define SMOOTH_FLOAT(a) SMOOTH_VALUE(a, FLOAT_EPSILON)
#define SMOOTH_DOUBLE(a) SMOOTH_VALUE(a, DOUBLE_EPSILON)

/* Math Constants*/
#define MATH_PI 3.14159265358979323846264
#define MATH_E 2.71828182845904523536028

#define MATH_PIF 3.14159265358979323846264f
#define MATH_EF 2.71828182845904523536028f

///<summary>
/// Sign function
///</summary>
///<param name='a'>Function argument</param>
#define SGN(a) ((a<0)?(-1):(1))

///<summary>
///Returns the smaller argument
///</summary>
///<param name='A'>first argument</param>
///<param name='B'>second argument</param>
#define MIN(A,B) ((A<B)?(A):(B))

///<summary>
///Returns the greater argument
///</summary>
///<param name='A'>first argument</param>
///<param name='B'>second argument</param>
#define MAX(A,B) ((A>B)?(A):(B))

///<summary>
///Converts degrees to radiant
///</summary>
///<param name='a'>Input in degrees</param>
#define RAD(a) (a/180.0 * MATH_PI)

///<summary>
///Converts radiant to degrees
///</summary>
///<param name='a'>Input in radiant</param>
#define DEG(r) (a/MATH_PI * 180.0)

///<summary>
///Converts degrees to radiant
///</summary>
///<param name='a'>Input in degrees</param>
#define RADF(a) (a/180.0f * MATH_PIF)

///<summary>
///Converts radiant to degrees
///</summary>
///<param name='a'>Input in radiant</param>
#define DEGF(r) (a/MATH_PIF * 180.0f)

#endif