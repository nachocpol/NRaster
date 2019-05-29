#pragma once

#if defined(__clang__) || defined(__GNUG__)

	#define hlslpp_inline inline __attribute__((always_inline))

#elif defined(_MSC_VER)

	#define hlslpp_inline __forceinline

#else

	#error No forceinline define for this platform

#endif

#define HLSLPP_SHUFFLE_MASK(X, Y, Z, W)		(((W) << 6) | ((Z) << 4) | ((Y) << 2) | (X))
#define HLSLPP_SHUFFLE_MASK_PD(X, Y)		(((Y) << 1) | (X))

// Create a mask where 1 selects from x, 0 selects from y
#define HLSLPP_BLEND_MASK(X, Y, Z, W)		(~((X) | ((Y) << 1) | ((Z) << 2) | ((W) << 3)) & 0xf)
#define HLSLPP_BLEND_MASK_PD(X, Y)			(~((X) | ((Y) << 1)) & 0x3)

#define HLSLPP_COMPONENT_X(X)				(1 << X)
#define HLSLPP_COMPONENT_XY(X, Y)			((1 << X) | (1 << Y))
#define HLSLPP_COMPONENT_XYZ(X, Y, Z)		((1 << X) | (1 << Y) | (1 << Z))
#define HLSLPP_COMPONENT_XYZW(X, Y, Z, W)	((1 << X) | (1 << Y) | (1 << Z) | (1 << W))

#if defined(_M_ARM) || defined(__arm__) || defined(_M_ARM64) || defined(__aarch64__)
	
	#include "platforms/hlsl++_neon.h"

#elif defined(_XBOX)

	#include "platforms/hlsl++_360.h"

#else

	#include "platforms/hlsl++_sse.h"

#endif

#if defined(__cpp_if_constexpr)
#define HLSLPP_CONSTEXPR_IF(x) if constexpr(x)
#else
#define HLSLPP_CONSTEXPR_IF(x) if(x)
#endif

#include <cstdint>

#include <type_traits>

#include "hlsl++_common.h"

#include "hlsl++_vector_float.h"

#include "hlsl++_matrix_float.h"

#include "hlsl++_vector_int.h"

#include "hlsl++_vector_double.h"

#include "hlsl++_quaternion.h"

// Some functions depend on having the knowledge of types that have been defined before.
// In order to break the header dependencies they are all defined here.
#include "hlsl++_dependent.h"