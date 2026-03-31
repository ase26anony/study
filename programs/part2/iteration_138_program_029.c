/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -fopenmp-simd -mavx2 -fdump-tree-optimized vector_test.c -o vector_test */

#include <stdio.h>
#include <stdint.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with AVX2 target attribute to ensure vector built-ins are considered */
__attribute__((target("avx2")))
__attribute__((noinline))
static v4si vector_operations(v4si a, v4si b, v4sf c, v2di d) {
    volatile v4si result1, result2;
    volatile v4sf float_result;
    volatile v2di double_int_result;
    
    /* Vector addition - may trigger built-in vectorized function creation */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Use built-in shuffle to encourage internal function creation */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    
    /* Type conversion using built-in - strong candidate for artificial decl */
    v4sf converted = __builtin_convertvector(a, v4sf);
    
    /* Mix operations - convert back and add */
    v4si reconverted = __builtin_convertvector(converted, v4si);
    v4si mixed = reconverted + shuffled;
    
    /* Store to volatile to prevent optimization */
    result1 = add_result + mul_result;
    result2 = mixed;
    float_result = converted + c;
    
    /* Use architecture-specific built-in if available */
    #ifdef __SSE2__
    v2di packed = __builtin_ia32_paddd128((v2di)add_result, d);
    double_int_result = packed;
    #endif
    
    return result1 + result2;
}

/* Another function with different vector types */
__attribute__((noinline))
static v8hi process_shorts(v8hi a, v8hi b) {
    /* Complex expression with multiple operations */
    v8hi result = a + b;
    result = result * (v8hi){1, 2, 1, 2, 1, 2, 1, 2};
    
    /* Shuffle with non-constant mask to prevent compile-time evaluation */
    int mask[8] = {1, 0, 3, 2, 5, 4, 7, 6};
    v8hi shuffled = __builtin_shuffle(result, result, *(v8hi*)mask);
    
    return result + shuffled;
}

int main() {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf float_vec = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di long_vec = {9LL, 10LL};
    
    v8hi short_vec1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi short_vec2 = {8, 7, 6, 5, 4, 3, 2, 1};
    
    int sum = 0;
    
    /* Call vector operations multiple times */
    for (int i = 0; i < 100; i++) {
        v4si result = vector_operations(vec1, vec2, float_vec, long_vec);
        
        /* OpenMP SIMD pragma to trigger vectorized version creation */
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 4; j++) {
            sum += result[j];
        }
        
        /* Modify inputs slightly */
        vec1[0] += 1;
        vec2[3] -= 1;
    }
    
    /* Process short vectors */
    v8hi short_result = process_shorts(short_vec1, short_vec2);
    
    /* Sum all elements to ensure code isn't eliminated */
    int short_sum = 0;
    for (int i = 0; i < 8; i++) {
        short_sum += short_result[i];
    }
    
    printf("Total sum: %d, Short sum: %d\n", sum, short_sum);
    
    /* Additional complex expression with mixed types */
    v2df double_vec1 = {1.0, 2.0};
    v2df double_vec2 = {3.0, 4.0};
    v2df double_result = double_vec1 * double_vec2;
    
    /* Force use of built-in convertvector with different types */
    v4si int_from_double = __builtin_convertvector(double_result, v4si);
    
    printf("Converted: %d %d %d %d\n", 
           int_from_double[0], int_from_double[1], 
           int_from_double[2], int_from_double[3]);
    
    return 0;
}
