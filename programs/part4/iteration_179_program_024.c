/* test_cache_detection.c - Force GCC driver to detect various cache configurations */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force cache-sensitive computation */
#define ARRAY_SIZE 1000000
static int data[ARRAY_SIZE];

/* Use CPU detection builtins to trigger cache initialization */
__attribute__((noinline))
static int detect_cpu_features(void) {
    int features = 0;
    
    /* These calls force CPUID execution and cache detection */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    
    /* CPU vendor detection also triggers cache initialization */
    if (__builtin_cpu_is("intel")) features |= 256;
    if (__builtin_cpu_is("amd")) features |= 512;
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
static long cache_sensitive_loop(void) {
    long sum = 0;
    volatile int *volatile ptr = data;
    
    /* Access pattern designed to stress cache detection */
    for (int i = 0; i < ARRAY_SIZE; i += 64) {  /* 64-byte stride */
        sum += ptr[i];
    }
    
    /* Another pattern with different stride */
    for (int i = 0; i < ARRAY_SIZE; i += 128) {
        sum += ptr[i] * 2;
    }
    
    return sum;
}

/* Function that should be optimized differently based on cache */
__attribute__((noinline))
static void matrix_multiply(int size, int *a, int *b, int *c) {
    /* Simple matrix multiplication - cache sensitive */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            for (int k = 0; k < size; k++) {
                sum += a[i * size + k] * b[k * size + j];
            }
            c[i * size + j] = sum;
        }
    }
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
    }
    
    /* Force CPU and cache detection */
    int features = detect_cpu_features();
    
    /* Perform cache-sensitive computation */
    long result = cache_sensitive_loop();
    
    /* Small matrix multiplication for additional cache pressure */
    int small_mat[16][16];
    matrix_multiply(16, (int *)small_mat, (int *)small_mat, (int *)small_mat);
    
    printf("CPU features: %d\n", features);
    printf("Computation result: %ld\n", result);
    
    return 0;
}
