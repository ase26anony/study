/* test_optabs_10_operands.c
 * Designed to trigger case 10: in optabs.cc lines 8254-8263
 * Compile with: gcc -O3 -ftree-vectorize -mavx2 -c test_optabs_10_operands.c -fdump-rtl-expand
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

/* Complex shuffle/permute function that requires many operands */
static v8si complex_vector_shuffle(v8si a, v8si b, v8si c, v8si d) {
    /* This complex expression should require many operands during expansion:
     * 1. Multiple source vectors (a, b, c, d)
     * 2. Multiple immediate control values for shuffling
     * 3. Multiple arithmetic operations
     */
    
    /* Create a complex permutation pattern using multiple builtins */
    v8si temp1 = __builtin_shuffle(a, b, (v8si){0, 2, 4, 6, 8, 10, 12, 14});
    v8si temp2 = __builtin_shuffle(c, d, (v8si){1, 3, 5, 7, 9, 11, 13, 15});
    
    /* Mix with arithmetic operations and constants */
    v8si result = temp1 + temp2;
    result = result * (v8si){1, 2, 3, 4, 5, 6, 7, 8};
    result = result & (v8si){0xFF, 0xFFFF, 0xFF, 0xFFFF, 0xFF, 0xFFFF, 0xFF, 0xFFFF};
    
    /* Final complex shuffle with many control elements */
    return __builtin_shuffle(result, 
                            result + (v8si){100, 200, 300, 400, 500, 600, 700, 800},
                            (v8si){7, 6, 5, 4, 3, 2, 1, 0});
}

/* Another approach: Use vector conversions with complex patterns */
static v8sf vector_conversion_test(v8si int_vec) {
    /* Multiple conversion steps with different operations */
    v8sf float_vec = __builtin_convertvector(int_vec, v8sf);
    v8sf scaled = float_vec * (v8sf){1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f};
    
    /* Complex shuffle with immediate control mask */
    return __builtin_shuffle(scaled, 
                            scaled + (v8sf){10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f},
                            (v8si){0, 8, 1, 9, 2, 10, 3, 11});
}

/* Test function with many parameters to potentially trigger 10-operand expansion */
static v8si multi_operand_expression(v8si a, v8si b, v8si c, v8si d,
                                     v8si e, v8si f, v8si g, v8si h,
                                     int imm1, int imm2) {
    /* This complex expression uses many operands:
     * - 8 vector parameters
     * - 2 immediate parameters
     * - Multiple operations that might be combined into one optab expansion
     */
    
    /* Create a data-dependent selection pattern */
    v8si mask = a > b;
    v8si selected = __builtin_shufflevector(c, d, 
         (mask[0] ? 0 : 8), (mask[1] ? 1 : 9), (mask[2] ? 2 : 10), (mask[3] ? 3 : 11),
         (mask[4] ? 4 : 12), (mask[5] ? 5 : 13), (mask[6] ? 6 : 14), (mask[7] ? 7 : 15));
    
    /* Mix with other vectors using arithmetic */
    selected = selected + e;
    selected = selected * f;
    selected = selected | g;
    selected = selected ^ h;
    
    /* Use immediates in shuffle control */
    return __builtin_shuffle(selected, 
                            selected + (v8si){imm1, imm2, imm1, imm2, imm1, imm2, imm1, imm2},
                            (v8si){0, 8, 2, 10, 4, 12, 6, 14});
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vectors with different patterns */
    v8si vec1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si vec3 = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si vec4 = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si vec5 = {33, 34, 35, 36, 37, 38, 39, 40};
    v8si vec6 = {41, 42, 43, 44, 45, 46, 47, 48};
    v8si vec7 = {49, 50, 51, 52, 53, 54, 55, 56};
    v8si vec8 = {57, 58, 59, 60, 61, 62, 63, 64};
    
    v8si accumulator = {0};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use different expressions to increase chance of hitting target */
        if (i % 3 == 0) {
            accumulator = accumulator + complex_vector_shuffle(vec1, vec2, vec3, vec4);
        } else if (i % 3 == 1) {
            v8sf float_result = vector_conversion_test(accumulator);
            /* Convert back and add to accumulator */
            v8si int_result = __builtin_convertvector(float_result, v8si);
            accumulator = accumulator + int_result;
        } else {
            accumulator = multi_operand_expression(vec1, vec2, vec3, vec4,
                                                  vec5, vec6, vec7, vec8,
                                                  i, i * 2);
        }
        
        /* Modify vectors slightly each iteration */
        vec1 = vec1 + (v8si){1};
        vec2 = vec2 + (v8si){2};
    }
    
    /* Print result to ensure side effects */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += accumulator[i];
    }
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
