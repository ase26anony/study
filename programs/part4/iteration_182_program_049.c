#include <immintrin.h>

__attribute__((noinline, used))
float compare_valid(int use_less_than) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    
    // Use a valid predicate based on condition
    int predicate = use_less_than ? _CMP_LT_OS : _CMP_GT_OS;
    __m128 c = _mm_cmp_ss(a, b, predicate);
    
    return _mm_cvtss_f32(c);
}
