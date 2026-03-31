/* test_cache_kernel.c - Cache detection test kernel */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force CPUID usage through builtins */
__attribute__((noinline)) 
static int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins force CPUID calls and cache initialization */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    
    /* CPU vendor detection also triggers cache init */
    if (__builtin_cpu_is("intel")) features |= 256;
    if (__builtin_cpu_is("amd")) features |= 512;
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
static double cache_sensitive_loop(size_t size) {
    volatile double *array = malloc(size * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize array */
    for (size_t i = 0; i < size; i++) {
        array[i] = (double)i / 1000.0;
    }
    
    /* Multiple passes to stress cache */
    for (int pass = 0; pass < 10; pass++) {
        for (size_t i = 0; i < size; i++) {
            sum += array[i];
        }
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    
    /* Test with different array sizes to potentially trigger
       different cache optimization decisions */
    double result1 = cache_sensitive_loop(1024);    /* Likely fits in L1 */
    double result2 = cache_sensitive_loop(8192);    /* Likely fits in L2 */
    double result3 = cache_sensitive_loop(65536);   /* Likely exceeds L2 */
    
    printf("Results: %.2f, %.2f, %.2f\n", result1, result2, result3);
    
    return 0;
}
