/* test_cache_detect.c - Test CPU cache detection via multiple compilation targets */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force cache-sensitive computation */
#define ARRAY_SIZE 1000000
static int data[ARRAY_SIZE];

/* Use CPU builtins to trigger cache initialization */
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
    
    /* CPU vendor detection */
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

int main(void) {
    /* Initialize data */
    memset(data, 0, sizeof(data));
    
    /* Trigger CPU detection */
    int features = detect_cpu_features();
    
    /* Perform cache-sensitive computation */
    long result = cache_sensitive_sum();
    
    printf("CPU Features: 0x%x\n", features);
    printf("Computation result: %ld\n", result);
    
    return 0;
}
