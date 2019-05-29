#pragma once

#include <ppcintrinsics.h>

typedef __vector4 n128;
typedef __vector4 n128i; // Xbox 360 has a __vector4i type but it's not actually backed by hardware

//----------
// Functions
//----------

hlslpp_inline __vector4 __vset1(float x)
{
	__vector4 v = { x, x, x, x };
	return v;
}

hlslpp_inline __vector4 __vset(float x, float y, float z, float w)
{
	__vector4 v = { x, y, z, w };
	return v;
}

hlslpp_inline __vector4 __vsqrt(__vector4 x)
{
	__vector4 half		= __vset1(0.5f);
	__vector4 estimate 	= __vrsqrtefp(x); // First estimate
	__vector4 cmp_zero 	= __vor(__vcmpeqfp(x, __vzero()), __vcmpeqfp(estimate, __vzero())); // Compare with zero (both input and estimate as __vrsqrtefp(infinity) == 0)
	__vector4 y 		= __vmulfp(estimate, x);
	__vector4 y_half 	= __vmulfp(y, half);
	
	// If either x or the estimate are 0, return 0 as we may have caused a NaN
	return __vsel(__vmaddfp(__vnmsubfp(estimate, y, __vspltisw(1)), y_half, y), __vzero(), cmp_zero);
}

hlslpp_inline __vector4 __vrsqrt(__vector4 x)
{
	__vector4 estimate = __vrsqrtefp(x); // First estimate
	
	// Newton-Raphson refinement
	__vector4 half = __vset1(0.5f);
	__vector4 x_half 	= __vmulfp(x, half);						// x / 2.0
    __vector4 est_2 	= __vmulfp(estimate, estimate);				// estimate^2
    __vector4 scale 	= __vnmsubfp(x_half, est_2, half);			// x / 2.0 - estimate^2 / 2.0
    __vector4 refined 	= __vmaddfp(estimate, scale, estimate);		// estimate * (x / 2.0 - estimate^2 / 2.0) + estimate

    return __vsel(estimate, refined, __vcmpeqfp(refined, refined)); // If refined is NaN use estimate, else use refined
}

hlslpp_inline __vector4 __vrcp(__vector4 x)
{
	__vector4 estimate = __vrefp(x);
	__vector4 refined = __vmaddfp(__vnmsubfp(x, estimate, __vspltisw(1)), estimate, estimate);
	return __vsel(estimate, refined, __vcmpeqfp(refined, refined)); // If refined is NaN use estimate, else use refined
}

//------
// Float
//------

#define _hlslpp_set1_ps(x)						__vset1((x))
#define _hlslpp_set_ps(x, y, z, w)				__vset((x), (y), (z), (w))
#define _hlslpp_setzero_ps()					__vzero()

#define _hlslpp_add_ps(x, y)					__vaddfp((x), (y))
#define _hlslpp_sub_ps(x, y)					__vsubfp((x), (y))
#define _hlslpp_mul_ps(x, y)					__vmulfp((x), (y))
#define _hlslpp_div_ps(x, y)					__vmulfp((x), __vrcp(y))

#define _hlslpp_add_ss(x, y)					__vaddfp((x), (y))
#define _hlslpp_sub_ss(x, y)					__vsubfp((x), (y))
#define _hlslpp_mul_ss(x, y)					__vmulfp((x), (y))
#define _hlslpp_div_ss(x, y)					__vmulfp((x), __vrcp(y))

#define _hlslpp_rcp_ps(x)						__vrcp((x))

#define _hlslpp_neg_ps(x)						__vxor((x), __vset1(negMask.f))

#define _hlslpp_madd_ps(x, y, z)				__vmaddfp((x), (y), (z))
#define _hlslpp_msub_ps(x, y, z)				_hlslpp_neg_ps(__vnmsubfp((z), (x), (y))) // Negate because __vnmsubfp does z - x * y and we want x * y - z
#define _hlslpp_subm_ps(x, y, z)				__vnmsubfp((z), (x), (y))

#define _hlslpp_abs_ps(x)						__vand(f4absMask, (x))

#define _hlslpp_rsqrt_ps(x)						__vrsqrt(x)
#define _hlslpp_sqrt_ps(x)						__vsqrt(x)

#define _hlslpp_cmpeq_ps(x, y)					__vcmpeqfp((x), (y))
#define _hlslpp_cmpneq_ps(x, y)					__vxor(__vcmpeqfp((x), (y)), __vset1(fffMask.f))

#define _hlslpp_cmpgt_ps(x, y)					__vcmpgtfp((x), (y))
#define _hlslpp_cmpge_ps(x, y)					__vcmpgefp((x), (y))

// Note intrinsic is the same but arguments are inverted
#define _hlslpp_cmplt_ps(x, y)					__vcmpgtfp((y), (x))
#define _hlslpp_cmple_ps(x, y)					__vcmpgefp((y), (x))

#define _hlslpp_max_ps(x, y)					__vmaxfp((x), (y))
#define _hlslpp_min_ps(x, y)					__vminfp((x), (y))

#define _hlslpp_trunc_ps(x)						__vrfiz((x))
#define _hlslpp_floor_ps(x)						__vrfim((x))
#define _hlslpp_ceil_ps(x)						__vrfip((x))
#define _hlslpp_round_ps(x)						__vrfin((x))

#define _hlslpp_frac_ps(x)						__vsubfp((x), __vrfim(x))

#define _hlslpp_clamp_ps(x, minx, maxx)			__vmaxfp(__vminfp((x), (maxx)), (minx))
#define _hlslpp_sat_ps(x)						__vmaxfp(__vminfp((x), __vspltisw(1)), __vzero())

#define _hlslpp_and_ps(x, y)					__vand((x), (y))
#define _hlslpp_andnot_ps(x, y)					__vandc((y), (x))
#define _hlslpp_or_ps(x, y)						__vor((x), (y))
#define _hlslpp_xor_ps(x, y)					__vxor((x), (y))

#define _hlslpp_movelh_ps(x, y)					__vmrglw((y), (x))
#define _hlslpp_movehl_ps(x, y)					__vmrghw((x), (y))
#define _hlslpp_movehdup_ps(x)					__vpermwi((x), VPERMWI_CONST(1, 1, 3, 3))

#define _hlslpp_perm_ps(x, mask)				__vpermwi((x), mask)
#define _hlslpp_shuffle_ps(x, y, mask)			__vperm((x), (y), __vset(mask & 3, (mask >> 2) & 3, (mask >> 4) & 3, (mask >> 6) & 3))

#define _hlslpp_sel_ps(x, y, mask)				__vsel((x), (y), (mask))

#define _hlslpp_blend_ps(x, y, mask)			__vsel((x), (y), __vset(~((mask & 1) * 0xffffffff), ~(((mask >> 1) & 1) * 0xffffffff), ~(((mask >> 2) & 1) * 0xffffffff), ~(((mask >> 3) & 1) * 0xffffffff)))

#define _hlslpp_dot3_ps(x, y)					__vmsum3fp((x), (y))

#define _hlslpp_dot4_ps(x, y)					__vmsum4fp((x), (y))

//--------
// Integer
//--------

#define _hlslpp_set1_epi32(x)					__vcfpsxws(__vset1(float(x)), 0)
#define _hlslpp_set_epi32(x, y, z, w)			__vcfpsxws(__vset(float(x), float(y), float(z), float(w)), 0)

#define _hlslpp_add_epi32(x, y)					__vaddsws((x), (y))
#define _hlslpp_sub_epi32(x, y)					__vsubsws((x), (y))

// VMX128 removed integer multiplication to make space for float multiplication, so cast, multiply and cast back
#define _hlslpp_mul_epi32(x, y)					__vcfpsxws(__vmulfp(__vcsxwfp((x), 0), __vcsxwfp((y), 0)), 0)

#define _hlslpp_div_epi32(x, y)					__vcfpsxws(__vmulfp(__vcsxwfp((x), 0), __vrcp(__vcsxwfp((y), 0))), 0)

#define _hlslpp_neg_epi32(x)					__vaddsws(__vxor((x), i4fffMask), __vcfpsxws(__vset1(1), 0))

#define _hlslpp_madd_epi32(x, y, z)				__vcfpsxws(__vmaddfp(__vcsxwfp((x), 0), __vcsxwfp((y), 0), __vcsxwfp((z), 0)), 0)
#define _hlslpp_msub_epi32(x, y, z)				__vcfpsxws(_hlslpp_neg_ps(__vnmsubfp(__vcsxwfp((z), 0), __vcsxwfp((x), 0), __vcsxwfp((y), 0))))
#define _hlslpp_subm_epi32(x, y, z)				__vcfpsxws(__vnmsubfp(__vcsxwfp((z), 0), __vcsxwfp((x), 0), __vcsxwfp((y), 0)))

#define _hlslpp_abs_epi32(x)					__vand((x), i4absMask)

#define _hlslpp_cmpeq_epi32(x, y)				__vcmpequw((x), (y))
#define _hlslpp_cmpneq_epi32(x, y)				__vxor(__vcmpequw((x), (y)), __vset1(fffMask.f))

#define _hlslpp_cmpgt_epi32(x, y)				__vcmpgtsw((x), (y))
#define _hlslpp_cmpge_epi32(x, y)				__vor(__vcmpgtsw((x), (y)), __vcmpequw((x), (y)))

// Note intrinsic is the same but arguments are inverted
#define _hlslpp_cmplt_epi32(x, y)				__vcmpgtsw((y), (x))
#define _hlslpp_cmple_epi32(x, y)				__vor(__vcmpgtsw((y), (x)), __vcmpequw((x), (y)))

#define _hlslpp_max_epi32(x, y)					__vmaxsw((x), (y))
#define _hlslpp_min_epi32(x, y)					__vminsw((x), (y))

#define _hlslpp_clamp_epi32(x, minx, maxx)		__vmaxsw(__vminsw((x), (maxx)), (minx))
#define _hlslpp_sat_epi32(x)					__vmaxsw(__vminsw((x), i4_1), i4_0)

#define _hlslpp_and_si128(x, y)					__vand((x), (y))
#define _hlslpp_or_si128(x, y)					__vor((x), (y))

#define _hlslpp_perm_epi32(x, mask)				__vpermwi((x), mask)
#define _hlslpp_shuffle_epi32(x, y, mask)		__vperm((x), (y), __vset(mask & 3, (mask >> 2) & 3, (mask >> 4) & 3, (mask >> 6) & 3))

#define _hlslpp_blend_epi32(x, y, mask)			__vsel((x), (y), __vset(~((mask & 1) * 0xffffffff), ~(((mask >> 1) & 1) * 0xffffffff), ~(((mask >> 2) & 1) * 0xffffffff), ~(((mask >> 3) & 1) * 0xffffffff)))

// There are no intrinsics to reinterpret cast like these do as integer and float are all in the same __vector4 structure
#define _hlslpp_castps_si128(x)					((x))
#define _hlslpp_castsi128_ps(x)					((x))

#define _hlslpp_cvtps_epi32(x)					__vcfpsxws((x), 0)
#define _hlslpp_cvtepi32_ps(x)					__vcsxwfp((x), 0)

#define _hlslpp_slli_epi32(x, y)				__vslw((x), __vset1(y))
#define _hlslpp_srli_epi32(x, y)				__vsrw((x), __vset1(y))

#define _hlslpp_sllv_epi32(x, y)				__vslw((x), (y))
#define _hlslpp_srlv_epi32(x, y)				__vsrw((x), (y))

#if !defined(XM_CRMASK_CR6TRUE)
#define XM_CRMASK_CR6TRUE  (1 << 7)
#endif

#if !defined(XM_CRMASK_CR6FALSE)
#define XM_CRMASK_CR6FALSE (1 << 5)
#endif

hlslpp_inline bool _hlslpp_any1_ps(n128 x)
{
	unsigned int ctrl;
	__vcmpeqfpR(__vpermwi(x, VPERMWI_CONST(0, 0, 0, 0)), __vzero(), &ctrl);
	return (ctrl & XM_CRMASK_CR6TRUE) != XM_CRMASK_CR6TRUE;
}

hlslpp_inline bool _hlslpp_any2_ps(n128 x)
{
	unsigned int ctrl;
	__vcmpeqfpR(__vpermwi(x, VPERMWI_CONST(0, 1, 0, 1)), __vzero(), &ctrl);
	return (ctrl & XM_CRMASK_CR6TRUE) != XM_CRMASK_CR6TRUE;
}

hlslpp_inline bool _hlslpp_any3_ps(n128 x)
{
	unsigned int ctrl;
	__vcmpeqfpR(__vpermwi(x, VPERMWI_CONST(0, 1, 2, 0)), __vzero(), &ctrl);
	return (ctrl & XM_CRMASK_CR6TRUE) != XM_CRMASK_CR6TRUE;
}

hlslpp_inline bool _hlslpp_any4_ps(n128 x)
{
	unsigned int ctrl;
	__vcmpeqfpR(x, __vzero(), &ctrl);
	return (ctrl & XM_CRMASK_CR6TRUE) != XM_CRMASK_CR6TRUE;
}

hlslpp_inline bool _hlslpp_all1_ps(n128 x)
{
	unsigned int ctrl;
	__vcmpeqfpR(__vpermwi(x, VPERMWI_CONST(0, 0, 0, 0)), __vzero(), &ctrl);
	return (ctrl & XM_CRMASK_CR6FALSE) == XM_CRMASK_CR6FALSE;
}

hlslpp_inline bool _hlslpp_all2_ps(n128 x)
{
	unsigned int ctrl;
	__vcmpeqfpR(__vpermwi(x, VPERMWI_CONST(0, 1, 2, 0)), __vzero(), &ctrl);
	return (ctrl & XM_CRMASK_CR6FALSE) == XM_CRMASK_CR6FALSE;
}

hlslpp_inline bool _hlslpp_all3_ps(n128 x)
{
	unsigned int ctrl;
	__vcmpeqfpR(__vpermwi(x, VPERMWI_CONST(0, 1, 0, 1)), __vzero(), &ctrl);
	return (ctrl & XM_CRMASK_CR6FALSE) == XM_CRMASK_CR6FALSE;
}

hlslpp_inline bool _hlslpp_all4_ps(n128 x)
{
	unsigned int ctrl;
	__vcmpeqfpR(x, __vzero(), &ctrl);
	return (ctrl & XM_CRMASK_CR6FALSE) == XM_CRMASK_CR6FALSE;
}

//--------
// Storing
//--------

hlslpp_inline void _hlslpp_store1_ps(float* p, n128 x)
{
	__stvewx(x, p, 0);
}

hlslpp_inline void _hlslpp_store2_ps(float* p, n128 x)
{
	__stvewx(x, p, 0);
	__stvewx(__vpermwi(x, 1), p, 4);
}

hlslpp_inline void _hlslpp_store3_ps(float* p, n128 x)
{
	__stvewx(x, p, 0);
	__stvewx(__vpermwi(x, 1), p, 4);
	__stvewx(__vpermwi(x, 2), p, 8);
}

hlslpp_inline void _hlslpp_store4_ps(float* p, n128 x)
{
	__stvlx(x, p, 0);
	__stvrx(x, p, 16);
}

hlslpp_inline void _hlslpp_store3x3_ps(float* p, n128 x0, n128 x1, n128 x2)
{
	
}

hlslpp_inline void _hlslpp_store4x4_ps(float* p, n128 x0, n128 x1, n128 x2, n128 x3)
{
	__stvlx(x0, p, 0);
	__stvrx(x0, p, 16);
	__stvlx(x1, p + 4, 0);
	__stvrx(x1, p + 4, 16);
	__stvlx(x2, p + 8, 0);
	__stvrx(x2, p + 8, 16);
	__stvlx(x3, p + 12, 0);
	__stvrx(x3, p + 12, 16);
}