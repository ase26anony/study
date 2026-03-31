/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/* Define vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

typedef int v8si __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* AVX-512 types for more operands */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Force no inlining to prevent optimization */
__attribute__((noinline,noipa))
v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    volatile v4si temp1, temp2, temp3;
    
    /* Complex shuffle with many operands - may expand to 10+ operands */
    v4si shuffled = __builtin_shuffle(a, b, mask);
    temp1 = shuffled;
    
    /* Vector conditional with arithmetic - generates VEC_COND_EXPR */
    v4si cmp_result = (a > b) ? (c * d) : (c + d);
    temp2 = cmp_result;
    
    /* Blend operation using conditional */
    v4si blend_mask = (mask != (v4si){0,0,0,0});
    v4si blended = blend_mask ? shuffled : cmp_result;
    temp3 = blended;
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    /* Chain operations to increase operand count */
    v4si result = blended + (shuffled * cmp_result) - (a & b) | (c ^ d);
    
    return result;
}

__attribute__((noinline,noipa))
v4df test_11_operands(v4df a, v4df b, v4df c, v4df d, v4df e, v4df f) {
    volatile v4df temp1, temp2, temp3, temp4;
    
    /* Complex FP vector operations that may require many operands */
    v4df sum1 = a + b;
    v4df sum2 = c + d;
    v4df sum3 = e + f;
    temp1 = sum1;
    temp2 = sum2;
    temp3 = sum3;
    
    /* Vector conditional with FP comparisons */
    v4df cmp_mask = (a > b);
    v4df sel1 = cmp_mask ? sum1 : sum2;
    v4df sel2 = (!cmp_mask) ? sum3 : a;
    
    /* Complex expression with many operands */
    v4df result = sel1 * sel2 + (sum1 / sum2) - (sum3 * a) + (b / c) * (d - e);
    temp4 = result;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Additional shuffle to potentially trigger 11-operand pattern */
    v4df shuffled = __builtin_shufflevector(sum1, sum2, 0, 2, 1, 3);
    result = result + shuffled;
    
    return result;
}

/* Test with AVX vectors */
#ifdef __AVX2__
__attribute__((noinline,noipa))
v8si test_avx_10_operands(v8si a, v8si b, v8si c, v8si d, v8si mask) {
    volatile v8si temp[4];
    
    /* Multiple operations chained together */
    v8si op1 = a + b;
    v8si op2 = c - d;
    v8si op3 = a * b;
    v8si op4 = c | d;
    
    temp[0] = op1;
    temp[1] = op2;
    temp[2] = op3;
    temp[3] = op4;
    
    /* Complex shuffle with mask */
    v8si shuffled = __builtin_shufflevector(op1, op2, 
        0, 2, 4, 6, 1, 3, 5, 7);
    
    /* Vector conditional with comparison */
    v8si cmp = (mask > (v8si){0,0,0,0,0,0,0,0});
    v8si result = cmp ? shuffled : (op3 & op4);
    
    /* More operations to increase operand count */
    result = result + (op1 * op2) - (op3 / (op4 + (v8si){1,1,1,1,1,1,1,1}));
    
    asm volatile("" ::: "memory");
    
    return result;
}
#endif

/* Main test driver */
int main() {
    /* Initialize test vectors */
    v4si a4 = {1, 2, 3, 4};
    v4si b4 = {5, 6, 7, 8};
    v4si c4 = {9, 10, 11, 12};
    v4si d4 = {13, 14, 15, 16};
    v4si mask4 = {0, 2, 1, 3};
    
    v4df ad = {1.0, 2.0, 3.0, 4.0};
    v4df bd = {5.0, 6.0, 7.0, 8.0};
    v4df cd = {9.0, 10.0, 11.0, 12.0};
    v4df dd = {13.0, 14.0, 15.0, 16.0};
    v4df ed = {17.0, 18.0, 19.0, 20.0};
    v4df fd = {21.0, 22.0, 23.0, 24.0};
    
    /* Call test functions */
    v4si res1 = test_10_operands(a4, b4, c4, d4, mask4);
    v4df res2 = test_11_operands(ad, bd, cd, dd, ed, fd);
    
#ifdef __AVX2__
    v8si a8 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b8 = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si c8 = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si d8 = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si mask8 = {0, 2, 1, 3, 4, 6, 5, 7};
    
    v8si res3 = test_avx_10_operands(a8, b8, c8, d8, mask8);
#endif
    
    /* Compute checksums to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += res1[i];
        checksum += (int)res2[i];
    }
    
#ifdef __AVX2__
    for (int i = 0; i < 8; i++) {
        checksum += res3[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return checksum != 0 ? 0 : 1;
}
