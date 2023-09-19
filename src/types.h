#ifndef WORLD_GENERATOR_TYPES_H
#define WORLD_GENERATOR_TYPES_H

// Properly define static assertions.
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

typedef _Bool b8;
typedef int b32;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;

#define true 1;
#define false 0;

#define CLAMP(value, min, max) (value < min) ? min : ((value > max) ? max : value)

#endif // WORLD_GENERATOR_TYPES_H
