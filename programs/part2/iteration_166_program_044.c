/*
 * test_cache_descriptors.c
 * 
 * This program is designed to trigger GCC driver's CPUID cache detection
 * logic for specific Intel cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.)
 * The uncovered lines are in driver-i386.cc lines 127-244.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Early CPUID query via constructor to force driver initialization */
static void __attribute__((constructor)) early_cpuid_init(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 0 - Get vendor string */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    
    /* CPUID leaf 1 - Get processor info and features */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    /* CPUID leaf 2 - Cache descriptors (triggers the switch cases) */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(2));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(4), "c"(i));
    }
}

/* Function with target attribute for Core 2 (triggers specific cache descriptors) */
__attribute__((target("arch=core2")))
void core2_optimized_function(int* data, int size) {
    for (int i = 0; i < size; i++) {
        /* Use prefetch with locality hints */
        __builtin_prefetch(&data[i + 16], 0, 3);
        data[i] = data[i] * 3 + 7;
    }
}

/* Function with target attribute for Nehalem */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_function(int* data, int size) {
    for (int i = 0; i < size; i += 8) {
        /* Process 8 elements at a time */
        __builtin_prefetch(&data[i + 32], 0, 2);
        for (int j = 0; j < 8 && (i + j) < size; j++) {
            data[i + j] = data[i + j] * 5 - 11;
        }
    }
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_function(int* data, int size) {
    /* AVX-optimized pattern */
    for (int i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 64], 0, 1);
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            data[i + j] = (data[i + j] << 2) | 0x1;
        }
    }
}

/* Function with target attribute for Ivy Bridge */
__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_function(int* data, int size) {
    /* Optimized for Ivy Bridge cache hierarchy */
    for (int i = 0; i < size; i += 32) {
        __builtin_prefetch(&data[i + 128], 0, 0);
        for (int j = 0; j < 32 && (i + j) < size; j++) {
            data[i + j] = data[i + j] ^ 0xAAAAAAAA;
        }
    }
}

/* Multi-versioned function using target_clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multiversion_cache_test(int* data, int size) {
    /* Base implementation */
    for (int i = 0; i < size; i++) {
        data[i] = data[i] + i;
    }
}

/* ifunc resolver for runtime dispatch */
static void (*resolve_cache_function(void)) (int*, int) {
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized_function;
    } else if (__builtin_cpu_supports("avx")) {
        return nehalem_optimized_function;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return ivybridge_optimized_function;
    } else {
        return core2_optimized_function;
    }
}

/* ifunc function that triggers CPU detection */
void dynamic_cache_function(int* data, int size) 
    __attribute__((ifunc("resolve_cache_function")));

/* Cache-sensitive matrix multiplication */
void cache_sensitive_matrix_multiply(int size) {
    /* Use sizes that match specific cache configurations */
    const int L1_SIZE = 8 * 1024 / sizeof(int);  /* 8KB L1 cache */
    const int L2_SIZE = 256 * 1024 / sizeof(int); /* 256KB L2 cache */
    
    int* A = malloc(size * size * sizeof(int));
    int* B = malloc(size * size * sizeof(int));
    int* C = malloc(size * size * sizeof(int));
    
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < size * size; i++) {
        A[i] = i % 100;
        B[i] = (i + 7) % 100;
        C[i] = 0;
    }
    
    /* Tiled matrix multiplication optimized for cache */
    const int TILE = 32; /* 32x32 tile fits in L1 cache */
    for (int i = 0; i < size; i += TILE) {
        for (int j = 0; j < size; j += TILE) {
            for (int k = 0; k < size; k += TILE) {
                /* Process tile */
                for (int ii = i; ii < i + TILE && ii < size; ii++) {
                    for (int kk = k; kk < k + TILE && kk < size; kk++) {
                        int a = A[ii * size + kk];
                        for (int jj = j; jj < j + TILE && jj < size; jj++) {
                            C[ii * size + jj] += a * B[kk * size + jj];
                        }
                    }
                }
            }
        }
    }
    
    /* Compute checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < size * size; i++) {
        checksum += C[i];
    }
    
    printf("Matrix checksum: %llu\n", checksum);
    
    free(A); free(B); free(C);
}

#else
/* Non-x86 fallback */
void early_cpuid_init(void) {}
void core2_optimized_function(int* data, int size) {}
void nehalem_optimized_function(int* data, int size) {}
void sandybridge_optimized_function(int* data, int size) {}
void ivybridge_optimized_function(int* data, int size) {}
void multiversion_cache_test(int* data, int size) {}
void dynamic_cache_function(int* data, int size) {}
void cache_sensitive_matrix_multiply(int size) {
    printf("Non-x86 architecture - cache test skipped\n");
}
#endif

int main(void) {
    /* Force CPU initialization in driver */
    __builtin_cpu_init();
    
    /* Extensive CPU feature checks to trigger driver detection */
    printf("CPU Feature Detection:\n");
    
#if defined(__i386__) || defined(__x86_64__)
    /* Check for various Intel CPUs to trigger different cache descriptors */
    if (__builtin_cpu_is("intel")) {
        printf("  Vendor: Intel\n");
    }
    if (__builtin_cpu_is("core2")) {
        printf("  Microarchitecture: Core 2\n");
        /* Core 2 uses cache descriptors like 0x66, 0x67, 0x68 */
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("  Microarchitecture: Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("  Microarchitecture: Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("  Microarchitecture: Ivy Bridge\n");
    }
    
    /* Check CPU features that correlate with cache configurations */
    printf("CPU Features:\n");
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("  SSSE3: %s\n", __builtin_cpu_supports("ssse3") ? "yes" : "no");
    printf("  SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("  SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("  AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    
    /* Query cache information via sysconf */
    printf("\nCache Information (via sysconf):\n");
    printf("  L1 DCache Line Size: %ld bytes\n", sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
    printf("  L1 DCache Size: %ld bytes\n", sysconf(_SC_LEVEL1_DCACHE_SIZE));
    printf("  L2 Cache Size: %ld bytes\n", sysconf(_SC_LEVEL2_CACHE_SIZE));
#endif
    
    /* Create test data with sizes that match specific cache configurations */
    const int TEST_SIZE = 8192; /* 8KB worth of integers */
    int* test_data = malloc(TEST_SIZE * sizeof(int));
    
    if (!test_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    for (int i = 0; i < TEST_SIZE; i++) {
        test_data[i] = i;
    }
    
    /* Call all target-specific functions to trigger driver detection */
    printf("\nExecuting target-specific functions:\n");
    
    core2_optimized_function(test_data, TEST_SIZE / 4);
    nehalem_optimized_function(test_data + TEST_SIZE / 4, TEST_SIZE / 4);
    sandybridge_optimized_function(test_data + TEST_SIZE / 2, TEST_SIZE / 4);
    ivybridge_optimized_function(test_data + 3 * TEST_SIZE / 4, TEST_SIZE / 4);
    
    /* Call multi-versioned function */
    multiversion_cache_test(test_data, TEST_SIZE);
    
    /* Call ifunc-based function */
    dynamic_cache_function(test_data, TEST_SIZE);
    
    /* Perform cache-sensitive computation */
    printf("\nPerforming cache-sensitive matrix multiplication:\n");
    cache_sensitive_matrix_multiply(128); /* 128x128 matrix */
    
    /* Compute final checksum */
    unsigned long long final_checksum = 0;
    for (int i = 0; i < TEST_SIZE; i++) {
        final_checksum += test_data[i];
    }
    printf("\nFinal data checksum: %llu\n", final_checksum);
    
    free(test_data);
    
    printf("\nTest completed successfully.\n");
    return 0;
}
