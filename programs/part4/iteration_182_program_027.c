#include <xmmintrin.h>
#include <emmintrin.h>

__attribute__((noinline, used))
float compare_valid(int comparison_type) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    
    // Validate the comparison type
    int valid_cond = comparison_type & 0x1F;  // Mask to valid SSE range (0-31)
    
    // Or better: use specific predicates
    __m128 c;
    switch(comparison_type) {
        case 0: c = _mm_cmp_ss(a, b, _CMP_EQ_OQ); break;
        case 1: c = _mm_cmp_ss(a, b, _CMP_LT_OS); break;
        case 2: c = _mm_cmp_ss(a, b, _CMP_LE_OS); break;
        // ... other valid predicates
        default: c = _mm_cmp_ss(a, b, _CMP_EQ_OQ); break;  // Default fallback
    }
    
    return _mm_cvtss_f32(c);
}
