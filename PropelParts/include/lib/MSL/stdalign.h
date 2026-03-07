#ifndef MSL_STDALIGN_H
#define MSL_STDALIGN_H
// Backported from C11/C++11
#define _Alignas(x) __declspec(align(x))
#define alignas(x) _Alignas(x)
#define __alignas_is_defined 1
#define __alignof_is_defined 1
#endif