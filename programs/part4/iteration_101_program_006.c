/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif
#ifdef __AVX__
#include <immintrin.h>
#endif

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Force no inlining to preserve operations */
__attribute__((noinline, target("avx2")))
v8si test_10_11_operands(v8si a, v8si b, v8si c, v8si d, 
                         v4df e, v4df f, v4df g, v4df h) {
    volatile v8si temp1, temp2, temp3;
    volatile v4df ftemp1, ftemp2;
    
    /* Complex shuffle with many operands - may expand to 10+ operands */
    /* Shuffle two v8si vectors with a mask */
    v8si mask = {0,8,1,9,2,10,3,11};
    v8si shuffled = __builtin_shuffle(a, b, mask);
    temp1 = shuffled;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Vector conditional expression with comparison */
    /* This generates VEC_COND_EXPR which may need many operands */
    v8si cmp_result = (a > b) ? (c * d) : (c + d);
    temp2 = cmp_result;
    
    /* Another barrier */
    asm volatile("" ::: "memory");
    
    /* Complex blend operation - emulate with conditional */
    v8si blend_mask = (a > c);
    v8si blended = blend_mask ? (a * b) : (c - d);
    
    /* Mix in floating point operations */
    /* Vector comparison on doubles */
    v4df dbl_cmp = (e > f) ? (g * h) : (g / h);
    ftemp1 = dbl_cmp;
    
    /* Another complex shuffle on the result */
    v4df shuffle_mask = {3.0, 2.0, 1.0, 0.0};
    /* Use __builtin_shufflevector for floating point */
    v4df shuffled_dbl = __builtin_shufflevector(g, h, 3, 2, 5, 6);
    ftemp2 = shuffled_dbl;
    
    /* Final combination - mix integer and float results */
    v8si int_result = blended + cmp_result;
    
    /* Use convert operation which may need many operands */
    v8sf float_convert = __builtin_convertvector(int_result, v8sf);
    
    /* Store to volatile to force all operations */
    volatile v8sf final_float = float_convert;
    
    /* Convert back to integer */
    v8si final_result = __builtin_convertvector(final_float, v8si);
    
    return final_result + shuffled;
}

/* Another test function focusing on exactly 11 operands */
__attribute__((noinline, target("avx512f")))
v8si test_11_operands_specific(v8si a, v8si b, v8si c, v8si d,
                               v8si e, v8si f, v8si g) {
    /* Create a complex expression that might need 11 operands */
    /* Multiple shuffles and blends combined */
    
    volatile v8si temp[4];
    
    /* Shuffle 1 */
    v8si mask1 = {1,0,3,2,5,4,7,6};
    v8si s1 = __builtin_shuffle(a, b, mask1);
    temp[0] = s1;
    
    /* Shuffle 2 */
    v8si mask2 = {7,6,5,4,3,2,1,0};
    v8si s2 = __builtin_shuffle(c, d, mask2);
    temp[1] = s2;
    
    /* Complex conditional blend */
    v8si cmp1 = (s1 > s2);
    v8si blend1 = cmp1 ? (s1 * e) : (s2 + f);
    temp[2] = blend1;
    
    /* Another level - condition on blend result */
    v8si cmp2 = (blend1 > a);
    v8si final_blend = cmp2 ? (blend1 * g) : (a - b);
    temp[3] = final_blend;
    
    /* Arithmetic chain that might expand to many operands */
    v8si result = final_blend + (s1 * s2) - (e / (f + 1));
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

int main() {
    /* Initialize vectors with pattern values */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {1, 3, 5, 7, 9, 11, 13, 15};
    
    v4df e = {1.0, 2.0, 3.0, 4.0};
    v4df f = {4.0, 3.0, 2.0, 1.0};
    v4df g = {1.5, 2.5, 3.5, 4.5};
    v4df h = {0.5, 1.5, 2.5, 3.5};
    
    printf("Testing 10/11 operand expansion paths...\n");
    
    /* Call test functions */
    v8si result1 = test_10_11_operands(a, b, c, d, e, f, g, h);
    
    /* Additional vectors for 11 operand test */
    v8si e2 = {10, 20, 30, 40, 50, 60, 70, 80};
    v8si f2 = {5, 10, 15, 20, 25, 30, 35, 40};
    v8si g2 = {2, 4, 8, 16, 32, 64, 128, 256};
    
    v8si result2 = test_11_operands_specific(a, b, c, d, e2, f2, g2);
    
    /* Compute checksums to prevent optimization */
    int checksum1 = 0, checksum2 = 0;
    for (int i = 0; i < 8; i++) {
        checksum1 += result1[i];
        checksum2 += result2[i];
    }
    
    printf("Checksum1: %d, Checksum2: %d\n", checksum1, checksum2);
    
    /* Return based on checksum to ensure execution */
    if (checksum1 != 0 || checksum2 != 0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("Unexpected zero checksum.\n");
        return 1;
    }
}
