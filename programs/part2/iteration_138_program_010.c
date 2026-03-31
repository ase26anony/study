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
static void vector_operations(v4si *result1, v4sf *result2, v2di *result3) {
    /* Initialize vectors with patterns */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4sf c = {1.5f, 2.5f, 3.5f, 4.5f};
    v4sf d = {5.5f, 6.5f, 7.5f, 8.5f};
    v2di e = {9LL, 10LL};
    v2di f = {11LL, 12LL};
    
    /* Use volatile to prevent optimization */
    volatile v4si v1, v2;
    volatile v4sf v3, v4;
    volatile v2di v5, v6;
    
    /* 1. Basic vector arithmetic - may trigger built-in vectorized function creation */
    v1 = a + b;
    v2 = a * b;
    
    /* 2. Mixed type operations with conversion */
    v3 = c + d;
    v4 = __builtin_convertvector(a, v4sf) * c;
    
    /* 3. Use built-in shuffle with a mask */
    v8hi mask_source = {1, 2, 3, 4, 5, 6, 7, 8};
    int shuffle_mask[] = {7, 6, 5, 4, 3, 2, 1, 0};
    v8hi shuffled = __builtin_shuffle(mask_source, shuffle_mask);
    
    /* 4. Architecture-specific built-in (Intel intrinsic style) */
    v5 = e + f;
    
    /* 5. Complex expression mixing different vector types */
    v2df g = {1.0, 2.0};
    v2df h = {3.0, 4.0};
    v2df converted = __builtin_convertvector(e, v2df);
    v2df complex_result = g * h + converted;
    
    /* Store results */
    *result1 = v1 + v2;
    *result2 = v3 + v4;
    *result3 = v5;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&shuffled), "r"(&complex_result) : "memory");
}

/* Another function with OpenMP SIMD pragma */
__attribute__((noinline))
static float omp_simd_loop(int n, float *a, float *b, float *c) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Mixed scalar and vector-like operations */
        float temp = a[i] * b[i] + c[i];
        
        /* Use built-in functions that might trigger vectorization */
        if (temp > 0) {
            sum += __builtin_sqrtf(temp);
        } else {
            sum += __builtin_fabsf(temp);
        }
    }
    
    return sum;
}

/* Function using explicit vector built-ins */
__attribute__((target("avx2"), noinline))
static v4si use_explicit_builtins(v4si x, v4si y) {
    /* These might trigger creation of artificial function declarations */
    v4si result;
    
    /* Simulate various built-in operations */
    result = x + y;
    result = result * x;
    
    /* Use shuffle with variable mask */
    int mask[4] = {3, 2, 1, 0};
    result = __builtin_shuffle(result, mask);
    
    return result;
}

int main() {
    v4si vec_result1, vec_result4;
    v4sf vec_result2;
    v2di vec_result3;
    float scalar_result;
    
    /* Call vector operations multiple times */
    for (int i = 0; i < 10; i++) {
        vector_operations(&vec_result1, &vec_result2, &vec_result3);
        
        /* Use explicit built-ins */
        v4si x = {i, i+1, i+2, i+3};
        v4si y = {i*2, i*3, i*4, i*5};
        vec_result4 = use_explicit_builtins(x, y);
    }
    
    /* Test OpenMP SIMD loop */
    const int N = 1024;
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
        c[i] = i * 0.3f;
    }
    
    scalar_result = omp_simd_loop(N, a, b, c);
    
    /* Aggregate results to ensure all code is live */
    int final_sum = 0;
    for (int i = 0; i < 4; i++) {
        final_sum += vec_result1[i];
        final_sum += (int)vec_result2[i];
    }
    final_sum += vec_result3[0] + vec_result3[1];
    
    printf("Final sum: %d, Scalar result: %f\n", final_sum, scalar_result);
    
    return 0;
}
