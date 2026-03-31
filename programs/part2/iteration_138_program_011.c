/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -fopenmp-simd -mavx2 -fdump-tree-optimized -o vector_test vector_test.c */

#include <stdio.h>
#include <stdint.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));  /* For AVX2 */

/* Use volatile to prevent optimization */
volatile v4si global_vec_result;

/* Function with AVX2 target attribute to ensure vector built-ins are considered */
__attribute__((target("avx2"), noinline))
static v4si process_vectors(v4si a, v4si b, v4sf c, v2di d) {
    v4si result;
    
    /* Vector addition - may trigger builtin_vectorized_function */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Type conversion using builtin */
    v4si conv_result = __builtin_convertvector(c, v4si);
    
    /* Shuffle operation - strong candidate for artificial function creation */
    v4si shuffle_result = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    
    /* Mix operations to create complexity */
    result = add_result + mul_result - conv_result;
    
    /* Use shuffle in computation */
    result = __builtin_shuffle(result, shuffle_result, (v4si){3, 2, 1, 0});
    
    /* Store to volatile to prevent elimination */
    global_vec_result = result;
    
    return result;
}

/* Another function using different vector types */
__attribute__((noinline))
static v8hi process_small_vectors(v8hi a, v8hi b) {
    /* Multiple operations that might generate internal functions */
    v8hi add = a + b;
    v8hi sub = a - b;
    v8hi mul = a * b;
    
    /* Complex shuffle pattern */
    v8hi shuffled = __builtin_shuffle(add, sub, (v8hi){0, 8, 1, 9, 2, 10, 3, 11});
    
    /* Mix with multiplication */
    return shuffled + mul;
}

/* Function with OpenMP SIMD pragma */
__attribute__((noinline))
static void omp_simd_loop(int* restrict in, int* restrict out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Vectorizable operation mixing scalar and potential vector patterns */
        out[i] = in[i] * 3 + in[i] / 2;
        
        /* Use of conditional that can be vectorized with mask */
        if (in[i] > 100) {
            out[i] -= 50;
        }
    }
}

int main(void) {
    /* Initialize vector variables with mixed patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_f = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di vec_d = {9LL, 10LL};
    
    v8hi vec_small_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi vec_small_b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Call vector processing functions multiple times */
    v4si result1 = process_vectors(vec_a, vec_b, vec_f, vec_d);
    
    /* Modify inputs and call again */
    vec_a[0] = 10;
    vec_b[3] = 20;
    v4si result2 = process_vectors(vec_a, vec_b, vec_f, vec_d);
    
    /* Process small vectors */
    v8hi small_result = process_small_vectors(vec_small_a, vec_small_b);
    
    /* Test OpenMP SIMD loop */
    int input[256];
    int output[256];
    
    for (int i = 0; i < 256; i++) {
        input[i] = i;
    }
    
    omp_simd_loop(input, output, 256);
    
    /* Aggregate results to ensure all code is live */
    int total = 0;
    
    /* Sum vector elements */
    for (int i = 0; i < 4; i++) {
        total += result1[i] + result2[i];
    }
    
    /* Sum small vector elements */
    for (int i = 0; i < 8; i++) {
        total += small_result[i];
    }
    
    /* Sum OpenMP output */
    for (int i = 0; i < 256; i++) {
        total += output[i];
    }
    
    printf("Total: %d\n", total);
    
    /* Additional complex expression with mixed vector types */
    v4si temp = __builtin_shuffle(result1, result2, (v4si){0, 4, 1, 5});
    v4si final = temp + __builtin_convertvector(vec_f, v4si);
    
    /* Use final result */
    printf("Final vector: [%d, %d, %d, %d]\n", 
           final[0], final[1], final[2], final[3]);
    
    return 0;
}
