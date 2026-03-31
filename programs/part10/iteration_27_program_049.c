#include <immintrin.h>  // for SSE/AVX intrinsics

// Assume mul and init are arrays of 4 ints
// input is array of 5000 ints
// result is array of 4 ints

__m128i v_acc = _mm_loadu_si128((__m128i*)init);   // load init[0..3]
__m128i v_mul = _mm_loadu_si128((__m128i*)mul);    // load mul[0..3]

for (int i = 0; i < 5000; i++) {
    __m128i v_input = _mm_set1_epi32(input[i]);    // broadcast input[i] to all lanes
    v_acc = _mm_add_epi32(_mm_mullo_epi32(v_acc, v_mul), v_input);
}

_mm_storeu_si128((__m128i*)result, v_acc);
