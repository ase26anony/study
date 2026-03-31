/* test_cache_detect.c - Cache detection test kernel */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force CPUID usage through builtins */
__attribute__((noinline)) 
static int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins force CPUID execution and cache initialization */
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
static long cache_sensitive_loop(int size) {
    volatile long *buffer = malloc(size * sizeof(long));
    long sum = 0;
    
    if (!buffer) return -1;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        buffer[i] = i;
    }
    
    /* Access pattern that stresses cache */
    for (int i = 0; i < size; i += 8) {
        sum += buffer[i];
    }
    
    free((void*)buffer);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    printf("CPU Features detected: 0x%x\n", features);
    
    /* Test with different working set sizes */
    long result1 = cache_sensitive_loop(1024);    /* Likely fits in L1 */
    long result2 = cache_sensitive_loop(32768);   /* Likely exceeds L1 */
    long result3 = cache_sensitive_loop(262144);  /* Likely exceeds L2 */
    
    printf("Results: %ld, %ld, %ld\n", result1, result2, result3);
    return 0;
}
