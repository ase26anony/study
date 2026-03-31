#include <immintrin.h>

void compute(int *input, int *init, int *mul, int *result) {
    __m128i v_mul = _mm_loadu_si128((__m128i*)mul);
    __m128i v_acc = _mm_loadu_si128((__m128i*)init);

    for (int i = 0; i < 5000; i += 4) {
        // Load 4 consecutive input values (replicate for each lane)
        __m128i v_in0 = _mm_set1_epi32(input[i]);
        __m128i v_in1 = _mm_set1_epi32(input[i+1]);
        __m128i v_in2 = _mm_set1_epi32(input[i+2]);
        __m128i v_in3 = _mm_set1_epi32(input[i+3]);

        // acc = acc * mul + input[i]
        v_acc = _mm_mullo_epi32(v_acc, v_mul);
        v_acc = _mm_add_epi32(v_acc, v_in0);

        v_acc = _mm_mullo_epi32(v_acc, v_mul);
        v_acc = _mm_add_epi32(v_acc, v_in1);

        v_acc = _mm_mullo_epi32(v_acc, v_mul);
        v_acc = _mm_add_epi32(v_acc, v_in2);

        v_acc = _mm_mullo_epi32(v_acc, v_mul);
        v_acc = _mm_add_epi32(v_acc, v_in3);
    }

    _mm_storeu_si128((__m128i*)result, v_acc);
}
