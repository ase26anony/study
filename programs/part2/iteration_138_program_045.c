/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -mavx2 -fopenmp-simd -fdump-tree-optimized -o vector_test vector_test.c */

#include <stdio.h>
#include <stdlib.h>

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
    v4si a = {seed, seed + 1, seed + 2, seed + 3};
    v4si b = {seed + 4, seed + 5, seed + 6, seed + 7};
    v4sf c = {seed * 1.0f, seed * 2.0f, seed * 3.0f, seed * 4.0f};
    v4sf d = {seed * 0.5f, seed * 1.5f, seed * 2.5f, seed * 3.5f};
    
    /* Operation 1: Vector addition using built-in operators */
    v4si add_result = a + b;
    
    /* Operation 2: Vector multiplication */
    v4si mul_result = a * b;
    
    /* Operation 3: Mixed operations encouraging internal function creation */
    v4si mixed = (a + b) * (a - b);
    
    /* Operation 4: Use __builtin_shuffle to create permutation */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si shuffled = __builtin_shuffle(a, shuffle_mask);
    
    /* Operation 5: Type conversion using built-in */
    v4sf float_vec = __builtin_convertvector(a, v4sf);
    
    /* Operation 6: More complex expression with multiple conversions */
    v4sf complex_float = __builtin_convertvector(mixed, v4sf) + 
                         __builtin_convertvector(shuffled, v4sf);
    
    /* Operation 7: Architecture-specific style built-in (may trigger internal decl) */
    v4si builtin_style = __builtin_ia32_paddd128(a, b);
    
    /* Store to volatile to prevent optimization */
    *out1 = add_result + mul_result + shuffled + builtin_style;
    *out2 = float_vec + complex_float + c * d;
    
    /* Work with different vector sizes */
    v2di di_vec = {seed * 100LL, seed * 200LL};
    v2di di_vec2 = {seed * 300LL, seed * 400LL};
    v2di di_result = di_vec + di_vec2;
    
    /* Convert between different vector types */
    v8hi hi_vec = __builtin_convertvector(a, v8hi);
    v4si back_converted = __builtin_convertvector(hi_vec, v4si);
    
    *out3 = di_result;
    *out1 = *out1 + back_converted;
}

/* Another function with OpenMP SIMD pragma */
__attribute__((noinline))
static void omp_simd_operations(float *arr1, float *arr2, float *result, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Mixed scalar-vector operations inside loop */
        v4sf tmp = {arr1[i], arr1[i] * 2.0f, arr1[i] * 3.0f, arr1[i] * 4.0f};
        v4sf tmp2 = {arr2[i], arr2[i] * 0.5f, arr2[i] * 1.5f, arr2[i] * 2.5f};
        
        /* Vector operation that may be expanded to built-in */
        v4sf vec_result = tmp * tmp2 + __builtin_convertvector(
            (v4si){i, i+1, i+2, i+3}, v4sf);
        
        /* Reduce to scalar */
        result[i] = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    }
}

int main() {
    volatile v4si vec_result1;
    volatile v4sf vec_result2;
    volatile v2di vec_result3;
    
    /* Call vector operations multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        vector_operations(&vec_result1, &vec_result2, &vec_result3, i * 10);
    }
    
    /* OpenMP SIMD section */
    const int n = 1024;
    float *arr1 = malloc(n * sizeof(float));
    float *arr2 = malloc(n * sizeof(float));
    float *result = malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = i * 0.1f;
        arr2[i] = i * 0.2f;
    }
    
    omp_simd_operations(arr1, arr2, result, n);
    
    /* Aggregate results to ensure all code is live */
    float total = 0.0f;
    for (int i = 0; i < n; i++) {
        total += result[i];
    }
    
    /* Also aggregate vector results */
    int vec_sum = 0;
    for (int i = 0; i < 4; i++) {
        vec_sum += vec_result1[i];
    }
    
    printf("Total: %f, Vector sum: %d\n", total, vec_sum);
    
    free(arr1);
    free(arr2);
    free(result);
    
    return 0;
}
