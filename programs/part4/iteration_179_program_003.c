/* test_cache_kernel.c - Cache detection test kernel */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force noinline to prevent optimization of CPU detection */
__attribute__((noinline)) 
int detect_cpu_features(void) {
    int result = 0;
    
    /* Test various CPUID-based builtins to force cache initialization */
    if (__builtin_cpu_supports("sse")) result |= 1;
    if (__builtin_cpu_supports("sse2")) result |= 2;
    if (__builtin_cpu_supports("sse3")) result |= 4;
    if (__builtin_cpu_supports("ssse3")) result |= 8;
    if (__builtin_cpu_supports("sse4.1")) result |= 16;
    if (__builtin_cpu_supports("sse4.2")) result |= 32;
    if (__builtin_cpu_supports("avx")) result |= 64;
    if (__builtin_cpu_supports("avx2")) result |= 128;
    
    /* CPU vendor detection */
    if (__builtin_cpu_is("intel")) result |= 256;
    if (__builtin_cpu_is("amd")) result |= 512;
    
    return result;
}

/* Cache-sensitive computation */
__attribute__((noinline))
double cache_sensitive_computation(int size) {
    volatile double *array = (volatile double*)malloc(size * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        array[i] = (i % 7) * 0.1;
    }
    
    /* Multiple passes to stress cache */
    for (int pass = 0; pass < 100; pass++) {
        for (int i = 0; i < size; i++) {
            sum += array[i];
        }
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    printf("Vendor: %s\n", (features & 256) ? "Intel" : 
                          (features & 512) ? "AMD" : "Unknown");
    
    /* Perform computations with different working set sizes */
    double result1 = cache_sensitive_computation(1024);    /* L1 size */
    double result2 = cache_sensitive_computation(16384);   /* L2 size */
    double result3 = cache_sensitive_computation(131072);  /* L3 size */
    
    printf("Results: %f, %f, %f\n", result1, result2, result3);
    
    return 0;
}
