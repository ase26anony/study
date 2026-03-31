/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/* Define vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Larger vector types for AVX */
#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Volatile store to force operations */
#define VOLATILE_STORE(var, val) do { \
    volatile __typeof__(var) _temp = (val); \
    var = _temp; \
    COMPILER_BARRIER(); \
} while(0)

/* Test function with many vector operations - marked noinline to prevent optimization */
__attribute__((noinline, target("sse2,avx")))
v4si test_vector_operations(v4si a, v4si b, v4si c, v4si d, 
                           v4sf fa, v4sf fb, v4sf fc, v4sf fd) {
    volatile v4si v1, v2, v3, v4;
    volatile v4sf fv1, fv2, fv3, fv4;
    
    /* Operation 1: Complex shuffle with many operands */
    /* __builtin_shuffle can expand to many operands depending on pattern */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    VOLATILE_STORE(v1, shuffled);
    
    /* Operation 2: Vector conditional expression */
    /* This generates VEC_COND_EXPR which may need many operands */
    v4si cmp_result = a > b;
    v4si cond_result = cmp_result ? (a * c + d) : (b * d - c);
    VOLATILE_STORE(v2, cond_result);
    
    /* Operation 3: Chain of arithmetic operations */
    /* Each operation may need temporary registers */
    v4si chain1 = a + b;
    v4si chain2 = chain1 * c;
    v4si chain3 = chain2 - d;
    v4si chain4 = chain3 / (a + 1);
    VOLATILE_STORE(v3, chain4);
    
    /* Operation 4: Mixed float/int operations with conversions */
    v4sf float_cond = fa > fb;
    v4sf float_result = float_cond ? (fa * fc + fd) : (fb * fd - fc);
    
    /* Convert float to int - may use conversion instruction with many operands */
    v4si int_result = __builtin_convertvector(float_result, v4si);
    VOLATILE_STORE(v4, int_result);
    
    COMPILER_BARRIER();
    
    /* Combine all results */
    v4si final_result = v1 + v2 + v3 + v4;
    
    /* Additional complex operation that might need 11 operands */
    /* Using __builtin_shufflevector with variable mask */
    v4si shuffle_vec1 = {1, 2, 3, 4};
    v4si shuffle_vec2 = {5, 6, 7, 8};
    int shuffle_indices[4] = {3, 1, 2, 0};
    
    /* Manually create shuffle to avoid constant propagation */
    v4si manual_shuffle;
    for (int i = 0; i < 4; i++) {
        int idx = shuffle_indices[i];
        if (idx < 4) {
            manual_shuffle[i] = shuffle_vec1[idx];
        } else {
            manual_shuffle[i] = shuffle_vec2[idx - 4];
        }
    }
    
    final_result = final_result + manual_shuffle;
    
    return final_result;
}

/* Second test function focusing on larger vectors for AVX */
#ifdef __AVX__
__attribute__((noinline, target("avx")))
v8si test_avx_operations(v8si a, v8si b, v8si c, v8si d) {
    volatile v8si v1, v2, v3;
    
    /* AVX operations with 256-bit vectors */
    /* These are more likely to need many operands */
    
    /* Complex blend operation - may use many operands */
    v8si blend_mask = {0, -1, 0, -1, 0, -1, 0, -1};
    v8si blended = __builtin_shuffle(a, b, blend_mask);
    VOLATILE_STORE(v1, blended);
    
    /* Vector conditional with 8 elements */
    v8si cmp = a > b;
    v8si cond = cmp ? (a * c) : (b * d);
    VOLATILE_STORE(v2, cond);
    
    /* Chain of operations that might need temporaries */
    v8si temp1 = a + b;
    v8si temp2 = temp1 * c;
    v8si temp3 = temp2 - d;
    v8si temp4 = temp3 / (a + 1);
    v8si temp5 = temp4 << 2;
    v8si temp6 = temp5 >> 1;
    VOLATILE_STORE(v3, temp6);
    
    COMPILER_BARRIER();
    
    return v1 + v2 + v3;
}
#endif

/* Test with double precision vectors */
__attribute__((noinline, target("sse2")))
v2df test_double_operations(v2df a, v2df b, v2df c, v2df d) {
    volatile v2df v1, v2;
    
    /* Double precision operations */
    v2df sum = a + b;
    v2df product = c * d;
    v2df diff = a - b;
    
    /* Complex expression that might need many registers */
    v2df result = (sum * product) / (diff + 1.0);
    VOLATILE_STORE(v1, result);
    
    /* Comparison and conditional */
    v2df cmp = a > b;
    v2df cond_result = cmp ? (a * c) : (b * d);
    VOLATILE_STORE(v2, cond_result);
    
    COMPILER_BARRIER();
    
    return v1 + v2;
}

int main() {
    /* Initialize test vectors with pattern values */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fd = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2df da = {1.0, 2.0};
    v2df db = {3.0, 4.0};
    v2df dc = {5.0, 6.0};
    v2df dd = {7.0, 8.0};
    
    /* Call test functions */
    v4si result1 = test_vector_operations(a, b, c, d, fa, fb, fc, fd);
    v2df result2 = test_double_operations(da, db, dc, dd);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    for (int i = 0; i < 4; i++) {
        checksum1 += result1[i];
    }
    
    double checksum2 = 0.0;
    for (int i = 0; i < 2; i++) {
        checksum2 += result2[i];
    }
    
#ifdef __AVX__
    /* Test AVX operations if available */
    v8si avx_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si avx_c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si avx_d = {25, 26, 27, 28, 29, 30, 31, 32};
    
    v8si avx_result = test_avx_operations(avx_a, avx_b, avx_c, avx_d);
    
    int avx_checksum = 0;
    for (int i = 0; i < 8; i++) {
        avx_checksum += avx_result[i];
    }
    
    printf("AVX Checksum: %d\n", avx_checksum);
#endif
    
    printf("SSE Checksum: %d\n", checksum1);
    printf("Double Checksum: %f\n", checksum2);
    
    /* Return based on checksums to ensure execution */
    return (checksum1 > 0) ? 0 : 1;
}
