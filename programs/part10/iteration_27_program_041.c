#include <emmintrin.h>   // SSE2
#include <smmintrin.h>   // SSE4.1

void compute(int init[4], int mul[4], int input[5000], int result[4]) {
    // Load init and mul into SSE registers
    __m128i v_init = _mm_loadu_si128((__m128i*)init);
    __m128i v_mul  = _mm_loadu_si128((__m128i*)mul);
    
    __m128i v_acc = v_init;
    
    for (int i = 0; i < 5000; i++) {
        // Broadcast input[i] to all 4 lanes
        __m128i v_broadcast = _mm_set1_epi32(input[i]);
        // acc = acc * mul + input[i]
        v_acc = _mm_add_epi32(_mm_mullo_epi32(v_acc, v_mul), v_broadcast);
    }
    
    // Store result
    _mm_storeu_si128((__m128i*)result, v_acc);
}
