/* test_cache_detect.c - Forces CPUID cache detection through various paths */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force noinline to ensure function calls aren't optimized away */
#define NOINLINE __attribute__((noinline))

/* Cache-sensitive computation */
NOINLINE static uint64_t cache_sensitive_sum(int *array, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) {
        /* Access pattern that stresses cache */
        sum += array[(i * 17) % size];  /* Non-linear access */
    }
    return sum;
}

/* Function that uses CPU detection builtins */
NOINLINE static int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins trigger CPUID and cache initialization */
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

/* Architecture-specific implementations using macros */
#ifdef ARCH_NEHALEM
NOINLINE static int nehalem_specific(void) {
    return __builtin_cpu_supports("popcnt") ? 1 : 0;
}
#endif

#ifdef ARCH_SANDYBRIDGE
NOINLINE static int sandybridge_specific(void) {
    return __builtin_cpu_supports("avx") ? 2 : 0;
}
#endif

#ifdef ARCH_HASWELL
NOINLINE static int haswell_specific(void) {
    return __builtin_cpu_supports("avx2") ? 3 : 0;
}
#endif

#ifdef ARCH_SKYLAKE
NOINLINE static int skylake_specific(void) {
    return __builtin_cpu_supports("avx512f") ? 4 : 0;
}
#endif

#ifdef ARCH_ZEN
NOINLINE static int zen_specific(void) {
    return __builtin_cpu_is("amd") ? 5 : 0;
}
#endif

int main(void) {
    /* Create array that's larger than typical L1 cache */
    const int ARRAY_SIZE = 16384;  /* 64KB for ints - stresses cache */
    static int data[16384];
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i & 0xFF;
    }
    
    /* Trigger CPU detection */
    int features = detect_cpu_features();
    
    /* Call architecture-specific code if compiled */
    int arch_specific = 0;
    
#ifdef ARCH_NEHALEM
    arch_specific += nehalem_specific();
#endif
#ifdef ARCH_SANDYBRIDGE
    arch_specific += sandybridge_specific();
#endif
#ifdef ARCH_HASWELL
    arch_specific += haswell_specific();
#endif
#ifdef ARCH_SKYLAKE
    arch_specific += skylake_specific();
#endif
#ifdef ARCH_ZEN
    arch_specific += zen_specific();
#endif
    
    /* Perform cache-sensitive computation */
    uint64_t sum = cache_sensitive_sum(data, ARRAY_SIZE);
    
    /* Use results to prevent optimization */
    printf("Features: %d, Arch: %d, Sum: %lu\n", 
           features, arch_specific, (unsigned long)sum);
    
    return (int)(sum & 0x7FFFFFFF);
}
