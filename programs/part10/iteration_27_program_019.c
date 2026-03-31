#include <immintrin.h>

void compute(int *input, int *init, int *mul, int *result) {
    __m128i acc = _mm_loadu_si128((__m128i*)init);   // load init[0..3]
    __m128i mulv = _mm_loadu_si128((__m128i*)mul);   // load mul[0..3]

    for (int i = 0; i < 5000; i++) {
        __m128i in = _mm_set1_epi32(input[i]);       // broadcast input[i]
        acc = _mm_add_epi32(_mm_mullo_epi32(acc, mulv), in);
    }

    _mm_storeu_si128((__m128i*)result, acc);
}
