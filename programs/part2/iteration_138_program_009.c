/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -fopenmp-simd -mavx2 -c vector_test.c */

#include <stdio.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with AVX2 target attribute to ensure vector built-ins are considered */
__attribute__((target("avx2"), noinline))
static v4si vector_operations(v4si a, v4si b, v4sf c, v4sf d) {
    /* Use volatile to prevent optimization of intermediate results */
    volatile v4si v1, v2;
    volatile v4sf v3, v4;
    volatile v2di v5;
    volatile v8hi v6;
    
    /* Basic vector arithmetic - may trigger built-in vectorized function creation */
    v1 = a + b;
    v2 = a * b;
    
    /* Mixed type operations */
    v3 = c + d;
    v4 = c * d;
    
    /* Use explicit built-in shuffle */
    v5 = __builtin_shuffle((v2di)v1, (v2di)v2, (v2di){1, 0});
    
    /* Type conversion using built-in */
    v6 = __builtin_convertvector(v1, v8hi);
    
    /* Complex expression mixing different operations */
    v4si result = v1 + v2 + (v4si)__builtin_shuffle((v2di)v1, (v2di)v2, (v2di){0, 1});
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(v6));
    
    return result;
}

/* Another function to trigger OpenMP SIMD vectorization */
__attribute__((noinline))
static float omp_simd_loop(float* arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Mixed scalar and implicit vector operations */
        sum += arr[i] * 2.5f + arr[i] * arr[i];
    }
    
    return sum;
}

/* Function using architecture-specific built-ins directly */
#ifdef __x86_64__
__attribute__((target("avx2"), noinline))
static v4si use_arch_builtins(v4si a, v4si b) {
    /* These may trigger creation of artificial declarations */
    v4si result;
    
    /* Use SSE/AVX built-ins if available */
    #ifdef __SSE2__
    result = __builtin_ia32_paddd128(a, b);
    #else
    result = a + b;
    #endif
    
    /* Shuffle operation */
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result;
}
#endif

int main() {
    /* Initialize vector variables with patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_d = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Call vector operations multiple times */
    v4si result1 = vector_operations(vec_a, vec_b, vec_c, vec_d);
    v4si result2 = vector_operations(vec_b, vec_a, vec_d, vec_c);
    
    #ifdef __x86_64__
    v4si result3 = use_arch_builtins(vec_a, vec_b);
    #else
    v4si result3 = vec_a + vec_b;
    #endif
    
    /* Create array for OpenMP SIMD test */
    float arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = (float)i / 10.0f;
    }
    
    float sum = omp_simd_loop(arr, 100);
    
    /* Aggregate results to ensure all code is live */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += result1[i] + result2[i] + result3[i];
    }
    
    printf("Vector total: %d, OMP SIMD sum: %f\n", total, sum);
    
    /* Additional complex vector expression that might trigger internal function creation */
    v2di double_vec = {10LL, 20LL};
    v2di shuffled = __builtin_shuffle(double_vec, double_vec, (v2di){1, 0});
    
    /* Type conversion between different vector types */
    v4si converted = __builtin_convertvector((v4sf){1.5f, 2.5f, 3.5f, 4.5f}, v4si);
    
    /* Mixed-size vector operations */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi short_result = short_vec + (v8hi){8, 7, 6, 5, 4, 3, 2, 1};
    
    printf("Shuffled: %lld %lld, Converted: %d %d %d %d\n", 
           shuffled[0], shuffled[1], 
           converted[0], converted[1], converted[2], converted[3]);
    
    return 0;
}
