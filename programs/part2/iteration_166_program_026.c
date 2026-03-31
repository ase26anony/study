/* test_cache_detection.c - Comprehensive test for Intel CPU cache descriptor detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target architectures */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void cache_sensitive_computation(int* data, int size) {
    /* Simple computation that benefits from cache awareness */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * (i % 256);
    }
    data[0] = sum;
}

/* Pattern B: Individual functions with specific target attributes */
__attribute__((target("arch=core2")))
void core2_optimized(int* data, int size) {
    /* 8KB L1 cache optimized loop */
    const int block_size = 8192 / sizeof(int);
    for (int i = 0; i < size; i += block_size) {
        int limit = (i + block_size < size) ? i + block_size : size;
        for (int j = i; j < limit; j++) {
            data[j] = data[j] * 2 + 1;
        }
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized(int* data, int size) {
    /* 32KB L1 cache optimized loop */
    const int block_size = 32768 / sizeof(int);
    for (int i = 0; i < size; i += block_size) {
        int limit = (i + block_size < size) ? i + block_size : size;
        for (int j = i; j < limit; j++) {
            data[j] = data[j] * 3 - 2;
        }
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized(int* data, int size) {
    /* 64KB L1 cache optimized loop */
    const int block_size = 65536 / sizeof(int);
    for (int i = 0; i < size; i += block_size) {
        int limit = (i + block_size < size) ? i + block_size : size;
        for (int j = i; j < limit; j++) {
            data[j] = (data[j] << 1) | (data[j] >> 31);
        }
    }
}

/* Pattern C: ifunc resolver for runtime dispatch */
static void (*resolved_computation)(int*, int);

static void (*resolve_computation(void))(int*, int) {
    /* Force CPU detection through builtins */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized;
    } else if (__builtin_cpu_supports("avx")) {
        return nehalem_optimized;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return core2_optimized;
    } else {
        return cache_sensitive_computation;
    }
}

void dynamic_computation(int* data, int size) 
    __attribute__((ifunc("resolve_computation")));

/* Pattern D: Inline assembly CPUID queries */
static void query_cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_cache_info(void) {
    query_cpuid_cache_info();
    
    /* Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
    /* Check for various Intel CPUs to trigger cache detection */
    if (__builtin_cpu_is("core2")) {
        printf("Detected Core 2 microarchitecture\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("Detected Nehalem microarchitecture\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("Detected Sandy Bridge microarchitecture\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("Detected Ivy Bridge microarchitecture\n");
    }
    if (__builtin_cpu_is("haswell")) {
        printf("Detected Haswell microarchitecture\n");
    }
    if (__builtin_cpu_is("skylake")) {
        printf("Detected Skylake microarchitecture\n");
    }
}

/* Pattern E: Cache line size sensitive prefetching */
void cache_line_optimized_loop(int* data, int size) {
    int cache_line = 64; /* Common cache line size */
    int elements_per_line = cache_line / sizeof(int);
    
    for (int i = 0; i < size; i += elements_per_line) {
        /* Prefetch next cache line */
        __builtin_prefetch(&data[i + elements_per_line], 0, 3);
        
        /* Process current cache line */
        for (int j = 0; j < elements_per_line && (i + j) < size; j++) {
            data[i + j] = data[i + j] * data[i + j];
        }
    }
}

/* Matrix multiplication with cache-aware tiling */
void cache_aware_matrix_multiply(int n, double* A, double* B, double* C) {
    /* Try different tile sizes to trigger different cache parameter usage */
    const int tile_sizes[] = {16, 32, 64, 128};
    int tile_size = 32; /* Default */
    
    /* Choose tile size based on CPU features */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx")) {
        tile_size = 64;
    } else if (__builtin_cpu_supports("sse2")) {
        tile_size = 32;
    }
    
    for (int i = 0; i < n; i += tile_size) {
        for (int j = 0; j < n; j += tile_size) {
            for (int k = 0; k < n; k += tile_size) {
                /* Mini matrix multiplication for tile */
                for (int ii = i; ii < i + tile_size && ii < n; ii++) {
                    for (int kk = k; kk < k + tile_size && kk < n; kk++) {
                        double a = A[ii * n + kk];
                        for (int jj = j; jj < j + tile_size && jj < n; jj++) {
                            C[ii * n + jj] += a * B[kk * n + jj];
                        }
                    }
                }
            }
        }
    }
}

#else
/* Non-x86 fallback implementations */
void cache_sensitive_computation(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

void dynamic_computation(int* data, int size) {
    cache_sensitive_computation(data, size);
}

void cache_line_optimized_loop(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * data[i];
    }
}

void cache_aware_matrix_multiply(int n, double* A, double* B, double* C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
}
#endif

/* Compile-time assertion for x86 */
#if defined(__i386__) || defined(__x86_64__)
_Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
#endif

int main(void) {
    const int data_size = 100000;
    const int matrix_size = 256;
    
    /* Allocate and initialize test data */
    int* data = (int*)malloc(data_size * sizeof(int));
    double* matrixA = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double* matrixB = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double* matrixC = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    
    if (!data || !matrixA || !matrixB || !matrixC) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < data_size; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrixA[i] = (double)(i % 100) / 100.0;
        matrixB[i] = (double)((i * 7) % 100) / 100.0;
        matrixC[i] = 0.0;
    }
    
    /* Execute all cache-sensitive computations */
    
    /* 1. Function multiversioning */
    cache_sensitive_computation(data, data_size);
    
    /* 2. Target-specific functions */
    core2_optimized(data, data_size);
    nehalem_optimized(data, data_size);
    sandybridge_optimized(data, data_size);
    
    /* 3. ifunc-based dynamic dispatch */
    dynamic_computation(data, data_size);
    
    /* 4. Cache line optimized loop */
    cache_line_optimized_loop(data, data_size);
    
    /* 5. Cache-aware matrix multiplication */
    cache_aware_matrix_multiply(matrix_size, matrixA, matrixB, matrixC);
    
    /* Verify results with simple checksum */
    int checksum = 0;
    for (int i = 0; i < data_size; i += 100) {
        checksum ^= data[i];
    }
    
    double matrix_checksum = 0.0;
    for (int i = 0; i < matrix_size * matrix_size; i += 101) {
        matrix_checksum += matrixC[i];
    }
    
    printf("Data checksum: %d\n", checksum);
    printf("Matrix checksum: %f\n", matrix_checksum);
    
    /* Print cache line size if available */
#if defined(_SC_LEVEL1_DCACHE_LINESIZE)
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
#endif
    
    free(data);
    free(matrixA);
    free(matrixB);
    free(matrixC);
    
    return 0;
}
