/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

/* Force no optimization on this function */
__attribute__((noinline, noipa))
static v8si test_10_operands(v8si a, v8si b, v8si c, v8si d, 
                             v8si e, v8si f, v8si g, v8si h) {
    /* Complex chain of operations that may require many operands */
    volatile v8si temp1, temp2, temp3, temp4;
    
    /* Shuffle with runtime mask - may expand to many operands */
    v8si mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled = __builtin_shuffle(a, b, mask);
    temp1 = shuffled;
    
    /* Vector conditional with complex expressions */
    v8si cmp = a > b;
    v8si cond_result = cmp ? (c * d + e) : (f - g * h);
    temp2 = cond_result;
    
    /* Another shuffle with different vectors */
    v8si mask2 = {0, 2, 4, 6, 1, 3, 5, 7};
    v8si shuffled2 = __builtin_shuffle(c, d, mask2);
    
    /* Complex arithmetic chain */
    v8si arith = (a + b) * (c - d) + (e & f) | (g ^ h);
    temp3 = arith;
    
    /* Blend-like operation using conditional */
    v8si blend_mask = a > c;
    v8si blended = blend_mask ? shuffled : shuffled2;
    
    /* Final combination */
    v8si result = blended + cond_result + arith;
    temp4 = result;
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4) : "memory");
    
    return result;
}

__attribute__((noinline, noipa))
static v4df test_11_operands(v4df a, v4df b, v4df c, v4df d,
                             v4df e, v4df f, v4df g, v4df h) {
    volatile v4df temp1, temp2, temp3, temp4, temp5;
    
    /* Complex floating-point vector operations */
    v4df sum1 = a + b;
    v4df sum2 = c + d;
    v4df sum3 = e + f;
    v4df sum4 = g + h;
    
    temp1 = sum1;
    temp2 = sum2;
    temp3 = sum3;
    temp4 = sum4;
    
    /* Vector comparisons for conditional */
    v4df cmp1 = a > b;
    v4df cmp2 = c > d;
    v4df cmp3 = e > f;
    
    /* Nested conditional expressions */
    v4df cond1 = cmp1 ? sum1 * sum2 : sum3 / sum4;
    v4df cond2 = cmp2 ? sum2 - sum3 : sum4 + sum1;
    v4df cond3 = cmp3 ? cond1 * cond2 : cond1 / cond2;
    
    /* Shuffle with floating-point vectors */
    long long mask_fp[4] = {2, 3, 0, 1};
    v4df shuffled;
    memcpy(&shuffled, &mask_fp, sizeof(shuffled));
    
    /* Complex blend operation */
    v4df blend_cmp = a > c;
    v4df blended = blend_cmp ? cond3 : shuffled;
    
    /* Final computation with many operands */
    v4df result = blended + cond1 - cond2 * cond3 / (sum1 + sum2);
    temp5 = result;
    
    /* Compiler barrier */
    asm volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3), 
                       "r"(temp4), "r"(temp5) : "memory");
    
    return result;
}

/* AVX-512 test with even larger vectors */
#ifdef __AVX512F__
__attribute__((noinline, noipa))
static v16si test_avx512_operands(v16si a, v16si b, v16si c, v16si d) {
    volatile v16si temp1, temp2;
    
    /* Large shuffle operation */
    v16si mask = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    v16si shuffled = __builtin_shuffle(a, b, mask);
    temp1 = shuffled;
    
    /* Complex operation on 512-bit vectors */
    v16si result = (a & b) | (c ^ d) + shuffled * mask;
    temp2 = result;
    
    asm volatile("" : : "r"(temp1), "r"(temp2) : "memory");
    
    return result;
}
#endif

int main() {
    /* Initialize test vectors with pattern */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {16, 14, 12, 10, 8, 6, 4, 2};
    v8si e = {3, 6, 9, 12, 15, 18, 21, 24};
    v8si f = {24, 21, 18, 15, 12, 9, 6, 3};
    v8si g = {4, 8, 12, 16, 20, 24, 28, 32};
    v8si h = {32, 28, 24, 20, 16, 12, 8, 4};
    
    /* Test 10-operand path */
    v8si res1 = test_10_operands(a, b, c, d, e, f, g, h);
    
    /* Initialize floating-point vectors */
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {4.0, 3.0, 2.0, 1.0};
    v4df fc = {2.0, 4.0, 6.0, 8.0};
    v4df fd = {8.0, 6.0, 4.0, 2.0};
    v4df fe = {3.0, 6.0, 9.0, 12.0};
    v4df ff = {12.0, 9.0, 6.0, 3.0};
    v4df fg = {4.0, 8.0, 12.0, 16.0};
    v4df fh = {16.0, 12.0, 8.0, 4.0};
    
    /* Test 11-operand path */
    v4df res2 = test_11_operands(fa, fb, fc, fd, fe, ff, fg, fh);
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    v16si va = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si vb = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si vc = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};
    v16si vd = {32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2};
    
    v16si res3 = test_avx512_operands(va, vb, vc, vd);
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += res1[i];
    }
    
    double fchecksum = 0.0;
    for (int i = 0; i < 4; i++) {
        fchecksum += res2[i];
    }
    
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    return (checksum != 0) ? 0 : 1;
}
