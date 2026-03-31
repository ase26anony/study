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
                               v4si mask_vec, v4si shuffle_mask) {
    volatile v4si v1, v2, v3, v4, v5;
    
    /* Complex shuffle operation - may require many operands */
    v1 = __builtin_shuffle(a, b, shuffle_mask);
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Vector conditional expression with comparison */
    v4si cmp_result = (a > b) ? (c * d) : (c + d);
    
    /* Store to volatile to force operation */
    v2 = cmp_result;
    
    /* Another shuffle with different inputs */
    v3 = __builtin_shuffle(v1, cmp_result, mask_vec);
    
    /* Complex arithmetic chain */
    v4si temp1 = a * b + c;
    v4si temp2 = d - a * 2;
    v4si temp3 = temp1 & temp2;
    
    /* Vector blend using conditional */
    v4si blend_mask = (mask_vec != 0);
    v4si blended = blend_mask ? temp3 : v3;
    
    v4 = blended;
    
    /* Final shuffle with all inputs */
    v5 = __builtin_shuffle(blended, v1, shuffle_mask + mask_vec);
    
    return v5;
}

__attribute__((noinline, noipa))
v4df test_11_operand_expansion(v4df a, v4df b, v4df c, v4df d,
                               v4df e, v4si int_mask) {
    volatile v4df v1, v2, v3;
    
    /* Convert int mask to double mask */
    v4df mask = __builtin_convertvector(int_mask, v4df);
    
    /* Complex conditional with FP operations */
    v4df cmp = (a > b);
    v4df true_val = c * d + e;
    v4df false_val = c / d - e;
    
    /* Vector conditional - may expand to many operands */
    v4df cond_result = cmp ? true_val : false_val;
    
    v1 = cond_result;
    
    /* Shuffle with FP values (needs conversion) */
    v4si shuffle_idx = {0, 2, 1, 3};
    v4df shuffled = __builtin_shuffle(cond_result, shuffle_idx);
    
    /* Blend operation */
    v4df blended = __builtin_shufflevector(cond_result, shuffled, 0, 5, 2, 7);
    
    v2 = blended;
    
    /* Another conditional with mask */
    v4df mask_result = (mask > 0.0) ? blended * a : blended / a;
    
    /* Complex arithmetic chain */
    v4df final = mask_result + b * c - d / e;
    
    v3 = final;
    
    return final;
}

/* Test with AVX2 256-bit vectors */
__attribute__((noinline, noipa))
v8si test_avx2_many_operands(v8si a, v8si b, v8si c, v8si d,
                             v8si mask1, v8si mask2) {
    volatile v8si v1, v2, v3;
    
    /* Multiple shuffles on 256-bit vectors */
    v8si shuffled1 = __builtin_shuffle(a, b, mask1);
    v8si shuffled2 = __builtin_shuffle(c, d, mask2);
    
    /* Vector comparison and conditional */
    v8si cmp = (shuffled1 > shuffled2);
    v8si cond_result = cmp ? (a * b) : (c + d);
    
    v1 = cond_result;
    
    /* Blend with mask */
    v8si blend_mask = (mask1 != mask2);
    v8si blended = blend_mask ? cond_result : shuffled1;
    
    /* Another shuffle chain */
    v8si final_shuffle = __builtin_shuffle(blended, shuffled2, mask1 + 1);
    
    v2 = final_shuffle;
    
    /* Complex arithmetic */
    v8si temp1 = a * b + c;
    v8si temp2 = d - a;
    v8si temp3 = temp1 & temp2;
    v8si temp4 = temp3 | blended;
    
    v3 = temp4;
    
    return temp4;
}

/* Main test function that combines everything */
__attribute__((noinline, noipa))
int test_all_patterns(void) {
    /* Initialize vectors with pattern values */
    v4si a4 = {1, 2, 3, 4};
    v4si b4 = {5, 6, 7, 8};
    v4si c4 = {9, 10, 11, 12};
    v4si d4 = {13, 14, 15, 16};
    v4si mask4 = {3, 2, 1, 0};
    v4si shuffle_mask4 = {1, 0, 3, 2};
    
    v4df ad = {1.0, 2.0, 3.0, 4.0};
    v4df bd = {5.0, 6.0, 7.0, 8.0};
    v4df cd = {9.0, 10.0, 11.0, 12.0};
    v4df dd = {13.0, 14.0, 15.0, 16.0};
    v4df ed = {17.0, 18.0, 19.0, 20.0};
    
    v8si a8 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b8 = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si c8 = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si d8 = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si mask8_1 = {0, 1, 2, 3, 4, 5, 6, 7};
    v8si mask8_2 = {7, 6, 5, 4, 3, 2, 1, 0};
    
    /* Call test functions */
    v4si result1 = test_10_operand_expansion(a4, b4, c4, d4, mask4, shuffle_mask4);
    v4df result2 = test_11_operand_expansion(ad, bd, cd, dd, ed, mask4);
    v8si result3 = test_avx2_many_operands(a8, b8, c8, d8, mask8_1, mask8_2);
    
    /* Compute checksums to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += result1[i];
    }
    
    double dchecksum = 0.0;
    for (int i = 0; i < 4; i++) {
        dchecksum += result2[i];
    }
    checksum += (int)dchecksum;
    
    for (int i = 0; i < 8; i++) {
        checksum += result3[i];
    }
    
    return checksum;
}

int main(void) {
    int result = test_all_patterns();
    
    /* Print result to ensure execution */
    printf("Test result checksum: %d\n", result);
    
    /* Return based on result to ensure no optimization */
    if (result != 0) {
        return 0; /* Success */
    } else {
        return 1; /* Should not happen with our inputs */
    }
}
