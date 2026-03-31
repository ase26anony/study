/* test_cache_detect.c - Cache detection stress test */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Force cache-sensitive computation */
#define ARRAY_SIZE 1000000
static int data[ARRAY_SIZE];

/* Noinline to prevent optimization of CPU detection */
__attribute__((noinline)) 
static int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins trigger CPUID and cache initialization */
    if (__builtin_cpu_is("intel")) {
        features |= 1;
        if (__builtin_cpu_supports("sse4.2")) features |= 2;
        if (__builtin_cpu_supports("avx")) features |= 4;
        if (__builtin_cpu_supports("avx2")) features |= 8;
    }
    
    if (__builtin_cpu_is("amd")) {
        features |= 16;
        if (__builtin_cpu_supports("sse4a")) features |= 32;
        if (__builtin_cpu_supports("avx")) features |= 64;
    }
    
    /* Check various cache-related features */
    if (__builtin_cpu_supports("clflushopt")) features |= 128;
    if (__builtin_cpu_supports("clwb")) features |= 256;
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
static long cache_sensitive_sum(void) {
    long sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 256;
        sum += data[i];
    }
    return sum;
}

int main(void) {
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 256;
    }
    
    /* Trigger CPU detection */
    int features = detect_cpu_features();
    
    /* Perform cache-sensitive computation */
    long result = cache_sensitive_sum();
    
    printf("CPU features: 0x%x, Result: %ld\n", features, result);
    return 0;
}
