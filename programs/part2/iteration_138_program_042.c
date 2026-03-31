/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -mavx2 -fopenmp-simd -fdump-tree-optimized vector_test.c -o vector_test */

#include <stdio.h>
#include <stdint.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with AVX2 target attribute to ensure vector builtins are considered */
__attribute__((target("avx2"), noinline))
static v4si vector_operations(v4si a, v4si b, v4sf c, v4sf d) {
    volatile v4si result1, result2;
    volatile v4sf fresult;
    volatile v2di dresult;
    
    /* Basic vector arithmetic - may trigger builtin vectorized function creation */
    v4si add_result = a + b;
    v4si mul_result = a * b;
    
    /* Mix operations - encourages internal function generation */
    v4si complex_op = (a + b) * (a - b);
    
    /* Use __builtin_shuffle - strong candidate for artificial declaration */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    
    /* Type conversion using builtin - likely creates internal function */
    v4sf float_vec = __builtin_convertvector(a, v4sf);
    v4sf float_op = float_vec + c * d;
    
    /* Convert back - another potential internal function */
    v4si int_vec = __builtin_convertvector(float_op, v4si);
    
    /* Prevent optimization */
    result1 = add_result;
    result2 = int_vec;
    fresult = float_op;
    
    /* Return mixed result to ensure all code is live */
    return result1 + result2 + shuffled + complex_op;
}

/* Another function with different vector types */
__attribute__((noinline))
static v2di mixed_vector_ops(v2di x, v2di y) {
    volatile v2di result;
    
    /* Use architecture-specific builtin if available */
    #ifdef __SSE2__
    v2di sum = x + y;
    v2di diff = x - y;
    v2di prod = x * y;
    
    /* Complex expression */
    result = (sum + diff) * prod;
    #else
    result = x + y;
    #endif
    
    return result;
}

/* Function with OpenMP SIMD pragma */
__attribute__((noinline))
static void omp_simd_loop(int* restrict a, int* restrict b, int* restrict c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Mix scalar and vectorizable operations */
        a[i] = b[i] * c[i] + i;
    }
}

int main() {
    /* Initialize vector variables with patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.5f, 2.5f, 3.5f, 4.5f};
    v4sf vec_d = {0.5f, 1.5f, 2.5f, 3.5f};
    
    v2di vec_x = {100, 200};
    v2di vec_y = {50, 75};
    
    /* Call vector operations multiple times */
    v4si result1 = vector_operations(vec_a, vec_b, vec_c, vec_d);
    v4si result2 = vector_operations(vec_b, vec_a, vec_d, vec_c);
    
    v2di result3 = mixed_vector_ops(vec_x, vec_y);
    v2di result4 = mixed_vector_ops(vec_y, vec_x);
    
    /* Array for OpenMP SIMD test */
    int size = 256;
    int array_a[256], array_b[256], array_c[256];
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array_b[i] = i;
        array_c[i] = size - i;
    }
    
    /* Trigger OpenMP SIMD vectorization */
    omp_simd_loop(array_a, array_b, array_c, size);
    
    /* Aggregate results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += result1[i] + result2[i];
    }
    
    sum += result3[0] + result3[1];
    sum += result4[0] + result4[1];
    
    for (int i = 0; i < 16; i++) {  /* Check first 16 elements */
        sum += array_a[i];
    }
    
    printf("Result: %d\n", sum);
    
    return 0;
}
