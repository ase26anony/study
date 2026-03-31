/* test_cache.c - Cache detection test kernel */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force cache-sensitive computation */
#define ARRAY_SIZE 1000000
static int data[ARRAY_SIZE];

/* __attribute__((noinline)) to prevent optimization */
__attribute__((noinline)) 
static int detect_cpu_features(void) {
    int result = 0;
    
    /* These builtins trigger CPUID and cache initialization */
    if (__builtin_cpu_is("intel")) {
        result |= 1;
        if (__builtin_cpu_supports("sse4.2")) result |= 2;
        if (__builtin_cpu_supports("avx")) result |= 4;
        if (__builtin_cpu_supports("avx2")) result |= 8;
    }
    
    if (__builtin_cpu_is("amd")) {
        result |= 16;
        if (__builtin_cpu_supports("sse4a")) result |= 32;
        if (__builtin_cpu_supports("avx")) result |= 64;
    }
    
    /* Additional CPUID checks that might affect cache detection */
    if (__builtin_cpu_supports("popcnt")) result |= 128;
    if (__builtin_cpu_supports("bmi")) result |= 256;
    if (__builtin_cpu_supports("bmi2")) result |= 512;
    
    return result;
}

/* Cache-sensitive computation */
__attribute__((noinline))
static long cache_sensitive_loop(void) {
    volatile long sum = 0;
    
    /* Access pattern designed to exercise different cache levels */
    for (int i = 0; i < ARRAY_SIZE; i += 64) {  /* 64-byte stride */
        sum += data[i];
    }
    
    /* Another pattern with different stride */
    for (int i = 0; i < ARRAY_SIZE; i += 128) {
        sum += data[i] * 2;
    }
    
    return sum;
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 256;
    }
    
    /* Trigger CPU feature detection (and thus cache detection) */
    int cpu_features = detect_cpu_features();
    
    /* Perform cache-sensitive computation */
    long result = cache_sensitive_loop();
    
    /* Use result to prevent dead code elimination */
    printf("CPU features: 0x%x, Result: %ld\n", cpu_features, result);
    
    return 0;
}
