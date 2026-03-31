/* test_cache_detect.c - Test CPU cache detection via __builtin_cpu_* */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force noinline to prevent optimization from removing CPU detection */
__attribute__((noinline)) int detect_cpu_features(void) {
    int features = 0;
    
    /* Test various CPU identification builtins */
    if (__builtin_cpu_is("intel")) {
        features |= 1;
    }
    if (__builtin_cpu_is("amd")) {
        features |= 2;
    }
    
    /* Test various instruction set support */
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
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline)) double cache_sensitive_computation(int size) {
    volatile double *array = malloc(size * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i * 0.1;
    }
    
    /* Perform computation that benefits from cache optimization */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < size; i++) {
            sum += array[i] * (i % 16);
        }
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    printf("  Intel CPU: %s\n", (features & 1) ? "yes" : "no");
    printf("  AMD CPU: %s\n", (features & 2) ? "yes" : "no");
    printf("  SSE: %s\n", (features & 4) ? "yes" : "no");
    printf("  SSE2: %s\n", (features & 8) ? "yes" : "no");
    printf("  SSE3: %s\n", (features & 16) ? "yes" : "no");
    printf("  SSSE3: %s\n", (features & 32) ? "yes" : "no");
    printf("  SSE4.1: %s\n", (features & 64) ? "yes" : "no");
    printf("  SSE4.2: %s\n", (features & 128) ? "yes" : "no");
    printf("  AVX: %s\n", (features & 256) ? "yes" : "no");
    printf("  AVX2: %s\n", (features & 512) ? "yes" : "no");
    
    /* Perform cache-sensitive computation with different sizes */
    double result1 = cache_sensitive_computation(1024);    /* Likely fits in L1 */
    double result2 = cache_sensitive_computation(8192);    /* Likely fits in L2 */
    double result3 = cache_sensitive_computation(65536);   /* Likely exceeds L2 */
    
    printf("Computation results: %.2f, %.2f, %.2f\n", result1, result2, result3);
    
    return 0;
}
