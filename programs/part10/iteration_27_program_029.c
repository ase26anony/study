#include <immintrin.h>  // or <xmmintrin.h> for SSE

void compute(int* init, int* mul, int* input, int* result) {
    // Load init and mul into SIMD registers
    __m128i v_acc = _mm_loadu_si128((__m128i*)init);   // v_acc = [init[3], init[2], init[1], init[0]]
    __m128i v_mul = _mm_loadu_si128((__m128i*)mul);    // v_mul = [mul[3], mul[2], mul[1], mul[0]]

    for (int i = 0; i < 5000; i++) {
        // Broadcast input[i] to all 4 lanes of a SIMD register
        __m128i v_input = _mm_set1_epi32(input[i]);
        
        // Multiply v_acc by v_mul
        v_acc = _mm_mullo_epi32(v_acc, v_mul);
        
        // Add v_input to v_acc
        v_acc = _mm_add_epi32(v_acc, v_input);
    }

    // Store results back
    _mm_storeu_si128((__m128i*)result, v_acc);
}
