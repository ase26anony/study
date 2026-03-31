/* test_cache_detection.c - Force GCC driver to detect various cache configurations */
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
    
    /* Force cache-sensitive computation */
    volatile int result = 0;
    for (int i = 0; i < 1000; i++) {
        result += i * (features & 0xFF);
    }
    
    return result;
}

/* Cache-sensitive kernel that uses different access patterns */
__attribute__((noinline))
static void cache_sensitive_kernel(int *array, int size) {
    /* Different stride patterns to potentially trigger different cache behaviors */
    for (int i = 0; i < size; i += 8) {
        array[i] = i;
    }
    for (int i = 1; i < size; i += 16) {
        array[i] += array[i-1];
    }
    for (int i = 2; i < size; i += 32) {
        array[i] *= 2;
    }
}

int main(void) {
    printf("Cache Detection Test Program\n");
    
    /* Force CPU detection */
    int features = detect_cpu_features();
    printf("Detected CPU features mask: 0x%x\n", features);
    
    /* Create cache-sensitive workload */
    const int ARRAY_SIZE = 100000;
    int *data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (data) {
        memset(data, 0, ARRAY_SIZE * sizeof(int));
        cache_sensitive_kernel(data, ARRAY_SIZE);
        
        /* Sum to prevent optimization */
        int sum = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sum += data[i];
        }
        printf("Result sum: %d\n", sum);
        
        free(data);
    }
    
    return 0;
}
