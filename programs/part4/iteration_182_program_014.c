// Valid - immediate known at compile time
__m128 c = _mm_cmp_ss(a, b, _CMP_LT_OS);  // Less than (ordered, signaling)

// Or using the numeric immediate
__m128 c = _mm_cmp_ss(a, b, 1);  // 1 = _CMP_LT_OS
