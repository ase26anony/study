/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -mavx2 -fopenmp-simd -fdump-tree-optimized -o vector_test vector_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with AVX2 target attribute to ensure vector builtins are considered */
__attribute__((target("avx2")))
__attribute__((noinline))
static v4si process_vectors(v4si a, v4si b, v4sf c, v2di d) {
    volatile v4si result1, result2;
    volatile v4sf float_result;
    volatile v2di double_int_result;
    
    /* Vector addition - may trigger builtin vectorized function */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Use builtin shuffle to create internal function declaration */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    
    /* Type conversion using builtin - strong candidate for artificial decl */
    v4sf converted = __builtin_convertvector(a, v4sf);
    
    /* Mix with different vector type */
    v2di di_vec = __builtin_convertvector(shuffled, v2di);
    
    /* Store to volatile to prevent optimization */
    result1 = add_result;
    result2 = mul_result;
    float_result = converted;
    double_int_result = di_vec;
    
    /* Complex expression mixing multiple operations */
    return (v4si)((add_result & mul_result) | shuffled);
}

/* Another function using explicit vector builtins */
__attribute__((target("avx2")))
__attribute__((noinline))
static v4si use_explicit_builtins(v4si a, v4si b) {
    /* Direct use of vector builtins - may create internal declarations */
    v4si result = a + b;
    
    /* Use shuffle with complex pattern */
    v4si shuffled = __builtin_shuffle(result, (v4si){3, 2, 1, 0});
    
    /* Simulate a vector comparison operation */
    v4si cmp_result = (a > b) ? a : b;
    
    return shuffled + cmp_result;
}

int main() {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf float_vec = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di di_vec = {9LL, 10LL};
    
    v4si final_result = {0, 0, 0, 0};
    
    /* Call vector processing function multiple times */
    for (int i = 0; i < 100; i++) {
        vec1[0] = i;
        vec2[3] = i * 2;
        
        v4si processed = process_vectors(vec1, vec2, float_vec, di_vec);
        v4si with_builtins = use_explicit_builtins(vec1, vec2);
        
        final_result += processed + with_builtins;
    }
    
    /* OpenMP SIMD loop - may trigger creation of vectorized versions */
    int array[1024];
    int sum = 0;
    
    #pragma omp simd
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 2;
    }
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 1024; i++) {
        sum += array[i];
    }
    
    /* Mix scalar and vector operations in SIMD loop */
    v4si accum = {0, 0, 0, 0};
    #pragma omp simd
    for (int i = 0; i < 256; i++) {
        v4si temp = {i, i+1, i+2, i+3};
        accum += temp;
    }
    
    /* Complex vector expression using multiple types */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si converted_from_short = __builtin_convertvector(short_vec, v4si);
    
    /* Use architecture-specific builtin if available */
    #ifdef __SSE2__
    v4si sse_result = __builtin_ia32_paddd128(vec1, vec2);
    final_result += sse_result;
    #endif
    
    /* Aggregate scalar result from vector elements */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += final_result[i] + accum[i] + converted_from_short[i];
    }
    
    total += sum / 1000;  /* Use sum to prevent dead code elimination */
    
    printf("Result: %d\n", total);
    
    return 0;
}
