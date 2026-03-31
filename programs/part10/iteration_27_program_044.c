#include <stdint.h>
#include <immintrin.h>  // for AVX2

void compute(int32_t* input, int32_t* init, int32_t* mul, int32_t* result) {
    // Load init and mul into SIMD registers
    __m128i acc = _mm_loadu_si128((__m128i*)init);   // acc = [init[0], init[1], init[2], init[3]]
    __m128i m   = _mm_loadu_si128((__m128i*)mul);    // m   = [mul[0], mul[1], mul[2], mul[3]]

    for (int i = 0; i < 5000; i++) {
        // Broadcast input[i] to all 4 lanes
        __m128i b = _mm_set1_epi32(input[i]);
        // acc = acc * m + b
        acc = _mm_add_epi32(_mm_mullo_epi32(acc, m), b);
    }

    // Store results
    _mm_storeu_si128((__m128i*)result, acc);
}
