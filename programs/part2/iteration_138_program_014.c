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
static v4si vector_operations(v4si a, v4si b, v4sf c, v4sf d) {
    /* Use volatile to prevent optimization */
    volatile v4si result1, result2;
    volatile v4sf fresult;
    volatile v2di dresult;
    
    /* Vector addition - may trigger built-in vectorized function */
    result1 = a + b;
    
    /* Vector multiplication */
    result2 = a * b;
    
    /* Type conversion using builtin */
    v4sf converted = __builtin_convertvector(result1, v4sf);
    
    /* Shuffle operation using builtin */
    v4si shuffled = __builtin_shuffle(result2, (v4si){3, 2, 1, 0});
    
    /* Mix with float vectors */
    fresult = c * d + converted;
    
    /* Convert to different vector type */
    dresult = __builtin_convertvector(shuffled, v2di);
    
    /* Complex expression mixing results */
    v4si final_vec = result1 + shuffled + __builtin_convertvector(fresult, v4si);
    
    return final_vec;
}

/* Another function with different vector operations */
__attribute__((target("avx2"), noinline))
static v4sf float_vector_ops(v4sf a, v4sf b) {
    volatile v4sf result;
    
    /* Use architecture-specific built-in if available */
    #ifdef __SSE__
    /* This may create internal artificial declarations */
    result = a + b;
    result = result * a;
    #endif
    
    /* Shuffle with mask */
    v4sf shuffled = __builtin_shuffle(result, (v4si){1, 0, 3, 2});
    
    return shuffled + b;
}

/* Function with OpenMP SIMD pragma */
__attribute__((noinline))
static void omp_simd_loop(int* restrict a, int* restrict b, int* restrict c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Vectorizable operation */
        c[i] = a[i] + b[i] * 2;
    }
}

int main() {
    /* Initialize vector variables */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.5f, 2.5f, 3.5f, 4.5f};
    v4sf vec_d = {0.5f, 1.5f, 2.5f, 3.5f};
    
    v4si results[4];
    
    /* Call vector operations multiple times */
    for (int i = 0; i < 4; i++) {
        /* Modify inputs slightly each iteration */
        vec_a[0] += i;
        vec_b[1] += i;
        
        results[i] = vector_operations(vec_a, vec_b, vec_c, vec_d);
        
        /* Also call float operations */
        vec_c = float_vector_ops(vec_c, vec_d);
    }
    
    /* Test with OpenMP SIMD */
    int array_size = 1024;
    int array_a[array_size];
    int array_b[array_size];
    int array_c[array_size];
    
    /* Initialize arrays */
    for (int i = 0; i < array_size; i++) {
        array_a[i] = i;
        array_b[i] = array_size - i;
    }
    
    /* This should trigger SIMD vectorization */
    omp_simd_loop(array_a, array_b, array_c, array_size);
    
    /* Additional complex vector expressions */
    v8hi short_vec1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi short_vec2 = {8, 7, 6, 5, 4, 3, 2, 1};
    
    /* Mixed size operations */
    v4si from_short = __builtin_convertvector(short_vec1 + short_vec2, v4si);
    
    /* Use builtin shufflevector directly */
    v4si shuffled_mixed = __builtin_shuffle(from_short, results[0]);
    
    /* Aggregate results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sum += results[i][j];
        }
    }
    
    for (int i = 0; i < array_size; i += 8) {
        sum += array_c[i];
    }
    
    sum += shuffled_mixed[0] + shuffled_mixed[2];
    
    printf("Result: %d\n", sum);
    
    return 0;
}
