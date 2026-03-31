/* test_cache_detection.c - Test CPU cache detection via multiple compilation targets */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force noinline to ensure function calls aren't optimized away */
__attribute__((noinline)) 
int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins trigger CPUID and cache initialization */
    if (__builtin_cpu_is("intel")) {
        features |= 1;
    }
    if (__builtin_cpu_is("amd")) {
        features |= 2;
    }
    
    /* Test various CPUID feature flags */
    if (__builtin_cpu_supports("sse")) features |= 4;
    if (__builtin_cpu_supports("sse2")) features |= 8;
    if (__builtin_cpu_supports("sse3")) features |= 16;
    if (__builtin_cpu_supports("ssse3")) features |= 32;
    if (__builtin_cpu_supports("sse4.1")) features |= 64;
    if (__builtin_cpu_supports("sse4.2")) features |= 128;
    if (__builtin_cpu_supports("avx")) features |= 256;
    if (__builtin_cpu_supports("avx2")) features |= 512;
    if (__builtin_cpu_supports("avx512f")) features |= 1024;
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
double cache_sensitive_work(int size_kb) {
    /* Create working set that fits in specified cache size */
    int elements = (size_kb * 1024) / sizeof(double);
    if (elements > 1000000) elements = 1000000;
    
    volatile double *array = (volatile double*)malloc(elements * sizeof(double));
    double sum = 0.0;
    
    /* Initialize array */
    for (int i = 0; i < elements; i++) {
        array[i] = i * 0.1;
    }
    
    /* Perform computation with cache access pattern */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < elements; i += 64) { /* 64-byte stride */
            sum += array[i];
        }
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    
    /* Perform cache-sensitive work at different sizes */
    double result1 = cache_sensitive_work(8);   /* L1 size */
    double result2 = cache_sensitive_work(64);  /* L2 size */
    double result3 = cache_sensitive_work(512); /* L3 size */
    
    printf("Results: %f %f %f\n", result1, result2, result3);
    
    /* Architecture-specific code paths */
#ifdef __AVX512F__
    printf("Compiled with AVX-512 support\n");
#elif defined(__AVX2__)
    printf("Compiled with AVX2 support\n");
#elif defined(__AVX__)
    printf("Compiled with AVX support\n");
#elif defined(__SSE4_2__)
    printf("Compiled with SSE4.2 support\n");
#endif
    
    return 0;
}
