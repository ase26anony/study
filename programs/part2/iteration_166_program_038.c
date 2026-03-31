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

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target attributes */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    /* Use cache-friendly access pattern */
    for (int i = 0; i < size; i += 8) {
        data[i] = i * 2;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    /* Different stride to potentially trigger different cache behavior */
    for (int i = 0; i < size; i += 16) {
        data[i] = i * 3;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    /* AVX-friendly pattern */
    for (int i = 0; i < size; i += 32) {
        data[i] = i * 4;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    /* Different access pattern */
    for (int i = 0; i < size; i += 64) {
        data[i] = i * 5;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
typedef void (*compute_func_t)(int*, int);

static compute_func_t resolve_compute() {
    /* These builtins cause driver to initialize CPU cache data */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("ssse3")) {
        return compute_core2;
    } else {
        return compute_core2;
    }
}

void compute_optimized(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Direct CPUID queries */
static void query_cpuid_cache_info() {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        eax = 4;
        ecx = i;
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_cache() {
    query_cpuid_cache_info();
    __builtin_cpu_init();
}

/* Pattern D: Cache-sensitive computation with prefetching */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=haswell")))
void cache_sensitive_compute(double* matrix, int n) {
    /* Tiled matrix processing optimized for cache */
    const int BLOCK_SIZE = 32; /* Try to match cache line sizes */
    
    for (int i = 0; i < n; i += BLOCK_SIZE) {
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            for (int ii = i; ii < i + BLOCK_SIZE && ii < n; ii++) {
                /* Prefetch with different locality hints */
                __builtin_prefetch(&matrix[(ii + 1) * n + j], 0, 3);
                for (int jj = j; jj < j + BLOCK_SIZE && jj < n; jj++) {
                    matrix[ii * n + jj] = matrix[ii * n + jj] * 1.01;
                }
            }
        }
    }
}

#else
/* Non-x86 fallback implementations */
void compute_optimized(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i;
    }
}

void cache_sensitive_compute(double* matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = matrix[i] * 1.01;
    }
}
#endif

/* Pattern B continued: Extensive use of CPU detection builtins */
static void detect_cpu_features() {
#if defined(__i386__) || defined(__x86_64__)
    /* Force driver to check many CPU types */
    if (__builtin_cpu_is("intel")) {
        /* Check various Intel microarchitectures */
        if (__builtin_cpu_is("core2")) {
            printf("Detected Core 2\n");
        }
        if (__builtin_cpu_is("nehalem")) {
            printf("Detected Nehalem\n");
        }
        if (__builtin_cpu_is("sandybridge")) {
            printf("Detected Sandy Bridge\n");
        }
        if (__builtin_cpu_is("ivybridge")) {
            printf("Detected Ivy Bridge\n");
        }
        if (__builtin_cpu_is("haswell")) {
            printf("Detected Haswell\n");
        }
        if (__builtin_cpu_is("skylake")) {
            printf("Detected Skylake\n");
        }
    }
    
    /* Check specific features that correlate with cache descriptors */
    printf("SSE2: %d\n", __builtin_cpu_supports("sse2"));
    printf("SSE3: %d\n", __builtin_cpu_supports("sse3"));
    printf("SSSE3: %d\n", __builtin_cpu_supports("ssse3"));
    printf("SSE4.1: %d\n", __builtin_cpu_supports("sse4.1"));
    printf("SSE4.2: %d\n", __builtin_cpu_supports("sse4.2"));
    printf("AVX: %d\n", __builtin_cpu_supports("avx"));
    printf("AVX2: %d\n", __builtin_cpu_supports("avx2"));
#endif
}

/* Validation: Check system cache information */
static void validate_cache_info() {
#if defined(_SC_LEVEL1_DCACHE_LINESIZE)
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
#endif
    
#if defined(_SC_LEVEL1_DCACHE_SIZE)
    long l1_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    printf("L1 cache size: %ld bytes\n", l1_size);
#endif
    
#if defined(_SC_LEVEL2_CACHE_SIZE)
    long l2_size = sysconf(_SC_LEVEL2_CACHE_SIZE);
    printf("L2 cache size: %ld bytes\n", l2_size);
#endif
}

/* Main computation that uses all patterns */
int main() {
    printf("Starting cache descriptor test...\n");
    
    /* Initialize CPU detection */
    detect_cpu_features();
    validate_cache_info();
    
    /* Create arrays sized to match specific cache sizes from uncovered lines */
    const int SIZE_8KB = 8192 / sizeof(int);      /* 0x0a: 8KB */
    const int SIZE_16KB = 16384 / sizeof(int);    /* 0x0c, 0x0d: 16KB */
    const int SIZE_32KB = 32768 / sizeof(int);    /* 0x2c: 32KB */
    const int SIZE_256KB = 262144 / sizeof(int);  /* 0x21: 256KB */
    
    int* data_small = malloc(SIZE_8KB * sizeof(int));
    int* data_medium = malloc(SIZE_16KB * sizeof(int));
    int* data_large = malloc(SIZE_32KB * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Use different compute functions to trigger various target attributes */
    compute_optimized(data_small, SIZE_8KB);
    compute_optimized(data_medium, SIZE_16KB);
    compute_optimized(data_large, SIZE_32KB);
    
    /* Create matrix for cache-sensitive computation */
    const int MATRIX_SIZE = 256; /* 256x256 = 64KB if double, good for L1/L2 */
    double* matrix = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    
    if (matrix) {
        /* Initialize matrix */
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            matrix[i] = (double)i;
        }
        
        /* Perform cache-sensitive computation */
        cache_sensitive_compute(matrix, MATRIX_SIZE);
        
        /* Compute checksum for validation */
        double checksum = 0.0;
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i += 64) {
            checksum += matrix[i];
        }
        printf("Matrix checksum: %f\n", checksum);
        
        free(matrix);
    }
    
    /* Compute final result from all arrays */
    int result = 0;
    for (int i = 0; i < SIZE_8KB; i += 32) {  /* 32-byte stride for 32-byte cache lines */
        result += data_small[i];
    }
    for (int i = 0; i < SIZE_16KB; i += 64) { /* 64-byte stride for 64-byte cache lines */
        result += data_medium[i];
    }
    
    printf("Final result: %d\n", result);
    
    /* Cleanup */
    free(data_small);
    free(data_medium);
    free(data_large);
    
    return 0;
}
