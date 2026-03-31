/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -fopenmp-simd -mavx2 -fdump-tree-optimized vector_test.c -o vector_test */

#include <stdio.h>
#include <stdlib.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization by using volatile */
volatile v4si global_vec_result;

/* Function with AVX2 target attribute to ensure vector built-ins are considered */
__attribute__((target("avx2")))
__attribute__((noinline))
static v4si process_vectors(v4si a, v4si b, v4sf c, v2di d) {
    /* Force creation of artificial declarations through various operations */
    
    /* 1. Vector arithmetic operations */
    v4si add_result = a + b;
    v4si mul_result = a * b;
    
    /* 2. Type conversion using builtin */
    v4si conv_result = __builtin_convertvector(c, v4si);
    
    /* 3. Shuffle operation */
    v4si shuffle_result = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    
    /* 4. Mix with different vector type */
    v2di wide_result = __builtin_convertvector(a, v2di) + d;
    
    /* 5. More complex expression combining multiple operations */
    v4si complex_result = (add_result * conv_result) + shuffle_result;
    
    /* 6. Use architecture-specific built-in if available */
    #ifdef __SSE2__
    v4si sse_result = __builtin_ia32_paddd128(add_result, mul_result);
    complex_result = complex_result + sse_result;
    #endif
    
    /* Store to volatile to prevent elimination */
    global_vec_result = complex_result;
    
    return complex_result;
}

/* Another function to trigger OpenMP SIMD vectorization */
__attribute__((noinline))
static int omp_simd_loop(int *arr, int n) {
    int sum = 0;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Mixed scalar and vectorizable operations */
        arr[i] = arr[i] * 2 + i;
        sum += arr[i];
    }
    
    return sum;
}

int main(void) {
    /* Initialize vector data with patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di vec_d = {9LL, 10LL};
    
    /* Call vector processing function multiple times */
    v4si result1 = process_vectors(vec_a, vec_b, vec_c, vec_d);
    
    /* Modify inputs and call again */
    vec_a = vec_a + (v4si){10, 20, 30, 40};
    v4si result2 = process_vectors(vec_a, vec_b, vec_c, vec_d);
    
    /* Create array for OpenMP SIMD test */
    int n = 1024;
    int *arr = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        arr[i] = i % 100;
    }
    
    /* Trigger OpenMP SIMD vectorization */
    int sum = omp_simd_loop(arr, n);
    
    /* Additional vector operations using different types */
    v8hi short_vec1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi short_vec2 = {8, 7, 6, 5, 4, 3, 2, 1};
    v8hi short_result = short_vec1 + short_vec2;
    
    v2df double_vec1 = {1.0, 2.0};
    v2df double_vec2 = {3.0, 4.0};
    v2df double_result = double_vec1 * double_vec2;
    
    /* Aggregate results to ensure all code is live */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += result1[i] + result2[i];
    }
    
    for (int i = 0; i < 8; i++) {
        total += short_result[i];
    }
    
    total += (int)(double_result[0] + double_result[1]);
    total += sum % 1000;
    
    printf("Result: %d\n", total);
    
    free(arr);
    return 0;
}
