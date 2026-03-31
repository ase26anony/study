#include <xmmintrin.h>

__attribute__((noinline, used))
float compare_valid(int use_equal) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    
    // Choose predicate based on condition
    int predicate = use_equal ? _CMP_EQ_OQ : _CMP_LT_OS;
    
    __m128 c = _mm_cmp_ss(a, b, predicate); // Valid immediate
    return _mm_cvtss_f32(c);
}
