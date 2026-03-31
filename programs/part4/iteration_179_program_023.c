/* test_cache_detection.c - Forces GCC driver to detect various cache configurations */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force CPUID usage through builtins */
__attribute__((noinline)) 
static int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins force CPUID calls and cache initialization */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    
    /* Force CPU vendor detection */
    if (__builtin_cpu_is("intel")) features |= 256;
    if (__builtin_cpu_is("amd")) features |= 512;
    
    return features;
}

/* Cache-sensitive computation to encourage optimization based on cache size */
__attribute__((noinline))
static double cache_sensitive_computation(int size) {
    volatile double *array = __builtin_alloca(size * sizeof(double));
    double sum = 0.0;
    
    /* Access pattern that depends on cache characteristics */
    for (int i = 0; i < size; i++) {
        array[i] = i * 0.1;
    }
    
    /* Strided access to test different cache line sizes */
    for (int i = 0; i < size; i += 8) {
        sum += array[i];
    }
    
    return sum;
}

/* Different computation patterns for different cache configurations */
#ifdef USE_L1_OPTIMIZED
__attribute__((noinline))
static void l1_optimized_kernel(void) {
    /* Small working set for L1 cache */
    char buffer[8192];  /* 8KB - matches some L1 cache sizes */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
    }
}
#endif

#ifdef USE_L2_OPTIMIZED  
__attribute__((noinline))
static void l2_optimized_kernel(void) {
    /* Larger working set for L2 cache */
    char buffer[262144];  /* 256KB - typical L2 size */
    for (int i = 0; i < sizeof(buffer); i += 128) {
        buffer[i] = i & 0xFF;
    }
}
#endif

int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    
    /* Perform computations with different working set sizes */
    double result1 = cache_sensitive_computation(1024);    /* Fits in L1 */
    double result2 = cache_sensitive_computation(32768);   /* May exceed L1 */
    double result3 = cache_sensitive_computation(262144);  /* May exceed L2 */
    
    printf("Results: %f %f %f\n", result1, result2, result3);
    
    #ifdef USE_L1_OPTIMIZED
    l1_optimized_kernel();
    #endif
    
    #ifdef USE_L2_OPTIMIZED
    l2_optimized_kernel();
    #endif
    
    return 0;
}
