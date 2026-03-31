// This works - immediate value
__m128 c = _mm_cmp_ss(a, b, _CMP_LT_OS);  // Less-than (ordered, signaling)

// This doesn't work - runtime variable
__m128 c = _mm_cmp_ss(a, b, cond);  // ERROR: 'cond' must be immediate
