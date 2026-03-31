/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));
typedef float v8sf __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

/* Force no optimization on specific operations */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var) : "memory")

/* Test function with many vector operations - marked noinline to prevent optimization */
__attribute__((noinline, target("avx2")))
v8si test_10_operand_expansion(v8si a, v8si b, v8si c, v8si d, v8si mask) {
    volatile v8si temp1, temp2, temp3;
    
    /* Complex shuffle operation that may require many operands */
    v8si shuffled = __builtin_shufflevector(a, b, 7, 6, 5, 4, 3, 2, 1, 0);
    KEEP_ALIVE(shuffled);
    
    /* Vector conditional expression with arithmetic */
    v8si cmp_result = (mask > 0) ? (a * b + c) : (d - a);
    temp1 = cmp_result;
    
    /* Another shuffle with different pattern */
    v8si shuffled2 = __builtin_shufflevector(c, d, 0, 1, 2, 3, 4, 5, 6, 7);
    
    /* Blend operation simulated using conditional */
    v8si blend_mask = (mask & 1) ? -1 : 0;
    v8si blended = blend_mask ? shuffled : shuffled2;
    temp2 = blended;
    
    /* Complex arithmetic chain */
    v8si result = a + b * c - d / (mask + 1) + blended * 2;
    temp3 = result;
    
    /* Final shuffle to potentially trigger 10+ operand expansion */
    v8si final = __builtin_shufflevector(
        result, blended, 
        0, 2, 4, 6, 1, 3, 5, 7
    );
    
    return final;
}

/* Test with floating point vectors */
__attribute__((noinline, target("avx")))
v4df test_fp_10_operand_expansion(v4df a, v4df b, v4df c, v4df d, v4df mask) {
    volatile v4df temp1, temp2;
    
    /* Vector comparison and conditional */
    v4df cmp = (mask > 0.5) ? a : b;
    temp1 = cmp;
    
    /* Complex FP expression */
    v4df result = a * b + c / d - cmp;
    
    /* Shuffle FP values */
    v4df shuffled = __builtin_shufflevector(result, cmp, 3, 2, 1, 0);
    
    /* Conditional blend */
    v4df final = (shuffled > result) ? shuffled * 2.0 : result / 2.0;
    temp2 = final;
    
    return final;
}

/* Test with AVX-512 sized vectors for more operands */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
v16si test_avx512_11_operand(v16si a, v16si b, v16si c, v16si d, v16si e, v16si mask) {
    volatile v16si temp1, temp2, temp3, temp4;
    
    /* Multiple shuffles to increase operand count */
    v16si s1 = __builtin_shufflevector(a, b, 
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    temp1 = s1;
    
    v16si s2 = __builtin_shufflevector(c, d,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    temp2 = s2;
    
    /* Complex conditional with many operands */
    v16si cond = (mask > 0) ? (a * b + c) : (d - e);
    
    /* Blend with mask */
    v16si blend_mask = (mask & 1) ? -1 : 0;
    v16si blended = blend_mask ? s1 : s2;
    temp3 = blended;
    
    /* Arithmetic with many terms */
    v16si result = a + b * c - d / (e + 1) + blended * 2 + cond;
    temp4 = result;
    
    /* Final complex shuffle that might need 11 operands */
    v16si final = __builtin_shufflevector(
        result, blended, cond,
        0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23
    );
    
    return final;
}
#endif

/* Main test driver */
int main() {
    /* Initialize test vectors with pattern */
    v8si a8 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b8 = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c8 = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d8 = {1, 3, 5, 7, 9, 11, 13, 15};
    v8si mask8 = {0, -1, 0, -1, 0, -1, 0, -1};
    
    v4df a4 = {1.0, 2.0, 3.0, 4.0};
    v4df b4 = {4.0, 3.0, 2.0, 1.0};
    v4df c4 = {0.5, 1.5, 2.5, 3.5};
    v4df d4 = {2.0, 2.0, 2.0, 2.0};
    v4df mask4 = {0.0, 1.0, 0.0, 1.0};
    
    printf("Testing 10/11 operand expansion patterns...\n");
    
    /* Call test functions multiple times with different inputs */
    v8si result1 = test_10_operand_expansion(a8, b8, c8, d8, mask8);
    v4df result2 = test_fp_10_operand_expansion(a4, b4, c4, d4, mask4);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    for (int i = 0; i < 8; i++) {
        checksum1 += result1[i];
    }
    
    double checksum2 = 0.0;
    for (int i = 0; i < 4; i++) {
        checksum2 += result2[i];
    }
    
    printf("Integer checksum: %d\n", checksum1);
    printf("FP checksum: %f\n", checksum2);
    
    #ifdef __AVX512F__
    v16si a16 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si b16 = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si c16 = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};
    v16si d16 = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31};
    v16si e16 = {0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30};
    v16si mask16 = {0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1};
    
    v16si result3 = test_avx512_11_operand(a16, b16, c16, d16, e16, mask16);
    
    int checksum3 = 0;
    for (int i = 0; i < 16; i++) {
        checksum3 += result3[i];
    }
    printf("AVX-512 checksum: %d\n", checksum3);
    #endif
    
    /* Return based on checksums to ensure execution */
    return (checksum1 != 0 && checksum2 != 0.0) ? 0 : 1;
}
