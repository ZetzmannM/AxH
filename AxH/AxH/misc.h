#ifndef __MACRO_UTILS_H
#define __MACRO_UTILS_H

#define __TBR__ //To be removed, because of bad or nonexisting design f.e.
#define __NOT_IMPLEMENTED__

//<summary>Marks critical sections</summary>
#define critical 
//<summary>Marks thread_safe Sections sections</summary>
#define thread_safe 

///<summary>Epsilon for very low precision calculation</summary>
constexpr const float SHORTFLOAT_EPSILON = 0.001f;

///<summary>Epsilon for single precision calculation</summary>
constexpr const float FLOAT_EPSILON = 0.000001f;

///<summary>Epsilon for double precision calculation</summary>
constexpr const double DOUBLE_EPSILON = 0.00000000001;

#endif