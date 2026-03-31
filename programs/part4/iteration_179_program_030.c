/* test_cache_detect.c - Exercise CPU cache detection logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Force cache-sensitive computation */
#define ARRAY_SIZE 1000000
static int data[ARRAY_SIZE];

/* Use CPU builtins to trigger cache initialization */
__attribute__((noinline))
static int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins force CPUID calls and cache detection */
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
static long cache_sensitive_sum(void) {
    long sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 256;
        sum += data[i];
    }
    return sum;
}

/* Different computation patterns to stress different cache levels */
__attribute__((noinline))
static long stride_access(int stride) {
    long sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i += stride) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 256;
    }
    
    /* Trigger CPU detection (forces cache initialization in driver) */
    int features = detect_cpu_features();
    
    /* Perform cache-sensitive computations */
    long sum1 = cache_sensitive_sum();
    
    /* Different access patterns */
    long sum2 = stride_access(1);   /* Sequential - good for prefetch */
    long sum3 = stride_access(16);  /* Strided - may stress cache */
    long sum4 = stride_access(64);  /* Larger stride */
    long sum5 = stride_access(256); /* May exceed L1 cache line */
    
    /* Use results to prevent optimization */
    volatile long total = sum1 + sum2 + sum3 + sum4 + sum5 + features;
    
    printf("CPU features: 0x%x\n", features);
    printf("Result: %ld\n", total);
    
    return 0;
}
