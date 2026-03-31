/* test_cache_detect.c - Test CPU cache detection via multiple compilation targets */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force cache-sensitive computation */
#define ARRAY_SIZE 1000000

/* Use CPU builtins to trigger CPUID/cache initialization */
__attribute__((noinline)) 
static int detect_cpu_features(void) {
    int features = 0;
    
    /* These calls force CPUID and cache detection */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    
    /* CPU vendor detection also triggers cache detection */
    if (__builtin_cpu_is("intel")) features |= 256;
    if (__builtin_cpu_is("amd")) features |= 512;
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
static double cache_sensitive_computation(void) {
    volatile double *array = malloc(ARRAY_SIZE * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i % 7) * 0.1;
    }
    
    /* Access pattern that stresses cache */
    for (int i = 0; i < ARRAY_SIZE; i += 64) {  /* 64-byte cache line stride */
        sum += array[i];
    }
    
    /* Another pattern with different stride */
    for (int i = 0; i < ARRAY_SIZE; i += 128) {
        sum += array[i] * 0.5;
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    double result = cache_sensitive_computation();
    
    printf("CPU Features: 0x%x\n", features);
    printf("Computation result: %f\n", result);
    
    /* Use result to prevent optimization */
    if (result > 1000.0) {
        printf("Large result detected\n");
    }
    
    return 0;
}
