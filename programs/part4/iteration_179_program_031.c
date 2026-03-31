/* test_cache_detect.c - Forces GCC driver to detect various cache configurations */
#include <stdio.h>
#include <stdlib.h>
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
    
    /* Force cache-sensitive computation */
    volatile int result = 0;
    for (int i = 0; i < 1000; i++) {
        result += i * features;
    }
    
    return features;
}

/* Cache-sensitive function that should trigger optimization decisions */
__attribute__((noinline))
static void cache_sensitive_loop(int *array, int size) {
    volatile int sum = 0;
    /* Loop designed to be cache-sensitive */
    for (int i = 0; i < size; i++) {
        sum += array[i];
        array[i] = sum;
    }
}

int main(void) {
    int cpu_features = detect_cpu_features();
    
    /* Create cache-sensitive workload */
    const int array_size = 10000;
    int *array = (int*)malloc(array_size * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array */
    for (int i = 0; i < array_size; i++) {
        array[i] = i % 256;
    }
    
    /* Perform cache-sensitive computation */
    cache_sensitive_loop(array, array_size);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum += array[i];
    }
    
    printf("CPU Features: 0x%x, Checksum: %d\n", cpu_features, checksum);
    
    free(array);
    return 0;
}
