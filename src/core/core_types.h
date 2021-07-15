#pragma once

#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)

// Unsigned types.
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

// Signed types.
typedef char      s8;
typedef short     s16;
typedef int       s32;
typedef long long s64;

// Floating point types.
typedef float  f32;
typedef double f64;

// Ensure all types are of the correct size.
static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

static_assert(sizeof(s8) == 1, "Expected i8 to be 1 byte.");
static_assert(sizeof(s16) == 2, "Expected i16 to be 2 bytes.");
static_assert(sizeof(s32) == 4, "Expected i32 to be 4 bytes.");
static_assert(sizeof(s64) == 8, "Expected i64 to be 8 bytes.");

static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");