/* test_kernel.c - Cache detection test kernel */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force noinline to prevent optimization of CPU detection */
#define NOINLINE __attribute__((noinline))

/* CPU detection function that uses builtins */
NOINLINE int detect_cpu_features(void) {
    int features = 0;
    
    /* Test various CPU identification builtins */
    if (__builtin_cpu_is("intel")) {
        features |= 0x01;
    }
    if (__builtin_cpu_is("amd")) {
        features |= 0x02;
    }
    
    /* Test various instruction set support */
    if (__builtin_cpu_supports("sse")) {
        features |= 0x04;
    }
    if (__builtin_cpu_supports("sse2")) {
        features |= 0x08;
    }
    if (__builtin_cpu_supports("sse3")) {
        features |= 0x10;
    }
    if (__builtin_cpu_supports("ssse3")) {
        features |= 0x20;
    }
    if (__builtin_cpu_supports("sse4.1")) {
        features |= 0x40;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        features |= 0x80;
    }
    if (__builtin_cpu_supports("avx")) {
        features |= 0x100;
    }
    if (__builtin_cpu_supports("avx2")) {
        features |= 0x200;
    }
    
    return features;
}

/* Cache-sensitive computation */
NOINLINE double cache_sensitive_computation(int size) {
    volatile double *array = (volatile double*)malloc(size * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = (double)i;
    }
    
    /* Perform computation that benefits from cache optimization */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < size; i++) {
            sum += array[i] * 0.5;
        }
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    printf("  Intel CPU: %s\n", (features & 0x01) ? "yes" : "no");
    printf("  AMD CPU: %s\n", (features & 0x02) ? "yes" : "no");
    printf("  SSE: %s\n", (features & 0x04) ? "yes" : "no");
    printf("  SSE2: %s\n", (features & 0x08) ? "yes" : "no");
    printf("  SSE3: %s\n", (features & 0x10) ? "yes" : "no");
    printf("  SSSE3: %s\n", (features & 0x20) ? "yes" : "no");
    printf("  SSE4.1: %s\n", (features & 0x40) ? "yes" : "no");
    printf("  SSE4.2: %s\n", (features & 0x80) ? "yes" : "no");
    printf("  AVX: %s\n", (features & 0x100) ? "yes" : "no");
    printf("  AVX2: %s\n", (features & 0x200) ? "yes" : "no");
    
    /* Perform cache-sensitive computation with different sizes */
    double result1 = cache_sensitive_computation(1024);    /* Likely fits in L1 */
    double result2 = cache_sensitive_computation(8192);    /* Likely fits in L2 */
    double result3 = cache_sensitive_computation(65536);   /* Likely exceeds L2 */
    
    printf("Computation results: %.2f, %.2f, %.2f\n", result1, result2, result3);
    
    return 0;
}
