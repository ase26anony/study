/* test_cache_detect.c - CPU cache detection test harness */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force cache-sensitive computation */
#define ARRAY_SIZE 1000000
static int data[ARRAY_SIZE];

/* Noinline to prevent optimization from removing CPU detection */
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
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
long cache_sensitive_computation(void) {
    long sum = 0;
    volatile int *volatile_data = data; /* Prevent optimization */
    
    /* Access pattern designed to exercise different cache levels */
    for (int i = 0; i < ARRAY_SIZE; i += 64) { /* 64-byte stride */
        sum += volatile_data[i];
    }
    
    /* Random access to defeat prefetching */
    for (int i = 0; i < 10000; i++) {
        int idx = (i * 997) % ARRAY_SIZE; /* Pseudo-random index */
        sum += volatile_data[idx];
    }
    
    return sum;
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 256;
    }
    
    /* Trigger CPU detection */
    int features = detect_cpu_features();
    
    /* Perform cache-sensitive computation */
    long result = cache_sensitive_computation();
    
    /* Use result to prevent dead code elimination */
    printf("CPU Features: 0x%x, Computation result: %ld\n", features, result);
    
    return 0;
}
