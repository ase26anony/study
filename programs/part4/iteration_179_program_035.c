/* test_cache_detect.c - Cache detection test kernel */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Force noinline to prevent optimization from removing CPU detection */
#define NOINLINE __attribute__((noinline))

/* CPU detection function that forces CPUID usage */
NOINLINE static int detect_cpu_features(void) {
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
NOINLINE static uint64_t cache_sensitive_compute(int size_kb) {
    /* Create working set based on requested cache size */
    int elements = (size_kb * 1024) / sizeof(int);
    int *array = (int*)malloc(elements * sizeof(int));
    uint64_t sum = 0;
    
    if (!array) return 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < elements; i++) {
        array[i] = i % 256;
    }
    
    /* Access pattern that stresses cache */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < elements; i += 64) { /* 64-byte stride */
            sum += array[i];
        }
    }
    
    free(array);
    return sum;
}

/* Architecture-specific functions (will be conditionally compiled) */
#ifdef ARCH_NEHALEM
NOINLINE static uint64_t nehalem_specific_work(void) {
    return cache_sensitive_compute(32) + detect_cpu_features();
}
#endif

#ifdef ARCH_SANDYBRIDGE
NOINLINE static uint64_t sandybridge_specific_work(void) {
    return cache_sensitive_compute(64) + detect_cpu_features();
}
#endif

#ifdef ARCH_HASWELL
NOINLINE static uint64_t haswell_specific_work(void) {
    return cache_sensitive_compute(128) + detect_cpu_features();
}
#endif

#ifdef ARCH_SKYLAKE
NOINLINE static uint64_t skylake_specific_work(void) {
    return cache_sensitive_compute(256) + detect_cpu_features();
}
#endif

#ifdef ARCH_ZEN
NOINLINE static uint64_t zen_specific_work(void) {
    return cache_sensitive_compute(512) + detect_cpu_features();
}
#endif

int main(void) {
    uint64_t result = 0;
    
    /* Force CPU detection early */
    int features = detect_cpu_features();
    
    /* Perform architecture-specific work if compiled for it */
    #ifdef ARCH_NEHALEM
    result += nehalem_specific_work();
    #endif
    
    #ifdef ARCH_SANDYBRIDGE
    result += sandybridge_specific_work();
    #endif
    
    #ifdef ARCH_HASWELL
    result += haswell_specific_work();
    #endif
    
    #ifdef ARCH_SKYLAKE
    result += skylake_specific_work();
    #endif
    
    #ifdef ARCH_ZEN
    result += zen_specific_work();
    #endif
    
    /* Default cache-sensitive work */
    result += cache_sensitive_compute(1024);
    
    printf("Result: %lu (Features: %d)\n", result, features);
    return 0;
}
