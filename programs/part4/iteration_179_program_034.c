/* test_cache_detect.c - Test CPU cache detection via __builtin_cpu_* functions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force noinline to ensure function calls aren't optimized away */
#define NOINLINE __attribute__((noinline))

/* Cache-sensitive computation */
NOINLINE static long cache_sensitive_sum(int *array, int size) {
    long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
        /* Add some data dependency to prevent over-optimization */
        array[i] = (array[i] * 13 + 7) % 100;
    }
    return sum;
}

/* Function that uses CPU detection builtins */
NOINLINE static int detect_cpu_features(void) {
    int features = 0;
    
    /* Test various CPU vendor checks */
    if (__builtin_cpu_is("intel")) {
        features |= 0x01;
    }
    if (__builtin_cpu_is("amd")) {
        features |= 0x02;
    }
    
    /* Test various instruction set support */
    if (__builtin_cpu_supports("sse")) {
        features |= 0x04;
    }
    if (__builtin_cpu_supports("sse2")) {
        features |= 0x08;
    }
    if (__builtin_cpu_supports("sse3")) {
        features |= 0x10;
    }
    if (__builtin_cpu_supports("ssse3")) {
        features |= 0x20;
    }
    if (__builtin_cpu_supports("sse4.1")) {
        features |= 0x40;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        features |= 0x80;
    }
    if (__builtin_cpu_supports("avx")) {
        features |= 0x100;
    }
    if (__builtin_cpu_supports("avx2")) {
        features |= 0x200;
    }
    
    return features;
}

/* Architecture-specific optimized functions */
#ifdef CPU_NEHALEM
NOINLINE static long nehalem_optimized_sum(int *array, int size) {
    return cache_sensitive_sum(array, size) * 2;
}
#endif

#ifdef CPU_SANDYBRIDGE
NOINLINE static long sandybridge_optimized_sum(int *array, int size) {
    long sum = cache_sensitive_sum(array, size);
    /* Different computation pattern */
    for (int i = 0; i < size; i += 4) {
        sum += array[i] * 3;
    }
    return sum;
}
#endif

#ifdef CPU_HASWELL
NOINLINE static long haswell_optimized_sum(int *array, int size) {
    long sum = 0;
    /* Different access pattern */
    for (int i = 0; i < size; i += 2) {
        sum += array[i];
        if (i + 1 < size) {
            sum += array[i + 1] * 2;
        }
    }
    return sum;
}
#endif

#ifdef CPU_SKYLAKE
NOINLINE static long skylake_optimized_sum(int *array, int size) {
    long sum = 0;
    /* Strided access pattern */
    for (int i = 0; i < size; i += 8) {
        for (int j = 0; j < 8 && i + j < size; j++) {
            sum += array[i + j] * (j + 1);
        }
    }
    return sum;
}
#endif

#ifdef CPU_ZEN
NOINLINE static long zen_optimized_sum(int *array, int size) {
    long sum = 0;
    /* Different optimization for AMD Zen */
    for (int i = size - 1; i >= 0; i--) {
        sum += array[i];
        array[i] = (array[i] + sum) % 256;
    }
    return sum;
}
#endif

int main(void) {
    const int array_size = 100000;
    int *array = malloc(array_size * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        array[i] = (i * 13 + 7) % 100;
    }
    
    /* Detect CPU features (triggers cache initialization) */
    int features = detect_cpu_features();
    
    /* Perform cache-sensitive computation */
    long result = 0;
    
    /* Call architecture-specific functions if compiled with those flags */
#ifdef CPU_NEHALEM
    result += nehalem_optimized_sum(array, array_size);
#endif
#ifdef CPU_SANDYBRIDGE
    result += sandybridge_optimized_sum(array, array_size);
#endif
#ifdef CPU_HASWELL
    result += haswell_optimized_sum(array, array_size);
#endif
#ifdef CPU_SKYLAKE
    result += skylake_optimized_sum(array, array_size);
#endif
#ifdef CPU_ZEN
    result += zen_optimized_sum(array, array_size);
#endif
    
    /* Default computation if no specific CPU defined */
    if (result == 0) {
        result = cache_sensitive_sum(array, array_size);
    }
    
    printf("CPU Features: 0x%x\n", features);
    printf("Computation result: %ld\n", result);
    
    /* Verify result isn't optimized away */
    volatile long check = result;
    
    free(array);
    return 0;
}
