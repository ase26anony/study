#include <xmmintrin.h>

__attribute__((noinline, used))
float compare_valid(int cond) {
    // Ensure cond is a valid comparison predicate
    if (cond < 0 || cond > 31) {
        cond = _CMP_EQ_OQ;  // Default to equality comparison
    }
    
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    __m128 c = _mm_cmp_ss(a, b, cond);
    return _mm_cvtss_f32(c);
}
