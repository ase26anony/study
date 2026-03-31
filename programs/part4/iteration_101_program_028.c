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
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Complex operation that may expand to many operands */
NOINLINE v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, 
                               v4si mask1, v4si mask2) {
    volatile v4si temp1, temp2, temp3;
    
    /* Chain of operations that may require many operands */
    /* 1. Vector shuffle with runtime mask - can expand to many operands */
    v4si shuffled = __builtin_shuffle(a, b, mask1);
    BARRIER();
    
    /* 2. Vector conditional expression with comparison */
    v4si cmp_result = (shuffled > c) ? a * b : c + d;
    temp1 = cmp_result;
    BARRIER();
    
    /* 3. Another shuffle with different mask */
    v4si shuffled2 = __builtin_shuffle(cmp_result, d, mask2);
    temp2 = shuffled2;
    BARRIER();
    
    /* 4. Complex arithmetic chain */
    v4si result = (shuffled2 * a) + (b - c) * (d + shuffled);
    temp3 = result;
    BARRIER();
    
    /* 5. Final conditional blend */
    v4si final = (mask1 > mask2) ? result : shuffled2;
    
    return final;
}

#ifdef __AVX__
NOINLINE v8si test_avx_10_operands(v8si a, v8si b, v8si c, v8si d,
                                   v8si mask1, v8si mask2, v8si mask3) {
    volatile v8si temp1, temp2, temp3, temp4;
    
    /* AVX operations that may require many operands */
    /* 1. Multiple shuffles */
    v8si s1 = __builtin_shuffle(a, b, mask1);
    temp1 = s1;
    BARRIER();
    
    v8si s2 = __builtin_shuffle(c, d, mask2);
    temp2 = s2;
    BARRIER();
    
    /* 2. Complex conditional with comparison */
    v8si cmp = (s1 > s2);
    v8si blend = cmp ? s1 * a : s2 * b;
    temp3 = blend;
    BARRIER();
    
    /* 3. Another shuffle with arithmetic */
    v8si s3 = __builtin_shuffle(blend, mask3, mask1);
    v8si result = s3 + (a & b) | (c ^ d);
    temp4 = result;
    BARRIER();
    
    return result;
}
#endif

#ifdef __AVX512F__
/* Test for potential 11 operand expansion */
NOINLINE v16si test_avx512_11_operands(v16si a, v16si b, v16si c, v16si d,
                                       v16si e, v16si mask1, v16si mask2,
                                       v16si mask3) {
    volatile v16si temp[5];
    
    /* Complex chain that may require 11 operands */
    /* 1. Multiple operations chained */
    v16si op1 = __builtin_shuffle(a, b, mask1);
    temp[0] = op1;
    BARRIER();
    
    v16si op2 = __builtin_shuffle(c, d, mask2);
    temp[1] = op2;
    BARRIER();
    
    v16si op3 = __builtin_shuffle(e, mask3, mask1);
    temp[2] = op3;
    BARRIER();
    
    /* 2. Complex conditional expression */
    v16si cmp = (op1 > op2);
    v16si blend1 = cmp ? op1 * a : op2 * b;
    temp[3] = blend1;
    BARRIER();
    
    /* 3. Another conditional with more operands */
    v16si cmp2 = (blend1 < op3);
    v16si final = cmp2 ? (blend1 + op3) * c : (op1 - op2) / d;
    temp[4] = final;
    BARRIER();
    
    return final;
}
#endif

/* Helper to initialize vectors */
void init_vectors(v4si *v, int start) {
    for (int i = 0; i < 4; i++) {
        (*v)[i] = start + i;
    }
}

#ifdef __AVX__
void init_v8si(v8si *v, int start) {
    for (int i = 0; i < 8; i++) {
        (*v)[i] = start + i;
    }
}
#endif

#ifdef __AVX512F__
void init_v16si(v16si *v, int start) {
    for (int i = 0; i < 16; i++) {
        (*v)[i] = start + i;
    }
}
#endif

int main() {
    v4si a, b, c, d, mask1, mask2;
    int result = 0;
    
    /* Initialize vectors with different patterns */
    init_vectors(&a, 1);
    init_vectors(&b, 5);
    init_vectors(&c, 9);
    init_vectors(&d, 13);
    init_vectors(&mask1, 0);  /* Shuffle mask: reverse order */
    mask1 = (v4si){3, 2, 1, 0};
    init_vectors(&mask2, 2);  /* Another shuffle mask */
    mask2 = (v4si){1, 0, 3, 2};
    
    /* Test 10 operand expansion path */
    v4si res1 = test_10_operands(a, b, c, d, mask1, mask2);
    
    /* Compute checksum to prevent elimination */
    for (int i = 0; i < 4; i++) {
        result += res1[i];
    }
    
#ifdef __AVX__
    /* Test with AVX vectors */
    v8si avx_a, avx_b, avx_c, avx_d, avx_m1, avx_m2, avx_m3;
    init_v8si(&avx_a, 1);
    init_v8si(&avx_b, 9);
    init_v8si(&avx_c, 17);
    init_v8si(&avx_d, 25);
    init_v8si(&avx_m1, 0);
    for (int i = 0; i < 8; i++) avx_m1[i] = 7 - i;  /* Reverse */
    init_v8si(&avx_m2, 2);
    for (int i = 0; i < 8; i++) avx_m2[i] = (i + 4) % 8;
    init_v8si(&avx_m3, 1);
    for (int i = 0; i < 8; i++) avx_m3[i] = i * 2 % 8;
    
    v8si res2 = test_avx_10_operands(avx_a, avx_b, avx_c, avx_d, 
                                     avx_m1, avx_m2, avx_m3);
    
    for (int i = 0; i < 8; i++) {
        result += res2[i];
    }
#endif

#ifdef __AVX512F__
    /* Test with AVX-512 vectors (potential 11 operands) */
    v16si avx512_a, avx512_b, avx512_c, avx512_d, avx512_e;
    v16si avx512_m1, avx512_m2, avx512_m3;
    
    init_v16si(&avx512_a, 1);
    init_v16si(&avx512_b, 17);
    init_v16si(&avx512_c, 33);
    init_v16si(&avx512_d, 49);
    init_v16si(&avx512_e, 65);
    init_v16si(&avx512_m1, 0);
    for (int i = 0; i < 16; i++) avx512_m1[i] = 15 - i;
    init_v16si(&avx512_m2, 4);
    for (int i = 0; i < 16; i++) avx512_m2[i] = (i + 8) % 16;
    init_v16si(&avx512_m3, 2);
    for (int i = 0; i < 16; i++) avx512_m3[i] = i * 3 % 16;
    
    v16si res3 = test_avx512_11_operands(avx512_a, avx512_b, avx512_c,
                                         avx512_d, avx512_e,
                                         avx512_m1, avx512_m2, avx512_m3);
    
    for (int i = 0; i < 16; i++) {
        result += res3[i];
    }
#endif
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Return based on result to ensure execution */
    return result != 0 ? 0 : 1;
}
