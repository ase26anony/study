/* test_cache_detection.c - Exercise CPU cache detection through various compilation scenarios */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force cache-sensitive computation */
#define ARRAY_SIZE 1000000

/* Use CPU builtins to trigger CPUID/cache initialization */
__attribute__((noinline)) 
int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins force CPUID calls which may trigger cache detection */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    
    /* CPU model detection also uses CPUID */
    if (__builtin_cpu_is("intel")) features |= 256;
    if (__builtin_cpu_is("amd")) features |= 512;
    if (__builtin_cpu_is("core2")) features |= 1024;
    if (__builtin_cpu_is("nehalem")) features |= 2048;
    if (__builtin_cpu_is("sandybridge")) features |= 4096;
    if (__builtin_cpu_is("haswell")) features |= 8192;
    if (__builtin_cpu_is("skylake")) features |= 16384;
    if (__builtin_cpu_is("znver1")) features |= 32768;
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
double cache_sensitive_computation(void) {
    volatile double* array = (volatile double*)malloc(ARRAY_SIZE * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (double)i / 1000.0;
    }
    
    /* Perform computation that should be cache-sensitive */
    for (int iter = 0; iter < 10; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sum += array[i] * 1.0001;
        }
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    double result = cache_sensitive_computation();
    
    printf("CPU Features detected: 0x%08x\n", features);
    printf("Computation result: %f\n", result);
    
    /* Use result to prevent optimization */
    if (result > 1000000000.0) {
        printf("Result is large\n");
    }
    
    return 0;
}
