/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization */
#define NOOPT __attribute__((noinline, noclone))
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

/* Complex shuffle with runtime mask - may expand to many operands */
NOOPT v4si shuffle_complex(v4si a, v4si b, v4si mask) {
    /* Force non-constant shuffle */
    v4si result;
    /* This shuffle with variable mask may require many operands during expansion */
    result = __builtin_shuffle(a, b, mask);
    BARRIER();
    return result;
}

/* Vector conditional expression with complex operations */
NOOPT v4df conditional_ops(v4df a, v4df b, v4df c, v4df d, v4df mask) {
    /* Complex conditional that may expand to VEC_COND_EXPR with many operands */
    v4df cmp = (mask > (v4df){0.5, 0.5, 0.5, 0.5});
    v4df true_val = a * b + c;
    v4df false_val = d / (b + (v4df){1.0, 1.0, 1.0, 1.0});
    v4df result = cmp ? true_val : false_val;
    BARRIER();
    return result;
}

/* Chain of operations that may require many operands */
NOOPT v8si multi_operand_chain(v8si a, v8si b, v8si c, v8si d, v8si e) {
    volatile v8si temp1, temp2, temp3;
    
    /* Operation chain that may require many temporary registers */
    temp1 = a + b * c;
    BARRIER();
    
    temp2 = (temp1 > d) ? temp1 : d;
    BARRIER();
    
    /* Complex expression with many operands */
    temp3 = temp2 + a - b * c / (e + (v8si){1,1,1,1,1,1,1,1});
    BARRIER();
    
    /* Shuffle with elements from multiple vectors */
    v8si mask = {7,6,5,4,3,2,1,0};
    v8si shuffled = __builtin_shuffle(temp3, e, mask);
    BARRIER();
    
    return shuffled + temp2;
}

/* Test function with mixed operations */
NOOPT v4sf test_10_operands(v4sf a, v4sf b, v4sf c, v4sf d, 
                           v4si mask1, v4si mask2) {
    volatile v4sf temp[4];
    v4sf result;
    
    /* Operation 1: Blend-like operation using conditional */
    v4sf cmp = (a > b);
    temp[0] = cmp ? a * c : b * d;
    BARRIER();
    
    /* Operation 2: Shuffle with runtime mask */
    v4si shuffle_mask = mask1 + mask2;
    v4sf shuffled = __builtin_shuffle(temp[0], c, shuffle_mask);
    BARRIER();
    
    /* Operation 3: Complex arithmetic chain */
    temp[1] = shuffled + a - b;
    temp[2] = temp[1] * c / d;
    BARRIER();
    
    /* Operation 4: Another conditional */
    v4sf cmp2 = (temp[2] > (v4sf){0.0f, 0.0f, 0.0f, 0.0f});
    temp[3] = cmp2 ? temp[2] * a : temp[2] * b;
    BARRIER();
    
    /* Final blend of all results */
    result = temp[0] + temp[1] - temp[2] + temp[3];
    BARRIER();
    
    return result;
}

/* AVX-512 style operations if available */
#ifdef __AVX512F__
NOOPT v16sf avx512_complex(v16sf a, v16sf b, v16sf c, v16sf d) {
    /* Complex operation that may require many operands */
    v16sf mask = a > b;
    v16sf true_val = a * c + d;
    v16sf false_val = b / c - d;
    v16sf result = mask ? true_val : false_val;
    
    /* Additional operations to increase operand count */
    v16sf temp = __builtin_shuffle(result, a, 
        (v16si){15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0});
    
    BARRIER();
    return temp + result;
}
#endif

/* Main test function */
NOOPT int test_function(void) {
    int checksum = 0;
    
    /* Initialize vectors with pattern */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = {9, 10, 11, 12};
    v4si v4 = {13, 14, 15, 16};
    v4si mask = {3, 2, 1, 0};
    
    v4df d1 = {1.0, 2.0, 3.0, 4.0};
    v4df d2 = {5.0, 6.0, 7.0, 8.0};
    v4df d3 = {9.0, 10.0, 11.0, 12.0};
    v4df d4 = {13.0, 14.0, 15.0, 16.0};
    v4df d_mask = {0.1, 0.9, 0.3, 0.7};
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf f3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf f4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
#ifdef __AVX__
    v8si avx1 = {1,2,3,4,5,6,7,8};
    v8si avx2 = {9,10,11,12,13,14,15,16};
    v8si avx3 = {17,18,19,20,21,22,23,24};
    v8si avx4 = {25,26,27,28,29,30,31,32};
    v8si avx5 = {33,34,35,36,37,38,39,40};
#endif
    
    /* Test 1: Complex shuffle (may trigger 10+ operand expansion) */
    v4si shuffled = shuffle_complex(v1, v2, mask);
    BARRIER();
    
    /* Test 2: Conditional operations with many operands */
    v4df cond_result = conditional_ops(d1, d2, d3, d4, d_mask);
    BARRIER();
    
    /* Test 3: Mixed operations that may require many operands */
    v4sf mixed_result = test_10_operands(f1, f2, f3, f4, mask, v3);
    BARRIER();
    
#ifdef __AVX__
    /* Test 4: AVX multi-operand chain */
    v8si chain_result = multi_operand_chain(avx1, avx2, avx3, avx4, avx5);
    BARRIER();
#endif
    
#ifdef __AVX512F__
    /* Test 5: AVX-512 complex operations */
    v16sf avx512_a = {0};
    v16sf avx512_b = {0};
    v16sf avx512_c = {0};
    v16sf avx512_d = {0};
    for (int i = 0; i < 16; i++) {
        avx512_a[i] = i * 1.0f;
        avx512_b[i] = i * 2.0f;
        avx512_c[i] = i * 3.0f;
        avx512_d[i] = i * 4.0f;
    }
    v16sf avx512_result = avx512_complex(avx512_a, avx512_b, avx512_c, avx512_d);
    BARRIER();
#endif
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 4; i++) {
        checksum += shuffled[i];
        checksum += (int)cond_result[i];
        checksum += (int)mixed_result[i];
    }
    
#ifdef __AVX__
    for (int i = 0; i < 8; i++) {
        checksum += chain_result[i];
    }
#endif
    
    return checksum;
}

int main(void) {
    int result = test_function();
    
    /* Use result to prevent dead code elimination */
    if (result != 0) {
        printf("Result: %d\n", result);
        return 0;
    }
    return 1;
}
