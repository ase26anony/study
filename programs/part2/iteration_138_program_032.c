/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -fopenmp-simd -mavx2 -c vector_test.c */

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
    
    /* 1. Basic vector arithmetic - may trigger built-in vectorized function creation */
    v4si add_result = a + b;
    v4si mul_result = a * b;
    
    /* 2. Use explicit vector built-ins */
    v4si shuffle_result = __builtin_shuffle(a, b, (v4si){0, 2, 4, 6});
    
    /* 3. Type conversion between vector types */
    v4sf convert_result = __builtin_convertvector(a, v4sf);
    
    /* 4. Mix operations with different vector types */
    v2di wide_result = __builtin_convertvector(add_result, v2di);
    v2di mixed_result = wide_result + d;
    
    /* 5. Complex expression combining multiple operations */
    v4si complex_result = (a + b) * shuffle_result - (a & b);
    
    /* Store to volatile to prevent optimization */
    result1 = add_result;
    result2 = complex_result;
    float_result = convert_result;
    double_int_result = mixed_result;
    
    /* Return a combination of results */
    return result1 + result2 + __builtin_convertvector(float_result, v4si);
}

/* Another function using OpenMP SIMD pragmas */
__attribute__((noinline))
static void omp_simd_operations(int* restrict a, int* restrict b, int* restrict c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Mixed scalar/vector-like operations that may trigger vectorization */
        a[i] = b[i] * c[i] + (b[i] >> 2);
        
        /* Additional operations to encourage complex vectorization */
        if (i % 4 == 0) {
            a[i] += c[i] * 3;
        }
    }
}

/* Function using architecture-specific built-ins when available */
#ifdef __x86_64__
__attribute__((target("avx2"), noinline))
static v4si use_arch_specific_builtins(v4si a, v4si b) {
    /* These may create artificial declarations */
    v4si result = a;
    
    /* Try to use architecture-specific built-ins if available */
    #ifdef __SSE2__
    result = result + b;
    #endif
    
    /* Complex shuffle pattern */
    v8hi short_vec = __builtin_convertvector(result, v8hi);
    v8hi shuffled = __builtin_shuffle(short_vec, short_vec, 
                                     (v8hi){7, 6, 5, 4, 3, 2, 1, 0});
    
    return result + __builtin_convertvector(shuffled, v4si);
}
#endif

int main() {
    /* Initialize vector data with various patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di vec_d = {100LL, 200LL};
    
    /* Call vector operations multiple times */
    v4si total_result = {0, 0, 0, 0};
    
    for (int i = 0; i < 10; i++) {
        vec_a[0] += i;
        v4si result = vector_operations(vec_a, vec_b, vec_c, vec_d);
        total_result = total_result + result;
    }
    
    #ifdef __x86_64__
    total_result = total_result + use_arch_specific_builtins(vec_a, vec_b);
    #endif
    
    /* OpenMP SIMD section */
    int array_size = 256;
    int array_a[256], array_b[256], array_c[256];
    
    for (int i = 0; i < array_size; i++) {
        array_b[i] = i;
        array_c[i] = array_size - i;
    }
    
    omp_simd_operations(array_a, array_b, array_c, array_size);
    
    /* Aggregate results to ensure all code is live */
    int final_sum = 0;
    for (int i = 0; i < 4; i++) {
        final_sum += total_result[i];
    }
    
    for (int i = 0; i < array_size; i++) {
        final_sum += array_a[i];
    }
    
    printf("Result: %d\n", final_sum);
    
    return 0;
}
