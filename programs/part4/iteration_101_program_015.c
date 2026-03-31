/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

/* Use volatile for memory operations to prevent optimization */
static volatile v4si v4si_volatile;
static volatile v4df v4df_volatile;
static volatile v8si v8si_volatile;

/* Compiler barrier */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test function with many vector operations - marked noinline to prevent optimization */
__attribute__((noinline, target("avx2")))
v8si test_many_operands(v4si a, v4si b, v4df c, v4df d, v8si e, v8si f, v8sf g, v8sf h) {
    /* Operation 1: Complex shuffle with many operands */
    v4si shuffle_temp1, shuffle_temp2;
    
    /* Create a non-constant mask for shuffle */
    int mask_arr[4] = {1, 0, 3, 2};
    v4si mask = *(v4si*)mask_arr;
    
    /* Shuffle operation that may expand to many operands */
    shuffle_temp1 = __builtin_shuffle(a, b, mask);
    COMPILER_BARRIER();
    
    /* Store to volatile to force the operation */
    v4si_volatile = shuffle_temp1;
    
    /* Operation 2: Vector conditional expression with arithmetic */
    v4df cmp_mask = c > d;
    v4df cond_result = cmp_mask ? c * d + c : d / c - d;
    
    /* Complex expression that may require many temporaries */
    v4df complex_expr = (c + d) * (c - d) / (c * d + 1.0);
    COMPILER_BARRIER();
    
    v4df_volatile = cond_result + complex_expr;
    
    /* Operation 3: Large vector operation with blending */
    v8si blend_mask = e > f;
    
    /* This complex expression may require many operands during expansion */
    v8si blended = blend_mask ? 
        (e + f) * (e - f) : 
        (f + e) / (f - e + 1);
    
    /* Force memory store */
    v8si_volatile = blended;
    COMPILER_BARRIER();
    
    /* Operation 4: Convert between vector types - may require many operands */
    v8sf float_from_int = __builtin_convertvector(blended, v8sf);
    
    /* Operation 5: Another shuffle with 8-element vectors */
    int mask8[8] = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si reverse_mask = *(v8si*)mask8;
    v8si reversed = __builtin_shuffle(blended, reverse_mask);
    
    /* Combine all results into final checksum */
    v8si result = reversed + __builtin_convertvector(float_from_int, v8si);
    
    return result;
}

/* Another test function focusing on exactly 10/11 operands */
__attribute__((noinline, target("avx512f")))
v8si test_avx512_patterns(v8si a, v8si b, v8si c, v8si d, v8si e, v8si f) {
    /* Complex chain of operations that may require many temporary registers */
    v8si temp1 = a * b + c;
    v8si temp2 = d / (e + 1);
    v8si temp3 = f - a;
    
    /* Vector comparison that generates mask */
    v8si cmp = a > b;
    
    /* Conditional select with complex expressions on both sides */
    v8si result = cmp ? 
        (temp1 * temp2 + temp3) / (a + 1) :
        (temp2 - temp1) * (temp3 + b);
    
    /* Additional shuffle to potentially trigger many-operand pattern */
    int complex_mask[8] = {3, 2, 1, 0, 7, 6, 5, 4};
    v8si shuffle_mask = *(v8si*)complex_mask;
    v8si shuffled = __builtin_shuffle(result, shuffle_mask);
    
    return shuffled;
}

/* Test with floating point vectors */
__attribute__((noinline, target("avx")))
v4df test_fp_many_ops(v4df a, v4df b, v4df c, v4df d, v4df e, v4df f) {
    /* Complex FP expression that may expand to many operands */
    v4df temp1 = a * b + c;
    v4df temp2 = d / e - f;
    v4df temp3 = (a + b) * (c - d);
    
    /* Conditional with FP comparison */
    v4df cmp = a > b;
    v4df result = cmp ? temp1 * temp2 : temp3 / temp1;
    
    /* Additional arithmetic to increase operand count */
    result = result + (temp2 * temp3) / (a + 1.0);
    
    return result;
}

int main() {
    /* Initialize vectors with pattern values */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    v4df c = {1.0, 2.0, 3.0, 4.0};
    v4df d = {5.0, 6.0, 7.0, 8.0};
    v4df e = {9.0, 10.0, 11.0, 12.0};
    v4df f = {13.0, 14.0, 15.0, 16.0};
    
    v8si v8a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v8b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si v8c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si v8d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si v8e = {33, 34, 35, 36, 37, 38, 39, 40};
    v8si v8f = {41, 42, 43, 44, 45, 46, 47, 48};
    
    v8sf v8g = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v8h = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Call test functions to trigger expansion */
    v8si result1 = test_many_operands(a, b, c, d, v8a, v8b, v8g, v8h);
    COMPILER_BARRIER();
    
    v8si result2 = test_avx512_patterns(v8a, v8b, v8c, v8d, v8e, v8f);
    COMPILER_BARRIER();
    
    v4df fp_result = test_fp_many_ops(c, d, e, f, c, d);
    COMPILER_BARRIER();
    
    /* Compute checksums to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += result1[i];
        checksum += result2[i];
    }
    
    for (int i = 0; i < 4; i++) {
        checksum += (int)fp_result[i];
    }
    
    /* Use checksum to affect program output */
    if (checksum != 0) {
        printf("Checksum: %d\n", checksum);
        return 0;
    }
    
    return 1;
}
