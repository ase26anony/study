/* test_cache_kernel.c - Cache detection stress test */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force noinline to prevent optimization from removing CPU detection */
__attribute__((noinline)) 
int detect_cpu_features(void) {
    int result = 0;
    
    /* Test various CPU identification builtins */
    if (__builtin_cpu_is("intel")) {
        result |= 1;
    }
    if (__builtin_cpu_is("amd")) {
        result |= 2;
    }
    
    /* Test various instruction set support */
    if (__builtin_cpu_supports("sse")) result |= 4;
    if (__builtin_cpu_supports("sse2")) result |= 8;
    if (__builtin_cpu_supports("sse3")) result |= 16;
    if (__builtin_cpu_supports("ssse3")) result |= 32;
    if (__builtin_cpu_supports("sse4.1")) result |= 64;
    if (__builtin_cpu_supports("sse4.2")) result |= 128;
    if (__builtin_cpu_supports("avx")) result |= 256;
    if (__builtin_cpu_supports("avx2")) result |= 512;
    if (__builtin_cpu_supports("avx512f")) result |= 1024;
    
    return result;
}

/* Cache-sensitive computation */
__attribute__((noinline))
double cache_sensitive_work(int size) {
    volatile double *array = malloc(size * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = (i % 100) * 0.01;
    }
    
    /* Perform computation that benefits from cache optimization */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < size; i++) {
            sum += array[i] * iter;
        }
    }
    
    free((void*)array);
    return sum;
}

int main(int argc, char **argv) {
    int cpu_features = detect_cpu_features();
    
    printf("CPU Features detected: 0x%x\n", cpu_features);
    
    /* Choose array size based on compilation target hints */
    int array_size = 1024; /* Default */
    
#ifdef __TUNE_CORE2__
    array_size = 8192;
#elif defined(__TUNE_NEHALEM__)
    array_size = 16384;
#elif defined(__TUNE_SANDYBRIDGE__)
    array_size = 32768;
#elif defined(__TUNE_HASWELL__)
    array_size = 65536;
#elif defined(__TUNE_SKYLAKE__)
    array_size = 131072;
#elif defined(__TUNE_ZEN__)
    array_size = 262144;
#endif
    
    double result = cache_sensitive_work(array_size);
    printf("Computation result: %f (array size: %d)\n", result, array_size);
    
    return 0;
}
