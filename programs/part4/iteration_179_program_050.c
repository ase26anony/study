/* test_cache_kernel.c - Cache detection test kernel */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force noinline to prevent optimization of CPU detection */
__attribute__((noinline)) 
int detect_cpu_features(void) {
    int result = 0;
    
    /* These builtins force CPUID calls and cache initialization */
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
    volatile double* array = (volatile double*)__builtin_alloca(size * sizeof(double));
    double sum = 0.0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = (double)i / 1000.0;
    }
    
    /* Multiple passes to stress cache */
    for (int pass = 0; pass < 100; pass++) {
        for (int i = 0; i < size; i++) {
            sum += array[i];
        }
    }
    
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", features);
    
    /* Perform computations with different working set sizes */
    double result1 = cache_sensitive_computation(1024);    /* Likely fits in L1 */
    double result2 = cache_sensitive_computation(8192);    /* Likely fits in L2 */
    double result3 = cache_sensitive_computation(65536);   /* Likely exceeds L2 */
    
    printf("Results: %f, %f, %f\n", result1, result2, result3);
    
    return (int)(result1 + result2 + result3) & 0xFF;
}
