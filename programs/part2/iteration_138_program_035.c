/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -mavx2 -fopenmp-simd -fdump-tree-optimized vector_test.c -o vector_test */

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
static void vector_operations(volatile v4si *out1, volatile v4sf *out2, 
                              volatile v2di *out3, int seed) {
    /* Initialize vectors with patterns */
    v4si a = {seed + 1, seed + 2, seed + 3, seed + 4};
    v4si b = {seed + 5, seed + 6, seed + 7, seed + 8};
    v4si c = {seed + 9, seed + 10, seed + 11, seed + 12};
    
    /* Vector addition - may trigger builtin vectorized function creation */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = add_result * c;
    
    /* Use __builtin_shuffle to create internal function declaration */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si shuffled = __builtin_shuffle(mul_result, shuffle_mask);
    
    /* Type conversion using builtin - strong candidate for artificial decl */
    v4sf float_vec = __builtin_convertvector(shuffled, v4sf);
    
    /* Mix with different vector type */
    v2di double_int = __builtin_convertvector(shuffled, v2di);
    
    /* Store to volatile to prevent optimization */
    *out1 = shuffled;
    *out2 = float_vec;
    *out3 = double_int;
    
    /* Additional architecture-specific built-in (x86 SSE/AVX) */
    #ifdef __x86_64__
    v4si builtin_add = __builtin_ia32_paddd128(a, b);
    *out1 = builtin_add;
    #endif
}

/* Another function with mixed operations */
__attribute__((noinline))
static v4si complex_vector_expr(v4si x, v4si y, v4si z) {
    /* Complex expression that might require internal helper */
    return (x + y) * z - (x & y) | (z << 2);
}

/* OpenMP SIMD loop that should trigger vectorized version creation */
__attribute__((noinline))
static int omp_simd_loop(int *arr, int n) {
    int sum = 0;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Mix scalar and vectorizable operations */
        arr[i] = arr[i] * 3 + i;
        sum += arr[i];
    }
    
    return sum;
}

int main(void) {
    volatile v4si result1;
    volatile v4sf result2;
    volatile v2di result3;
    
    /* Call vector operations multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        vector_operations(&result1, &result2, &result3, i * 100);
    }
    
    /* Test complex vector expressions */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    v4si complex_result = complex_vector_expr(vec1, vec2, vec3);
    
    /* Test OpenMP SIMD vectorization */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    int sum = omp_simd_loop(arr, 100);
    
    /* Aggregate results to ensure code is live */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += complex_result[i];
    }
    total += sum;
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (checksum: %d)\n", total, 
           total + result1[0] + (int)result2[0] + (int)result3[0]);
    
    return 0;
}
