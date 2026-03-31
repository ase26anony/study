/*
 * test_target.c - Test program to trigger GCC driver CPU cache detection
 * Compile with: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 * For 32-bit: gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with target attributes */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 2;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 11;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 5;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

static void* resolve_compute(void) {
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("ssse3")) {
        return compute_core2;
    }
    
    return compute_default;
}

__attribute__((ifunc("resolve_compute")))
void compute_dispatch(int* data, int size);

/* Pattern C: Inline assembly to force CPUID execution */
static void cpuid_query(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(2));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(4), "c"(0));
    
    /* CPUID leaf 1 - Feature bits */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(1));
}

/* Pattern D: Cache-sensitive computation with prefetching */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=haswell")))
void cache_sensitive_compute(double* matrix, int n) {
    /* Tile sizes that match common cache line sizes */
    const int tile_size = 64; /* 64 bytes = typical cache line */
    
    for (int i = 0; i < n; i += tile_size / sizeof(double)) {
        for (int j = 0; j < n; j += tile_size / sizeof(double)) {
            int imax = (i + tile_size / sizeof(double)) < n ? 
                       (i + tile_size / sizeof(double)) : n;
            int jmax = (j + tile_size / sizeof(double)) < n ? 
                       (j + tile_size / sizeof(double)) : n;
            
            for (int ii = i; ii < imax; ii++) {
                /* Prefetch with different locality hints */
                __builtin_prefetch(&matrix[(ii + 1) * n + j], 0, 0); /* PREFETCHT0 */
                __builtin_prefetch(&matrix[(ii + 2) * n + j], 0, 1); /* PREFETCHT1 */
                __builtin_prefetch(&matrix[(ii + 3) * n + j], 0, 2); /* PREFETCHT2 */
                
                for (int jj = j; jj < jmax; jj++) {
                    matrix[ii * n + jj] = matrix[ii * n + jj] * 1.5 + 2.0;
                }
            }
        }
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_features(void) {
    printf("Initializing CPU features...\n");
    __builtin_cpu_init();
    cpuid_query();
}

/* Runtime validation of cache parameters */
static void validate_cache(void) {
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
#endif
    
    /* Compile-time assertion for x86 features */
#if defined(__i386__) || defined(__x86_64__)
    /* This forces the compiler to consider SSE2 support */
    if (!__builtin_cpu_supports("sse2")) {
        printf("Warning: SSE2 not supported\n");
    }
#endif
}

#else
/* Non-x86 fallback implementations */
void compute_dispatch(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

void cache_sensitive_compute(double* matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = matrix[i] * 1.5 + 2.0;
    }
}

static void validate_cache(void) {
    printf("Non-x86 architecture - cache validation skipped\n");
}
#endif

/* Main test program */
int main(void) {
    printf("Starting cache detection test...\n");
    
    /* Pattern B: Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
#if defined(__i386__) || defined(__x86_64__)
    if (__builtin_cpu_is("core2")) {
        printf("CPU detected as Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU detected as Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU detected as Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU detected as Ivy Bridge\n");
    }
    
    /* Check for specific features that correlate with cache descriptors */
    if (__builtin_cpu_supports("sse4.2")) {
        printf("SSE4.2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
    }
#endif
    
    /* Validate cache parameters */
    validate_cache();
    
    /* Test 1: Array computation with different cache sizes */
    const int small_size = 2048;  /* 8KB array (fits in L1) */
    const int medium_size = 32768; /* 128KB array (fits in L2) */
    const int large_size = 262144; /* 1MB array (fits in L3) */
    
    int* data_small = malloc(small_size * sizeof(int));
    int* data_medium = malloc(medium_size * sizeof(int));
    int* data_large = malloc(large_size * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < small_size; i++) data_small[i] = i % 100;
    for (int i = 0; i < medium_size; i++) data_medium[i] = i % 100;
    for (int i = 0; i < large_size; i++) data_large[i] = i % 100;
    
    /* Execute computations with different cache footprints */
    compute_dispatch(data_small, small_size);
    compute_dispatch(data_medium, medium_size);
    compute_dispatch(data_large, large_size);
    
    /* Test 2: Matrix computation with cache tiling */
    const int matrix_size = 512;
    double* matrix = malloc(matrix_size * matrix_size * sizeof(double));
    
    if (matrix) {
        for (int i = 0; i < matrix_size * matrix_size; i++) {
            matrix[i] = (double)(i % 100) / 10.0;
        }
        
        cache_sensitive_compute(matrix, matrix_size);
        
        /* Compute checksum for verification */
        double checksum = 0.0;
        for (int i = 0; i < matrix_size * matrix_size; i++) {
            checksum += matrix[i];
        }
        printf("Matrix checksum: %f\n", checksum);
        
        free(matrix);
    }
    
    /* Compute final checksum */
    int final_sum = 0;
    for (int i = 0; i < small_size; i++) final_sum += data_small[i];
    for (int i = 0; i < medium_size; i++) final_sum += data_medium[i];
    for (int i = 0; i < large_size; i++) final_sum += data_large[i];
    
    printf("Final checksum: %d\n", final_sum);
    
    /* Cleanup */
    free(data_small);
    free(data_medium);
    free(data_large);
    
    printf("Test completed successfully\n");
    return 0;
}
