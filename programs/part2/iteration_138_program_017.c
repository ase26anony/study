/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -mavx2 -fopenmp-simd -fdump-tree-optimized -o vector_test vector_test.c */

#include <stdio.h>
#include <stdint.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with AVX2 target attribute to ensure vector built-ins are considered */
__attribute__((target("avx2"), noinline))
static v4si vector_operations(v4si a, v4si b, v4sf c, v2di d) {
    volatile v4si result1, result2;
    volatile v4sf float_result;
    volatile v2di double_int_result;
    
    /* Vector addition - may trigger builtin vectorized function creation */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Use builtin shuffle with a mask */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    
    /* Type conversion using builtin */
    v4sf converted = __builtin_convertvector(a, v4sf);
    
    /* Mixed operations to encourage internal function creation */
    float_result = converted + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Another conversion */
    v2di conv2 = __builtin_convertvector((v4si){a[0], a[1], b[0], b[1]}, v2di);
    double_int_result = conv2 + d;
    
    /* Complex expression mixing results */
    result1 = add_result + shuffled;
    result2 = mul_result - shuffled;
    
    /* Return a combination to prevent optimization */
    return result1 + result2;
}

/* Another function with different vector types */
__attribute__((noinline))
static v8hi process_shorts(v8hi a, v8hi b) {
    /* Use architecture-specific built-in if available */
    v8hi result = a + b;
    
    /* Shuffle with complex pattern */
    v8hi shuffled = __builtin_shuffle(a, b, (v8hi){7, 6, 5, 4, 3, 2, 1, 0});
    
    /* Mixed operations */
    return result * shuffled;
}

int main() {
    /* Initialize vectors with patterns */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf vec3 = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di vec4 = {100LL, 200LL};
    v8hi vec5 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi vec6 = {9, 10, 11, 12, 13, 14, 15, 16};
    
    int total = 0;
    
    /* Call vector operations multiple times */
    for (int i = 0; i < 100; i++) {
        v4si result = vector_operations(vec1, vec2, vec3, vec4);
        
        /* Modify inputs slightly each iteration */
        vec1[0] += 1;
        vec2[3] -= 1;
        
        /* Sum elements to create live output */
        total += result[0] + result[1] + result[2] + result[3];
    }
    
    /* Process shorts */
    v8hi short_result = process_shorts(vec5, vec6);
    for (int i = 0; i < 8; i++) {
        total += short_result[i];
    }
    
    /* OpenMP SIMD loop - may trigger vectorized version creation */
    int array[1024];
    #pragma omp simd
    for (int i = 0; i < 1024; i++) {
        /* Mix scalar and vector-like operations */
        array[i] = i * 2 + (i % 4);
    }
    
    /* Use array to prevent dead code elimination */
    for (int i = 0; i < 1024; i += 64) {
        total += array[i];
    }
    
    /* Additional complex vector expression */
    v2df double_vec1 = {1.0, 2.0};
    v2df double_vec2 = {3.0, 4.0};
    v2df double_result = double_vec1 * double_vec2 + (v2df){0.5, 1.5};
    
    total += (int)(double_result[0] + double_result[1]);
    
    printf("Result: %d\n", total);
    return 0;
}
