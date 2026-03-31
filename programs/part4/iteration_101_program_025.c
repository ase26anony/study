/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* GCC vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* AVX-512 types if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Force no inlining to prevent optimization */
__attribute__((noinline, noipa))
v4si test_10_operand_expansion(v4si a, v4si b, v4si c, v4si d, 
                               v4si e, v4si f, v4si g, v4si h) {
    /* Complex shuffle with many operands */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si temp1, temp2, temp3, temp4;
    
    /* Prevent constant propagation */
    asm volatile("" : "+x"(a), "+x"(b), "+x"(c), "+x"(d),
                      "+x"(e), "+x"(f), "+x"(g), "+x"(h) : : "memory");
    
    /* Operation 1: Complex shuffle chain that may require many operands */
    temp1 = __builtin_shuffle(a, b, shuffle_mask);
    temp2 = __builtin_shuffle(c, d, shuffle_mask);
    temp3 = __builtin_shuffle(e, f, shuffle_mask);
    temp4 = __builtin_shuffle(g, h, shuffle_mask);
    
    /* Force memory operations with volatile */
    volatile v4si vtemp1, vtemp2, vtemp3, vtemp4;
    vtemp1 = temp1;
    vtemp2 = temp2;
    vtemp3 = temp3;
    vtemp4 = temp4;
    
    /* Complex arithmetic with many intermediate values */
    v4si result = (temp1 * temp2) + (temp3 - temp4);
    result = result & (temp1 | temp2);
    result = result ^ (temp3 & temp4);
    
    /* Another barrier */
    asm volatile("" ::: "memory");
    
    /* Vector conditional with comparison - may generate VEC_COND_EXPR */
    v4si cmp_mask = (temp1 > temp2);
    v4si cond_result = cmp_mask ? (temp3 * temp4) : (temp1 + temp2);
    
    /* Final combination */
    result = result + cond_result;
    
    return result;
}

__attribute__((noinline, noipa))
v4df test_11_operand_expansion(v4df a, v4df b, v4df c, v4df d,
                               v4df e, v4df f, v4df g, v4df h) {
    /* Complex floating-point vector operations */
    v4df temp1, temp2, temp3, temp4, temp5;
    
    /* Compiler barrier */
    asm volatile("" : "+x"(a), "+x"(b), "+x"(c), "+x"(d),
                      "+x"(e), "+x"(f), "+x"(g), "+x"(h) : : "memory");
    
    /* Chain of operations that may require many operands */
    temp1 = a * b + c;
    temp2 = d - e * f;
    temp3 = __builtin_convertvector((v4si){1,2,3,4}, v4df);
    temp4 = g / h;
    temp5 = temp1 + temp2;
    
    /* Volatile stores to force operations */
    volatile v4df vtemp1, vtemp2, vtemp3, vtemp4, vtemp5;
    vtemp1 = temp1;
    vtemp2 = temp2;
    vtemp3 = temp3;
    vtemp4 = temp4;
    vtemp5 = temp5;
    
    /* Complex conditional expression */
    v4df cmp_mask = (temp1 > temp2);
    v4df cond_result = cmp_mask ? (temp3 * temp4) : (temp5 / temp1);
    
    /* Blend-like operation using conditional */
    v4df blend_result = (temp1 > 0.0) ? cond_result : temp2;
    
    /* More arithmetic */
    v4df result = blend_result + temp3 - temp4 * temp5;
    
    /* Another barrier */
    asm volatile("" ::: "memory");
    
    /* Final operation with many inputs */
    result = result + a + b + c + d + e + f + g + h;
    
    return result;
}

/* Test with AVX-512 if available */
#ifdef __AVX512F__
__attribute__((noinline, noipa))
v8df test_avx512_many_operands(v8df a, v8df b, v8df c, v8df d,
                               v8df e, v8df f, v8df g, v8df h) {
    /* AVX-512 can handle more operands */
    v8df temp1, temp2, temp3, temp4;
    
    /* Complex mask operation */
    __mmask8 mask = 0xAA; /* 10101010 */
    
    /* Operations that may use many operands */
    temp1 = a + b;
    temp2 = c * d;
    temp3 = e - f;
    temp4 = g / h;
    
    /* Conditional with mask */
    v8df cond_result = (temp1 > temp2) ? temp3 : temp4;
    
    /* Blend with mask */
    v8df blend_result;
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            ((double*)&blend_result)[i] = ((double*)&temp1)[i];
        } else {
            ((double*)&blend_result)[i] = ((double*)&temp2)[i];
        }
    }
    
    /* Final result */
    v8df result = blend_result + cond_result;
    
    return result;
}
#endif

/* Main test driver */
int main() {
    /* Initialize test vectors */
    v4si int_vec1 = {1, 2, 3, 4};
    v4si int_vec2 = {5, 6, 7, 8};
    v4si int_vec3 = {9, 10, 11, 12};
    v4si int_vec4 = {13, 14, 15, 16};
    v4si int_vec5 = {17, 18, 19, 20};
    v4si int_vec6 = {21, 22, 23, 24};
    v4si int_vec7 = {25, 26, 27, 28};
    v4si int_vec8 = {29, 30, 31, 32};
    
    v4df double_vec1 = {1.0, 2.0, 3.0, 4.0};
    v4df double_vec2 = {5.0, 6.0, 7.0, 8.0};
    v4df double_vec3 = {9.0, 10.0, 11.0, 12.0};
    v4df double_vec4 = {13.0, 14.0, 15.0, 16.0};
    v4df double_vec5 = {17.0, 18.0, 19.0, 20.0};
    v4df double_vec6 = {21.0, 22.0, 23.0, 24.0};
    v4df double_vec7 = {25.0, 26.0, 27.0, 28.0};
    v4df double_vec8 = {29.0, 30.0, 31.0, 32.0};
    
    /* Call test functions */
    v4si int_result = test_10_operand_expansion(int_vec1, int_vec2, int_vec3, int_vec4,
                                                int_vec5, int_vec6, int_vec7, int_vec8);
    
    v4df double_result = test_11_operand_expansion(double_vec1, double_vec2, double_vec3, double_vec4,
                                                   double_vec5, double_vec6, double_vec7, double_vec8);
    
    /* Compute checksums to prevent dead code elimination */
    int int_checksum = 0;
    double double_checksum = 0.0;
    
    for (int i = 0; i < 4; i++) {
        int_checksum += int_result[i];
        double_checksum += double_result[i];
    }
    
    /* Print results to ensure execution */
    printf("Integer checksum: %d\n", int_checksum);
    printf("Double checksum: %f\n", double_checksum);
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    v8df avx512_vec1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df avx512_vec2 = {9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
    v8df avx512_vec3 = {17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0};
    v8df avx512_vec4 = {25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0};
    v8df avx512_vec5 = {33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0};
    v8df avx512_vec6 = {41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0};
    v8df avx512_vec7 = {49.0, 50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0};
    v8df avx512_vec8 = {57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0, 64.0};
    
    v8df avx512_result = test_avx512_many_operands(avx512_vec1, avx512_vec2, avx512_vec3, avx512_vec4,
                                                   avx512_vec5, avx512_vec6, avx512_vec7, avx512_vec8);
    
    double avx512_checksum = 0.0;
    for (int i = 0; i < 8; i++) {
        avx512_checksum += avx512_result[i];
    }
    printf("AVX-512 checksum: %f\n", avx512_checksum);
#endif
    
    /* Return based on checksums to ensure execution */
    if (int_checksum != 0 || double_checksum != 0.0) {
        return 0; /* Success */
    } else {
        return 1; /* Failure - all zeros unlikely */
    }
}
