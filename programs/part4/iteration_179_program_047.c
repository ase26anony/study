/* Test kernel with CPU detection and cache-sensitive operations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force noinline to ensure function calls aren't optimized away */
#define NOINLINE __attribute__((noinline))

/* CPU detection function that uses builtins to trigger CPUID */
NOINLINE static int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins will trigger CPUID and cache initialization */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    
    /* CPU vendor detection */
    if (__builtin_cpu_is("intel")) features |= 256;
    if (__builtin_cpu_is("amd")) features |= 512;
    
    return features;
}

/* Cache-sensitive computation - size tuned for different cache levels */
NOINLINE static double cache_sensitive_computation(int size_kb) {
    volatile double sum = 0.0;
    int elements = (size_kb * 1024) / sizeof(double);
    double *array = (double*)malloc(elements * sizeof(double));
    
    if (!array) return 0.0;
    
    /* Initialize array */
    for (int i = 0; i < elements; i++) {
        array[i] = (double)i;
    }
    
    /* Perform computation that should benefit from cache optimization */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < elements; i++) {
            sum += array[i] * 0.5;
        }
    }
    
    free(array);
    return sum;
}

/* Main function with architecture-specific conditional compilation */
int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    
    /* Perform computations sensitive to different cache sizes */
    double result1 = cache_sensitive_computation(8);   /* L1 cache size */
    double result2 = cache_sensitive_computation(64);  /* L2 cache size */
    double result3 = cache_sensitive_computation(512); /* L3 cache size */
    
    printf("Results: %.2f, %.2f, %.2f\n", result1, result2, result3);
    
    /* Use features to control which path is taken */
    if (features & 256) { /* Intel CPU */
        printf("Intel CPU detected\n");
    } else if (features & 512) { /* AMD CPU */
        printf("AMD CPU detected\n");
    }
    
    return 0;
}
