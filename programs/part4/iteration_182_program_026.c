#include <immintrin.h>

// Valid comparison types for _mm_cmp_ss:
// _CMP_EQ_OQ    = 0  // Equal (ordered, quiet)
// _CMP_LT_OS    = 1  // Less-than (ordered, signaling)
// _CMP_LE_OS    = 2  // Less-than-or-equal (ordered, signaling)
// _CMP_UNORD_Q  = 3  // Unordered (quiet)
// _CMP_NEQ_UQ   = 4  // Not-equal (unordered, quiet)
// _CMP_NLT_US   = 5  // Not-less-than (unordered, signaling)
// _CMP_NLE_US   = 6  // Not-less-than-or-equal (unordered, signaling)
// _CMP_ORD_Q    = 7  // Ordered (quiet)
// ... and more up to 31

__attribute__((noinline, used))
float compare_valid(int cond) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    __m128 c = _mm_cmp_ss(a, b, _CMP_LT_OS); // Valid: less-than comparison
    return _mm_cvtss_f32(c);
}
