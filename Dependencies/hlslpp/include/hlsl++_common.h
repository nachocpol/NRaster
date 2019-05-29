namespace hlslpp
{
	union BitMask
	{
		uint32_t i;
		float f;
		explicit BitMask(uint32_t i) : i(i) {}
		explicit BitMask(float f) : f(f) {}
	};

	const BitMask fffMask(0xffffffffu); // Negative NaN
	const BitMask nanMask(0x7fffffffu); // Positive NaN
	const BitMask infMask(0x7f800000u);
	const BitMask minusinfMask(0xff800000u);
	const BitMask absMask(0x7fffffffu);
	const BitMask negMask(0x80000000u);

	//----------------
	// Float Constants
	//----------------

	const n128 f4_0           = _hlslpp_set1_ps(0.0f);
	const n128 f4_1           = _hlslpp_set1_ps(1.0f);
	const n128 f4minusOne     = _hlslpp_set1_ps(-1.0f);
	const n128 f4_05          = _hlslpp_set1_ps(0.5f);
	const n128 f4_minus05     = _hlslpp_set1_ps(-0.5f);
	const n128 f4_2           = _hlslpp_set1_ps(2.0f);
	const n128 f4_minus2      = _hlslpp_set1_ps(-2.0f);
	const n128 f4_3           = _hlslpp_set1_ps(3.0f);
	const n128 f4_10          = _hlslpp_set1_ps(10.0f);
	const n128 f4_e           = _hlslpp_set1_ps(2.718281828f);

	const n128 f4_pi          = _hlslpp_set1_ps(3.14159265f);  // pi
	const n128 f4_minusPi     = _hlslpp_set1_ps(-3.14159265f); // -pi
	const n128 f4_invPi       = _hlslpp_set1_ps(0.31830988f);  // 1 / pi

	const n128 f4_2pi         = _hlslpp_set1_ps(6.28318530f);  //  2 * pi
	const n128 f4_minus2pi    = _hlslpp_set1_ps(-6.28318530f); // -2 * pi
	const n128 f4_inv2pi      = _hlslpp_set1_ps(0.15915494f);  // 1 / (2 * pi)

	const n128 f4_pi2         = _hlslpp_set1_ps(1.57079632f);  //  pi / 2
	const n128 f4_minusPi2    = _hlslpp_set1_ps(-1.57079632f); // -pi / 2
	const n128 f4_invPi2      = _hlslpp_set1_ps(0.63661977f);  // 2 / pi

	const n128 f4_3pi2        = _hlslpp_set1_ps(4.71238898f);  //  3 * pi / 2
	const n128 f4_minus3pi2   = _hlslpp_set1_ps(-4.71238898f); // -3 * pi / 2

	const n128 f4_pi4         = _hlslpp_set1_ps(0.78539816f);  // pi / 4
	const n128 f4_minusPi4    = _hlslpp_set1_ps(-0.78539816f); // -pi / 4

	const n128 f4_NaN         = _hlslpp_set1_ps(nanMask.f);      // Quiet NaN
	const n128 f4_inf         = _hlslpp_set1_ps(infMask.f);      // Infinity
	const n128 f4_minusinf    = _hlslpp_set1_ps(minusinfMask.f); // -Infinity
	const n128 f4_fff         = _hlslpp_set1_ps(fffMask.f);      // 0xffffffff

	const n128 f4_rad2deg     = _hlslpp_set1_ps(180.0f / 3.14159265f);
	const n128 f4_deg2rad     = _hlslpp_set1_ps(3.14159265f / 180.f);

	const n128 f4negativeMask = _hlslpp_set1_ps(negMask.f);
	const n128 f4absMask      = _hlslpp_set1_ps(absMask.f);

	//------------------
	// Integer Constants
	//------------------

	const n128i i4_0           = _hlslpp_set1_epi32(0);
	const n128i i4_1           = _hlslpp_set1_epi32(1);

	const n128i i4negativeMask = _hlslpp_set1_epi32(negMask.i);
	const n128i i4absMask      = _hlslpp_set1_epi32(absMask.i);
	const n128i i4fffMask      = _hlslpp_set1_epi32(fffMask.i);

	static const int IdentityMask  = ((3 << 6) | (2 << 4) | (1 << 2) | 0);
	static const int IdentityMask2 = (1 << 1) | 0;

	const uint32_t MaskX = 0;
	const uint32_t MaskY = 1;
	const uint32_t MaskZ = 2;
	const uint32_t MaskW = 3;

	// Auxiliary templates for disambiguation with standard header functions

	#define hlslpp_enable_if_return(T, R) typename std::enable_if<std::is_arithmetic<T>::value, R>::type
	
	#define hlslpp_enable_if_number(T) typename std::enable_if<std::is_arithmetic<T>::value, void*>::type = nullptr
	
	#define hlslpp_enable_if_number_2(T1, T2) \
			typename std::enable_if< \
			std::is_arithmetic<T1>::value * \
			std::is_arithmetic<T2>::value, void*>::type = nullptr
	
	#define hlslpp_enable_if_number_3(T1, T2, T3) \
			typename std::enable_if< \
			std::is_arithmetic<T1>::value * \
			std::is_arithmetic<T2>::value * \
			std::is_arithmetic<T3>::value, void*>::type = nullptr
	
	#define hlslpp_enable_if_number_4(T1, T2, T3, T4) \
			typename std::enable_if< \
			std::is_arithmetic<T1>::value * \
			std::is_arithmetic<T2>::value * \
			std::is_arithmetic<T3>::value * \
			std::is_arithmetic<T4>::value, void*>::type = nullptr

	// Helper intrinsics

	#define _hlslpp_perm_xxxx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskX, MaskX))
	#define _hlslpp_perm_xxyx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskY, MaskX))
	#define _hlslpp_perm_xxyz_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskY, MaskZ))
	#define _hlslpp_perm_xyxy_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskY, MaskX, MaskY))
	#define _hlslpp_perm_xyww_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskY, MaskW, MaskW))
	#define _hlslpp_perm_xzyw_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskZ, MaskY, MaskW))
	#define _hlslpp_perm_xzxy_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskZ, MaskX, MaskY))
	#define _hlslpp_perm_xzwx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskZ, MaskW, MaskX))
	#define _hlslpp_perm_xwxw_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskW, MaskX, MaskW))
	#define _hlslpp_perm_yxxx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskX, MaskX, MaskX))
	#define _hlslpp_perm_yxwx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskX, MaskW, MaskX))
	#define _hlslpp_perm_yxwz_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskX, MaskW, MaskZ))
	#define _hlslpp_perm_yyyy_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskY, MaskY, MaskY))
	#define _hlslpp_perm_yzxx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskZ, MaskX, MaskX))
	#define _hlslpp_perm_yzxw_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskZ, MaskX, MaskW))
	#define _hlslpp_perm_yzwx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskZ, MaskW, MaskX))
	#define _hlslpp_perm_zxxx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskX, MaskX, MaskX))
	#define _hlslpp_perm_zxyw_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskX, MaskY, MaskW))
	#define _hlslpp_perm_zyyx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskY, MaskY, MaskX))
	#define _hlslpp_perm_zyzy_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskY, MaskZ, MaskY))
	#define _hlslpp_perm_zzyx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskZ, MaskY, MaskX))
	#define _hlslpp_perm_zzzz_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskZ, MaskZ, MaskZ))
	#define _hlslpp_perm_zwzw_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskW, MaskZ, MaskW))
	#define _hlslpp_perm_wyzx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskW, MaskY, MaskZ, MaskX))
	#define _hlslpp_perm_wzxx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskW, MaskZ, MaskX, MaskX))
	#define _hlslpp_perm_wwyx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskW, MaskW, MaskY, MaskX))
	#define _hlslpp_perm_wwzx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskW, MaskW, MaskZ, MaskX))
	#define _hlslpp_perm_wwzw_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskW, MaskW, MaskZ, MaskW))
	#define _hlslpp_perm_wwww_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskW, MaskW, MaskW, MaskW))
	#define _hlslpp_perm_xxyy_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskY, MaskY))
	#define _hlslpp_perm_zwxx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskW, MaskX, MaskX))
	#define _hlslpp_perm_xyxx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskY, MaskX, MaskX))
	#define _hlslpp_perm_zxwy_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskX, MaskW, MaskY))
	#define _hlslpp_perm_yzzy_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskZ, MaskZ, MaskY))
	#define _hlslpp_perm_zxyz_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskX, MaskY, MaskZ))
	#define _hlslpp_perm_yzxz_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskY, MaskZ, MaskX, MaskZ))
	#define _hlslpp_perm_zyzx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskZ, MaskY, MaskZ, MaskX))
	#define _hlslpp_perm_wwwx_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskW, MaskW, MaskW, MaskX))
	#define _hlslpp_perm_wxyz_ps(x)		_hlslpp_perm_ps((x), HLSLPP_SHUFFLE_MASK(MaskW, MaskX, MaskY, MaskZ))
	
	#define _hlslpp_shuf_xxxx_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskX, MaskX))
	#define _hlslpp_shuf_xxxy_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskX, MaskY))
	#define _hlslpp_shuf_xyxx_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskY, MaskX, MaskX))
	#define _hlslpp_shuf_xyxy_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskY, MaskX, MaskY))
	#define _hlslpp_shuf_yzyz_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskY, MaskZ, MaskY, MaskZ))
	#define _hlslpp_shuf_xzxw_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskZ, MaskX, MaskW))
	#define _hlslpp_shuf_ywyw_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskY, MaskW, MaskY, MaskW))
	#define _hlslpp_shuf_ywzw_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskY, MaskW, MaskZ, MaskW))
	#define _hlslpp_shuf_zwzw_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskZ, MaskW, MaskZ, MaskW))
	#define _hlslpp_shuf_xzxz_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskZ, MaskX, MaskZ))
	#define _hlslpp_shuf_wzxx_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskW, MaskZ, MaskX, MaskX))
	#define _hlslpp_shuf_ywzy_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskY, MaskW, MaskZ, MaskY))
	#define _hlslpp_shuf_xzxx_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskZ, MaskX, MaskX))
	#define _hlslpp_shuf_ywyy_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskY, MaskW, MaskY, MaskY))
	#define _hlslpp_shuf_yyyy_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskY, MaskY, MaskY, MaskY))
	#define _hlslpp_shuf_zyzy_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskZ, MaskY, MaskZ, MaskY))
	#define _hlslpp_shuf_xwxw_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskW, MaskX, MaskW))
	#define _hlslpp_shuf_wxwx_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskW, MaskX, MaskW, MaskX))
	#define _hlslpp_shuf_zxxz_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskZ, MaskX, MaskX, MaskZ))
	#define _hlslpp_shuf_wyyw_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskW, MaskY, MaskY, MaskW))
	#define _hlslpp_shuf_xzzx_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskZ, MaskZ, MaskX))
	#define _hlslpp_shuf_ywwy_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskY, MaskW, MaskW, MaskY))
	#define _hlslpp_shuf_yxwz_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskY, MaskX, MaskW, MaskZ))
	#define _hlslpp_shuf_zzxx_ps(x, y)	_hlslpp_shuffle_ps((x), (y), HLSLPP_SHUFFLE_MASK(MaskZ, MaskZ, MaskX, MaskX))
	
	// Reference http://www.liranuna.com/sse-intrinsics-optimizations-in-popular-compilers/
	#define _hlslpp_sign_ps(val)				_hlslpp_and_ps(_hlslpp_or_ps(_hlslpp_and_ps((val), f4minusOne), f4_1), _hlslpp_cmpneq_ps((val), f4_0))
	
	#define _hlslpp_cmpneq1_ps(val1, val2)		_hlslpp_and_ps(_hlslpp_cmpneq_ps((val1), (val2)), f4_1)
	#define _hlslpp_cmpeq1_ps(val1, val2)		_hlslpp_and_ps(_hlslpp_cmpeq_ps((val1), (val2)), f4_1)
	
	#define _hlslpp_cmpgt1_ps(val1, val2)		_hlslpp_and_ps(_hlslpp_cmpgt_ps((val1), (val2)), f4_1)
	#define _hlslpp_cmpge1_ps(val1, val2)		_hlslpp_and_ps(_hlslpp_cmpge_ps((val1), (val2)), f4_1)
	
	#define _hlslpp_cmplt1_ps(val1, val2)		_hlslpp_and_ps(_hlslpp_cmplt_ps((val1), (val2)), f4_1)
	#define _hlslpp_cmple1_ps(val1, val2)		_hlslpp_and_ps(_hlslpp_cmple_ps((val1), (val2)), f4_1)

	#define _hlslpp_perm_xxxx_epi32(x)			_hlslpp_perm_epi32((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskX, MaskX))
	#define _hlslpp_perm_xxyx_epi32(x)			_hlslpp_perm_epi32((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskY, MaskX))
	#define _hlslpp_perm_xxyz_epi32(x)			_hlslpp_perm_epi32((x), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskY, MaskZ))

	#define _hlslpp_shuf_xxxx_epi32(x, y)		_hlslpp_shuffle_epi32((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskX, MaskX))
	#define _hlslpp_shuf_xyxx_epi32(x, y)		_hlslpp_shuffle_epi32((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskY, MaskX, MaskX))
	#define _hlslpp_shuf_xxxy_epi32(x, y)		_hlslpp_shuffle_epi32((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskX, MaskX, MaskY))
	#define _hlslpp_shuf_xyxy_epi32(x, y)		_hlslpp_shuffle_epi32((x), (y), HLSLPP_SHUFFLE_MASK(MaskX, MaskY, MaskX, MaskY))

	#define _hlslpp_cmpneq1_epi32(val1, val2)	_hlslpp_and_si128(_hlslpp_cmpneq_epi32((val1), (val2)), i4_1)
	#define _hlslpp_cmpeq1_epi32(val1, val2)	_hlslpp_and_si128(_hlslpp_cmpeq_epi32((val1), (val2)), i4_1)
	
	#define _hlslpp_cmpgt1_epi32(val1, val2)	_hlslpp_and_si128(_hlslpp_cmpgt_epi32((val1), (val2)), i4_1)
	#define _hlslpp_cmpge1_epi32(val1, val2)	_hlslpp_and_si128(_hlslpp_cmpge_epi32((val1), (val2)), i4_1)
	
	#define _hlslpp_cmplt1_epi32(val1, val2)	_hlslpp_and_si128(_hlslpp_cmplt_epi32((val1), (val2)), i4_1)
	#define _hlslpp_cmple1_epi32(val1, val2)	_hlslpp_and_si128(_hlslpp_cmple_epi32((val1), (val2)), i4_1)

	// Forward declarations

	struct float1;
	struct float2;
	struct float3;
	struct float4;

	struct int1;
	struct int2;
	struct int3;
	struct int4;

	struct float1x1;

	struct double1;
	struct double2;
	struct double3;
	struct double4;
}