/* test_cache_kernel.c - Cache detection stress test kernel */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force CPUID usage through builtins */
__attribute__((noinline))
static int detect_cpu_features(void) {
    int features = 0;
    
    /* Test various CPU identification builtins */
    if (__builtin_cpu_is("intel")) {
        features |= 1;
    }
    if (__builtin_cpu_is("amd")) {
        features |= 2;
    }
    
    /* Test various instruction set support */
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
static uint64_t cache_sensitive_loop(size_t size) {
    volatile uint64_t *buffer = malloc(size * sizeof(uint64_t));
    uint64_t sum = 0;
    
    if (!buffer) return 0;
    
    /* Initialize with pattern */
    for (size_t i = 0; i < size; i++) {
        buffer[i] = i;
    }
    
    /* Access pattern that stresses cache */
    for (size_t i = 0; i < size; i++) {
        sum += buffer[i];
        if (i % 128 == 0) {
            sum += buffer[(i * 17) % size]; /* Stride access */
        }
    }
    
    free((void*)buffer);
    return sum;
}

int main(void) {
    int cpu_features = detect_cpu_features();
    
    printf("CPU Features mask: 0x%x\n", cpu_features);
    printf("Compiled with architecture: ");
    
    /* These macros help identify compilation target */
#ifdef __x86_64__
    printf("x86_64 ");
#endif
#ifdef __i386__
    printf("i386 ");
#endif
#ifdef __SSE__
    printf("SSE ");
#endif
#ifdef __SSE2__
    printf("SSE2 ");
#endif
#ifdef __SSE3__
    printf("SSE3 ");
#endif
#ifdef __SSSE3__
    printf("SSSE3 ");
#endif
#ifdef __SSE4_1__
    printf("SSE4.1 ");
#endif
#ifdef __SSE4_2__
    printf("SSE4.2 ");
#endif
#ifdef __AVX__
    printf("AVX ");
#endif
#ifdef __AVX2__
    printf("AVX2 ");
#endif
    printf("\n");
    
    /* Perform cache-sensitive computation with different sizes */
    uint64_t results[3];
    results[0] = cache_sensitive_loop(1024);      /* Likely fits in L1 */
    results[1] = cache_sensitive_loop(32768);     /* Likely fits in L2 */
    results[2] = cache_sensitive_loop(262144);    /* Likely fits in L3 */
    
    printf("Cache test results: %lu %lu %lu\n", 
           (unsigned long)results[0],
           (unsigned long)results[1],
           (unsigned long)results[2]);
    
    return 0;
}
