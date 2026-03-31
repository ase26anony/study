/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdalign.h>

/* Function with target clones - forces generation of multiple versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
__attribute__((visibility("hidden")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) {
    /* Alignment hints for better vectorization */
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop 1: Multiple vectorizable built-ins in sequence */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float temp = __builtin_sqrtf(fabsf(src[i]));
        temp = __builtin_sinf(temp);
        temp = __builtin_expf(temp * 0.1f);
        temp = __builtin_powf(temp, 1.5f);
        dst[i] = temp;
        sum += temp;
    }
    
    return sum;
}

/* Another function targeting specific ISA extensions */
__attribute__((target("avx2,fma")))
__attribute__((visibility("default")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n) {
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    /* Loop with double-precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double temp = __builtin_sqrt(fabs(src[i]));
        temp = __builtin_sin(temp);
        temp = __builtin_exp(temp * 0.1);
        temp = __builtin_pow(temp, 1.5);
        dst[i] = temp;
        sum += temp;
    }
    
    return sum;
}

/* Function with mixed precision operations */
__attribute__((target("sse4.2")))
static float compute_mixed(float* restrict dst, const float* restrict src, int n) {
    dst = __builtin_assume_aligned(dst, 16);
    src = __builtin_assume_aligned(src, 16);
    
    float sum = 0.0f;
    
    /* Mix of math built-ins and type conversions */
    for (int i = 0; i < n; i++) {
        float temp = src[i];
        
        /* Use ldexp which may have vectorized versions */
        int exp;
        temp = frexpf(temp, &exp);
        temp = __builtin_ldexpf(temp, exp / 2);
        
        /* More vectorizable built-ins */
        temp = __builtin_sinf(__builtin_logf(fabsf(temp) + 1.0f));
        temp = __builtin_cosf(temp);
        
        dst[i] = temp;
        sum += temp;
    }
    
    return sum;
}

/* Main test function with multiple vectorizable patterns */
__attribute__((target_clones("avx2", "avx512f", "default")))
float test_vector_builtins(int size) {
    /* Use aligned storage for better vectorization */
    alignas(32) float src[1024];
    alignas(32) float dst[1024];
    alignas(32) double src_double[512];
    alignas(32) double dst_double[512];
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < 1024; i++) {
        src[i] = sinf(i * 0.01f);
    }
    for (int i = 0; i < 512; i++) {
        src_double[i] = sin(i * 0.01);
    }
    
    float sum1 = 0.0f, sum2 = 0.0f;
    double sum3 = 0.0;
    
    /* Call all vectorized functions */
    sum1 = compute_vector(dst, src, 1024);
    sum2 = compute_mixed(dst, src, 1024);
    sum3 = compute_vector_double(dst_double, src_double, 512);
    
    /* Additional loop with reduction to ensure vectorization */
    float checksum = 0.0f;
    for (int i = 0; i < 1024; i++) {
        /* Use multiple built-ins in reduction */
        checksum += __builtin_sqrtf(fabsf(dst[i])) + 
                   __builtin_sinf(dst[i] * 0.5f);
    }
    
    return sum1 + sum2 + checksum + (float)sum3;
}

int main() {
    printf("Testing vectorized built-in functions...\n");
    
    /* Call the multiversioned function */
    float result = test_vector_builtins(1024);
    
    printf("Result: %f\n", result);
    printf("Test completed.\n");
    
    /* Additional test with compile-time known size for better vectorization */
    {
        const int N = 1024;
        alignas(32) float a[N], b[N];
        
        for (int i = 0; i < N; i++) {
            a[i] = i * 0.001f;
        }
        
        /* Simple vectorizable pattern */
        for (int i = 0; i < N; i++) {
            b[i] = __builtin_expf(__builtin_sqrtf(a[i]));
        }
        
        /* Compute checksum */
        float sum = 0.0f;
        for (int i = 0; i < N; i++) {
            sum += b[i];
        }
        printf("Checksum: %f\n", sum);
    }
    
    return 0;
}
