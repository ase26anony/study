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
__attribute__((target("avx2")))
__attribute__((noinline))
static v4si process_vectors(v4si a, v4si b, v4sf c, v2di d) {
    volatile v4si result1, result2;
    volatile v4sf float_result;
    volatile v2di double_int_result;
    
    /* Vector addition - may trigger builtin_vectorized_function */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Use __builtin_shuffle to create internal function declaration */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){0, 1, 2, 3});
    
    /* Type conversion using builtin - strong candidate for artificial decl */
    v4sf converted = __builtin_convertvector(a, v4sf);
    
    /* Mix operations between different vector types */
    v4sf mixed_ops = converted + c;
    
    /* Another shuffle with different mask */
    v4si shuffle2 = __builtin_shuffle(mul_result, add_result, (v4si){3, 2, 1, 0});
    
    /* Store to volatile to prevent optimization */
    result1 = shuffled;
    result2 = shuffle2;
    float_result = mixed_ops;
    
    /* Return a combination of results */
    return result1 + result2;
}

/* Another function with different vector operations */
__attribute__((target("avx2")))
__attribute__((noinline))
static v2di process_large_vectors(v2di a, v2di b) {
    /* Direct use of x86-specific builtin (if available) */
    #ifdef __x86_64__
    /* This builtin often creates artificial declarations */
    v2di sum = a + b;
    #else
    v2di sum = a + b;
    #endif
    
    /* Shuffle for 64-bit vectors */
    v2di shuffled = __builtin_shuffle(a, b, (v2di){0, 1});
    
    return sum + shuffled;
}

int main() {
    /* Initialize vectors with patterns */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf vec3 = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di vec4 = {100LL, 200LL};
    v2di vec5 = {300LL, 400LL};
    
    int total_sum = 0;
    
    /* Call vector processing function multiple times */
    for (int i = 0; i < 10; i++) {
        v4si result = process_vectors(vec1, vec2, vec3, vec4);
        
        /* Extract and sum elements to ensure code is live */
        for (int j = 0; j < 4; j++) {
            total_sum += result[j];
        }
        
        /* Modify inputs slightly each iteration */
        vec1[0] += i;
        vec2[3] -= i;
    }
    
    /* Process large vectors */
    v2di large_result = process_large_vectors(vec4, vec5);
    total_sum += large_result[0] + large_result[1];
    
    /* OpenMP SIMD loop - may trigger vectorized version creation */
    int array[1024];
    #pragma omp simd
    for (int i = 0; i < 1024; i++) {
        /* Mix scalar and vector operations */
        v4si temp = {i, i+1, i+2, i+3};
        array[i] = temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    /* Sum array to prevent dead code elimination */
    int array_sum = 0;
    for (int i = 0; i < 1024; i++) {
        array_sum += array[i];
    }
    
    total_sum += array_sum;
    
    printf("Result: %d\n", total_sum);
    
    return 0;
}
