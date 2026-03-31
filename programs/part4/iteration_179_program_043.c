/* test_cache_detect.c - Cache detection test kernel */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Force no inlining to ensure each arch-specific path is compiled */
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
    if (__builtin_cpu_supports("sse")) {
        features |= 4;
    }
    if (__builtin_cpu_supports("sse2")) {
        features |= 8;
    }
    if (__builtin_cpu_supports("sse3")) {
        features |= 16;
    }
    if (__builtin_cpu_supports("ssse3")) {
        features |= 32;
    }
    if (__builtin_cpu_supports("sse4.1")) {
        features |= 64;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        features |= 128;
    }
    if (__builtin_cpu_supports("avx")) {
        features |= 256;
    }
    if (__builtin_cpu_supports("avx2")) {
        features |= 512;
    }
    if (__builtin_cpu_supports("avx512f")) {
        features |= 1024;
    }
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
double cache_sensitive_work(int size) {
    volatile double *array = malloc(size * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        array[i] = (i % 7) * 0.1;
    }
    
    /* Multiple passes to stress cache */
    for (int pass = 0; pass < 100; pass++) {
        for (int i = 0; i < size; i++) {
            sum += array[i];
        }
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    
    /* Perform work with different sizes to potentially trigger
       different cache optimization decisions */
    double result1 = cache_sensitive_work(1024);    /* Likely fits in L1 */
    double result2 = cache_sensitive_work(16384);   /* Likely fits in L2 */
    double result3 = cache_sensitive_work(262144);  /* Likely fits in L3 */
    
    printf("Results: %f, %f, %f\n", result1, result2, result3);
    
    return 0;
}
