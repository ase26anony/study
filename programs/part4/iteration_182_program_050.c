#include <xmmintrin.h>

// Valid - using a compile-time constant
float compare_valid() {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    // _CMP_LT_OS = 1 (Less than, ordered, signaling)
    __m128 c = _mm_cmp_ss(a, b, _CMP_LT_OS);
    return _mm_cvtss_f32(c);
}

// If you need runtime selection, use a switch statement
float compare_with_switch(int cond) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    __m128 c;
    
    switch(cond) {
        case 0: c = _mm_cmp_ss(a, b, _CMP_EQ_OQ); break;
        case 1: c = _mm_cmp_ss(a, b, _CMP_LT_OS); break;
        case 2: c = _mm_cmp_ss(a, b, _CMP_LE_OS); break;
        // ... other cases
        default: c = _mm_set_ss(0.0f); break;
    }
    return _mm_cvtss_f32(c);
}
