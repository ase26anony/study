#include <immintrin.h>

__attribute__((noinline, used))
float compare_valid(int use_greater) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    
    // Use proper comparison predicates
    int cmp_mode = use_greater ? _CMP_GT_OQ : _CMP_LT_OQ;
    __m128 c = _mm_cmp_ss(a, b, cmp_mode);
    
    return _mm_cvtss_f32(c);
}
