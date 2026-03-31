/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdint.h>
#include <stdio.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Even larger types for AVX-512 if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

/* Force no inlining to prevent optimization */
__attribute__((noinline, noipa))
v8si test_10_operands(v8si a, v8si b, v8si c, v8si d, v8si e) {
    volatile v8si temp1, temp2, temp3;
    
    /* Complex shuffle operation - may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    temp1 = shuffled;  /* Force memory store */
    
    /* Vector conditional expression with arithmetic */
    v8si cmp_result = (a > b) ? (c * d) : (d - e);
    temp2 = cmp_result;
    
    /* Complex blend-like operation using conditional */
    v8si blend_mask = (shuffled > cmp_result);
    v8si blended = blend_mask ? (a + b) : (c - d);
    
    /* Another shuffle with the blended result */
    v8si shuffle_mask2 = {0, 2, 4, 6, 1, 3, 5, 7};
    v8si final_shuffle = __builtin_shuffle(blended, e, shuffle_mask2);
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    /* Final arithmetic combining everything */
    v8si result = final_shuffle + temp1 + temp2;
    
    return result;
}

__attribute__((noinline, noipa))
v4df test_11_operands(v4df a, v4df b, v4df c, v4df d, v4df e, v4df f) {
    volatile v4df temp1, temp2, temp3;
    
    /* Complex floating-point vector operations */
    v4df mul_result = a * b;
    temp1 = mul_result;
    
    v4df add_result = c + d;
    temp2 = add_result;
    
    /* Vector conditional with floating point */
    v4df cmp_mask = (a > b);
    v4df select_result = cmp_mask ? (mul_result / add_result) : (e * f);
    
    /* Shuffle with floating-point vectors */
    long long shuffle_idx[4] = {1, 0, 3, 2};
    v4df shuffled = __builtin_shufflevector(select_result, e, 
                                           shuffle_idx[0], shuffle_idx[1],
                                           shuffle_idx[2], shuffle_idx[3]);
    
    /* Complex expression requiring many temporaries */
    v4df expr1 = (a + b) * (c - d);
    v4df expr2 = (e + f) / (a - b);
    
    v4df final_cmp = (expr1 > expr2);
    v4df final_select = final_cmp ? shuffled : select_result;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Combine all results - this may require many operands */
    v4df result = final_select + temp1 + temp2 + expr1 + expr2;
    
    return result;
}

/* Test with AVX-512 if available */
#ifdef __AVX512F__
__attribute__((noinline, noipa))
v16si test_avx512_many_operands(v16si a, v16si b, v16si c, v16si d) {
    volatile v16si temp1, temp2;
    
    /* Large vector shuffle - likely to require many operands */
    int shuffle_mask[16] = {15, 14, 13, 12, 11, 10, 9, 8,
                           7, 6, 5, 4, 3, 2, 1, 0};
    
    /* Use __builtin_shuffle with a large mask */
    v16si shuffled = __builtin_shuffle(a, b, 
        (v16si){shuffle_mask[0], shuffle_mask[1], shuffle_mask[2], shuffle_mask[3],
                shuffle_mask[4], shuffle_mask[5], shuffle_mask[6], shuffle_mask[7],
                shuffle_mask[8], shuffle_mask[9], shuffle_mask[10], shuffle_mask[11],
                shuffle_mask[12], shuffle_mask[13], shuffle_mask[14], shuffle_mask[15]});
    
    temp1 = shuffled;
    
    /* Complex conditional operation on large vectors */
    v16si mask = (a > b) & (c < d);
    v16si result = mask ? (a * b + c) : (d - a);
    
    /* Another shuffle */
    v16si shuffle2 = __builtin_shuffle(result, c,
        (v16si){0, 2, 4, 6, 8, 10, 12, 14,
                1, 3, 5, 7, 9, 11, 13, 15});
    
    asm volatile("" ::: "memory");
    
    return shuffle2 + temp1;
}
#endif

int main() {
    /* Initialize test vectors with pattern values */
    v8si v8a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v8b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si v8c = {2, 3, 4, 5, 6, 7, 8, 9};
    v8si v8d = {9, 8, 7, 6, 5, 4, 3, 2};
    v8si v8e = {3, 4, 5, 6, 7, 8, 9, 10};
    
    v4df v4a = {1.0, 2.0, 3.0, 4.0};
    v4df v4b = {4.0, 3.0, 2.0, 1.0};
    v4df v4c = {1.5, 2.5, 3.5, 4.5};
    v4df v4d = {4.5, 3.5, 2.5, 1.5};
    v4df v4e = {0.5, 1.5, 2.5, 3.5};
    v4df v4f = {3.5, 2.5, 1.5, 0.5};
    
    /* Call test functions */
    v8si result1 = test_10_operands(v8a, v8b, v8c, v8d, v8e);
    v4df result2 = test_11_operands(v4a, v4b, v4c, v4d, v4e, v4f);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    double checksum2 = 0.0;
    
    for (int i = 0; i < 8; i++) {
        checksum1 += result1[i];
    }
    
    for (int i = 0; i < 4; i++) {
        checksum2 += result2[i];
    }
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    v16si v16a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    v16si v16b = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    v16si v16c = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};
    v16si v16d = {32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2};
    
    v16si result3 = test_avx512_many_operands(v16a, v16b, v16c, v16d);
    
    int checksum3 = 0;
    for (int i = 0; i < 16; i++) {
        checksum3 += result3[i];
    }
    
    printf("Checksums: %d, %.2f, %d\n", checksum1, checksum2, checksum3);
    return (checksum1 != 0 && checksum2 != 0.0 && checksum3 != 0) ? 0 : 1;
#else
    printf("Checksums: %d, %.2f\n", checksum1, checksum2);
    return (checksum1 != 0 && checksum2 != 0.0) ? 0 : 1;
#endif
}
