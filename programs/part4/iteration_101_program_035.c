/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* SSE/AVX types for builtins */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Force no optimization on specific operations */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    __asm__ volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() __asm__ volatile("" : : : "memory")

/* Test function with many vector operations */
__attribute__((noinline, target("avx2")))
v8si test_10_11_operands(v8si a, v8si b, v8si c, v8si d, 
                         v4df f1, v4df f2, v4df f3, v4df f4) {
    volatile v8si v1, v2, v3, v4;
    volatile v4df fv1, fv2, fv3;
    v8si result = {0};
    
    /* Operation 1: Complex shuffle that may need many operands */
    /* __builtin_shuffle with runtime mask */
    v8si mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v1 = __builtin_shuffle(a, b, mask);
    BARRIER();
    
    /* Operation 2: Vector conditional with comparison */
    /* This generates VEC_COND_EXPR which expands to many operands */
    v8si cmp = a > b;
    v2 = cmp ? (c * d + v1) : (c - d * v1);
    BARRIER();
    
    /* Operation 3: Chain of operations forcing many temps */
    v3 = (a + b) * (c - d) / (v1 + 1) + (v2 * 2);
    KEEP(v3);
    
    /* Operation 4: Use AVX2 blend with variable mask */
    /* __builtin_ia32_pblendd256 needs 3 operands but may expand further */
    v4 = __builtin_ia32_pblendd256(a, b, 0xAA);
    BARRIER();
    
    /* Operation 5: Convert between vector types - may need many operands */
    v8sf floats = __builtin_convertvector(v3, v8sf);
    KEEP(floats);
    
    /* Operation 6: Complex floating point vector conditional */
    /* Double precision comparison and blend */
    v4df cmp_df = f1 > f2;
    fv1 = cmp_df ? (f1 * f2 + f3) : (f1 / f2 - f3);
    BARRIER();
    
    /* Operation 7: Shuffle doubles - needs mask, inputs, output */
    v4di mask_df = {3, 2, 1, 0};
    fv2 = __builtin_shuffle(f1, f2, mask_df);
    KEEP(fv2);
    
    /* Operation 8: Mix everything together */
    v8si ints_from_floats = __builtin_convertvector(floats, v8si);
    result = v4 + ints_from_floats + v3;
    
    /* Operation 9: Another complex conditional with mixed types */
    v8si final_cmp = result > a;
    result = final_cmp ? (result * b) : (result + c * d);
    
    /* Operation 10: Use permutevar which needs many operands */
    /* __builtin_ia32_permvarsi256 needs mask and input */
    result = __builtin_ia32_permvarsi256(result, mask);
    
    return result;
}

/* Another test focusing on exactly 10/11 operand patterns */
__attribute__((noinline, target("avx512f")))
v8si test_many_operands() {
    /* AVX-512 types for more operands */
    typedef int v16si __attribute__((vector_size(64)));
    
    /* Initialize many vector variables */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 3, 4, 5, 6, 7, 8, 9};
    v8si d = {9, 8, 7, 6, 5, 4, 3, 2};
    v8si e = {3, 4, 5, 6, 7, 8, 9, 10};
    v8si f = {10, 9, 8, 7, 6, 5, 4, 3};
    v8si g = {4, 5, 6, 7, 8, 9, 10, 11};
    v8si h = {11, 10, 9, 8, 7, 6, 5, 4};
    
    volatile v8si temp1, temp2, temp3, temp4;
    
    /* Complex expression that may need many temporary registers */
    /* a * b + c * d - e * f + g * h / 2 */
    temp1 = a * b;
    BARRIER();
    temp2 = c * d;
    BARRIER();
    temp3 = e * f;
    BARRIER();
    temp4 = g * h / 2;
    BARRIER();
    
    v8si result = temp1 + temp2 - temp3 + temp4;
    
    /* Vector comparison chain */
    v8si cmp1 = a > b;
    v8si cmp2 = c < d;
    v8si cmp3 = e == f;
    
    /* Nested conditionals - may expand to many operands */
    result = cmp1 ? (cmp2 ? result * 2 : result / 2) : 
                   (cmp3 ? result + 1 : result - 1);
    
    /* Shuffle with computed mask */
    v8si shuffle_mask = result & 7;  /* Mask values 0-7 */
    result = __builtin_shuffle(result, shuffle_mask);
    
    return result;
}

int main() {
    /* Initialize test vectors */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 3, 4, 5, 6, 7, 8, 9};
    v8si d = {9, 8, 7, 6, 5, 4, 3, 2};
    
    v4df f1 = {1.0, 2.0, 3.0, 4.0};
    v4df f2 = {4.0, 3.0, 2.0, 1.0};
    v4df f3 = {1.5, 2.5, 3.5, 4.5};
    v4df f4 = {4.5, 3.5, 2.5, 1.5};
    
    printf("Testing 10/11 operand expansion patterns...\n");
    
    /* Call test functions */
    v8si result1 = test_10_11_operands(a, b, c, d, f1, f2, f3, f4);
    v8si result2 = test_many_operands();
    
    /* Compute checksums to prevent optimization */
    int checksum1 = 0, checksum2 = 0;
    for (int i = 0; i < 8; i++) {
        checksum1 += result1[i];
        checksum2 += result2[i];
    }
    
    printf("Checksum1: %d, Checksum2: %d\n", checksum1, checksum2);
    
    /* Return based on checksum to ensure execution */
    return (checksum1 + checksum2) != 0 ? 0 : 1;
}
