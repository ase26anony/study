/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))
#define BARRIER() asm volatile("" ::: "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

/* Complex shuffle with many operands */
NOINLINE v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    /* Force many operands through complex shuffle chain */
    volatile v4si temp1, temp2, temp3;
    
    /* Shuffle with runtime mask - may expand to many operands */
    v4si shuffled1 = __builtin_shuffle(a, b, mask);
    BARRIER();
    
    /* Another shuffle with different inputs */
    v4si shuffled2 = __builtin_shuffle(c, d, mask);
    BARRIER();
    
    /* Vector conditional with comparison - generates VEC_COND_EXPR */
    v4si cmp = (shuffled1 > shuffled2);
    v4si cond_result = cmp ? (shuffled1 * shuffled2) : (shuffled1 + shuffled2);
    BARRIER();
    
    /* More complex operation chain */
    v4si blended = __builtin_shufflevector(shuffled1, shuffled2, 0, 5, 2, 7);
    BARRIER();
    
    /* Store to volatile to prevent elimination */
    temp1 = shuffled1;
    temp2 = shuffled2;
    temp3 = cond_result;
    
    /* Final complex expression that might need many operands */
    v4si result = (blended * cond_result) + (temp1 - temp2) / (cond_result + 1);
    
    return result;
}

#ifdef __AVX__
NOINLINE v8si test_avx_10_operands(v8si a, v8si b, v8si c, v8si d, v8si mask) {
    volatile v8si temp1, temp2, temp3, temp4;
    
    /* AVX shuffle with 256-bit vectors - more complex expansion */
    v8si shuffled1 = __builtin_shuffle(a, b, mask);
    BARRIER();
    
    v8si shuffled2 = __builtin_shuffle(c, d, mask);
    BARRIER();
    
    /* Multiple comparisons and blends */
    v8si cmp1 = (shuffled1 > shuffled2);
    v8si cmp2 = (shuffled1 < shuffled2);
    BARRIER();
    
    /* Complex conditional with multiple operands */
    v8si blend1 = cmp1 ? (shuffled1 * shuffled2) : (shuffled1 + shuffled2);
    v8si blend2 = cmp2 ? (shuffled1 - shuffled2) : (shuffled1 / (shuffled2 + 1));
    BARRIER();
    
    /* Cross-lane shuffle */
    v8si cross_shuffle = __builtin_shufflevector(
        blend1, blend2, 0, 9, 2, 11, 4, 13, 6, 15);
    BARRIER();
    
    /* Store intermediates */
    temp1 = shuffled1;
    temp2 = shuffled2;
    temp3 = blend1;
    temp4 = blend2;
    
    /* Final expression with many operands */
    v8si result = cross_shuffle + 
                  (temp1 * temp2) - 
                  (temp3 / (temp4 + 1)) + 
                  (blend1 & blend2) | 
                  (blend1 ^ blend2);
    
    return result;
}
#endif

#ifdef __AVX512F__
/* Test with AVX-512 - potentially triggers 11 operand paths */
NOINLINE v16si test_avx512_11_operands(v16si a, v16si b, v16si c, 
                                       v16si d, v16si e, v16si mask) {
    volatile v16si temp[6];
    
    /* Multiple shuffles with different masks */
    v16si shuffle1 = __builtin_shuffle(a, b, mask);
    v16si shuffle2 = __builtin_shuffle(c, d, mask);
    v16si shuffle3 = __builtin_shuffle(e, a, mask);
    BARRIER();
    
    /* Complex comparison chain */
    v16si cmp1 = (shuffle1 > shuffle2);
    v16si cmp2 = (shuffle2 < shuffle3);
    v16si cmp3 = (shuffle1 == shuffle3);
    BARRIER();
    
    /* Nested conditional expressions - each needs many operands */
    v16si cond1 = cmp1 ? (shuffle1 * shuffle2) : (shuffle1 + shuffle2);
    v16si cond2 = cmp2 ? (shuffle2 - shuffle3) : (shuffle2 / (shuffle3 + 1));
    v16si cond3 = cmp3 ? (cond1 & cond2) : (cond1 | cond2);
    BARRIER();
    
    /* Store all intermediates */
    temp[0] = shuffle1;
    temp[1] = shuffle2;
    temp[2] = shuffle3;
    temp[3] = cond1;
    temp[4] = cond2;
    temp[5] = cond3;
    
    /* Very complex final expression - likely needs 11+ operands */
    v16si result = (temp[0] * temp[1]) +
                   (temp[2] - temp[3]) +
                   (temp[4] / (temp[5] + 1)) +
                   (cond1 & cond2) +
                   (cond2 | cond3) +
                   (shuffle1 ^ shuffle2) +
                   (shuffle2 & shuffle3);
    
    return result;
}
#endif

/* Helper to initialize vectors */
void init_vectors(v4si *v, int base) {
    for (int i = 0; i < 4; i++) {
        (*v)[i] = base + i;
    }
}

#ifdef __AVX__
void init_v8si(v8si *v, int base) {
    for (int i = 0; i < 8; i++) {
        (*v)[i] = base + i;
    }
}
#endif

#ifdef __AVX512F__
void init_v16si(v16si *v, int base) {
    for (int i = 0; i < 16; i++) {
        (*v)[i] = base + i;
    }
}
#endif

int main() {
    int checksum = 0;
    
    /* Test SSE/SSE2 paths */
    v4si a, b, c, d, mask;
    init_vectors(&a, 1);
    init_vectors(&b, 5);
    init_vectors(&c, 9);
    init_vectors(&d, 13);
    init_vectors(&mask, 0);  /* Simple mask: 0,1,2,3 */
    
    v4si result1 = test_10_operands(a, b, c, d, mask);
    
    /* Extract checksum */
    for (int i = 0; i < 4; i++) {
        checksum += result1[i];
    }
    
#ifdef __AVX__
    /* Test AVX paths */
    v8si avx_a, avx_b, avx_c, avx_d, avx_mask;
    init_v8si(&avx_a, 1);
    init_v8si(&avx_b, 9);
    init_v8si(&avx_c, 17);
    init_v8si(&avx_d, 25);
    init_v8si(&avx_mask, 0);
    
    v8si result2 = test_avx_10_operands(avx_a, avx_b, avx_c, avx_d, avx_mask);
    
    for (int i = 0; i < 8; i++) {
        checksum += result2[i];
    }
#endif

#ifdef __AVX512F__
    /* Test AVX-512 paths - most likely to trigger 11 operand case */
    v16si avx512_a, avx512_b, avx512_c, avx512_d, avx512_e, avx512_mask;
    init_v16si(&avx512_a, 1);
    init_v16si(&avx512_b, 17);
    init_v16si(&avx512_c, 33);
    init_v16si(&avx512_d, 49);
    init_v16si(&avx512_e, 65);
    init_v16si(&avx512_mask, 0);
    
    v16si result3 = test_avx512_11_operands(avx512_a, avx512_b, avx512_c,
                                           avx512_d, avx512_e, avx512_mask);
    
    for (int i = 0; i < 16; i++) {
        checksum += result3[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum > 0) ? 0 : 1;
}
