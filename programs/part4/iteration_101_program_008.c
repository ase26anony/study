/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* GCC vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

/* Force no inlining to preserve expansion patterns */
__attribute__((noinline, noipa))
v8si test_10_operand_expansion(v8si a, v8si b, v8si c, v8si d, 
                               v8si e, v8si f, v8si mask) {
    volatile v8si temp1, temp2, temp3;
    
    /* Complex shuffle with many operands - may expand to 10+ operands */
    v8si shuffled = __builtin_shuffle(a, b, mask);
    temp1 = shuffled;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Vector conditional with arithmetic - generates VEC_COND_EXPR */
    v8si cmp_result = (a > b) ? (c * d + shuffled) : (e - f);
    temp2 = cmp_result;
    
    /* Another shuffle with different inputs */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled2 = __builtin_shuffle(cmp_result, temp1, shuffle_mask);
    
    /* Blend operation using conditional */
    v8si blend_mask = a < b;
    v8si blended = blend_mask ? shuffled2 : cmp_result;
    
    /* Final arithmetic chain that may require many temporaries */
    v8si result = blended + (a * b) - (c / (d + 1)) + (e & f);
    
    /* Store to volatile to prevent elimination */
    temp3 = result;
    
    return temp3;
}

__attribute__((noinline, noipa))
v4df test_11_operand_expansion(v4df a, v4df b, v4df c, v4df d,
                               v4df e, v4df f, v4df mask) {
    volatile v4df temp1, temp2, temp3;
    
    /* Complex floating-point conditional with many operands */
    v4df cmp = (mask > 0.0) ? a : b;
    temp1 = cmp;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Arithmetic chain that may expand to many operands */
    v4df t1 = a * b + c;
    v4df t2 = d / e - f;
    v4df t3 = __builtin_shuffle(t1, t2, (v4di){3, 2, 1, 0});
    
    /* Nested conditional expression */
    v4df result = (cmp > t3) ? 
                  (t1 * t2 + a - b) : 
                  (t3 / t1 + c * d);
    
    /* Mix with integer conversion for more complexity */
    v4si int_vec = __builtin_convertvector(result, v4si);
    v4df reconverted = __builtin_convertvector(int_vec, v4df);
    
    result = result + reconverted * 0.5;
    
    /* Store to volatile */
    temp2 = result;
    
    /* Final blend-like operation */
    v4df final_mask = {1.0, -1.0, 1.0, -1.0};
    v4df final_result = (final_mask > 0) ? result : temp1;
    
    temp3 = final_result;
    return temp3;
}

/* AVX-512 style test for maximum operands */
#ifdef __AVX512F__
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

__attribute__((noinline, noipa))
v16sf test_avx512_many_operands(v16sf a, v16sf b, v16sf c, v16sf d,
                                v16sf e, v16sf f, v16si mask) {
    volatile v16sf temp;
    
    /* Very complex expression that may require many temporary registers */
    v16sf t1 = __builtin_shuffle(a, b, mask);
    v16sf t2 = c * d + e / f;
    v16sf t3 = __builtin_shuffle(t1, t2, mask);
    
    v16sf result = (t1 > t2) ? (t3 * a + b - c) : (d + e * f);
    
    /* Multiple conversions */
    v16si int_result = __builtin_convertvector(result, v16si);
    v16sf float_result = __builtin_convertvector(int_result, v16sf);
    
    result = result + float_result;
    temp = result;
    
    return temp;
}
#endif

int main() {
    /* Initialize test vectors with pattern values */
    v8si a8 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b8 = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c8 = {2, 3, 4, 5, 6, 7, 8, 9};
    v8si d8 = {9, 8, 7, 6, 5, 4, 3, 2};
    v8si e8 = {3, 4, 5, 6, 7, 8, 9, 10};
    v8si f8 = {10, 9, 8, 7, 6, 5, 4, 3};
    v8si mask8 = {0, 1, 2, 3, 4, 5, 6, 7};
    
    v4df a4 = {1.0, 2.0, 3.0, 4.0};
    v4df b4 = {4.0, 3.0, 2.0, 1.0};
    v4df c4 = {1.5, 2.5, 3.5, 4.5};
    v4df d4 = {0.5, 1.5, 2.5, 3.5};
    v4df e4 = {2.0, 3.0, 4.0, 5.0};
    v4df f4 = {5.0, 4.0, 3.0, 2.0};
    v4df mask4 = {1.0, -1.0, 1.0, -1.0};
    
    /* Call test functions */
    v8si result8 = test_10_operand_expansion(a8, b8, c8, d8, e8, f8, mask8);
    v4df result4 = test_11_operand_expansion(a4, b4, c4, d4, e4, f4, mask4);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum_int = 0;
    double checksum_double = 0.0;
    
    for (int i = 0; i < 8; i++) {
        checksum_int += result8[i];
    }
    
    for (int i = 0; i < 4; i++) {
        checksum_double += result4[i];
    }
    
    /* Use results to affect return value */
    if (checksum_int > 0 && checksum_double != 0.0) {
        printf("Test executed successfully. Checksums: int=%d, double=%f\n", 
               checksum_int, checksum_double);
        return 0;
    } else {
        return 1;
    }
}
