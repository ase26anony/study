/* test_cache_detection.c - Trigger GCC driver CPU cache detection for specific Intel cache descriptors */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target architectures */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 + 11;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 13;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 + 17;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
typedef void (*compute_func_t)(int*, int);

static compute_func_t resolve_compute() {
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_core2;
    } else {
        return compute_ivybridge;
    }
}

void compute_dynamic(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Target clones for multi-architecture compilation */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=ivybridge")))
void compute_multiversion(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        data[i] = sum % 256;
    }
}

/* Pattern D: Cache-sensitive computation with prefetching */
void cache_sensitive_compute(double* matrix, int n) {
    /* Tile sizes matching common cache line sizes */
    const int tile_size = 64; /* Try to match 64-byte cache lines */
    
    for (int i = 0; i < n; i += tile_size) {
        for (int j = 0; j < n; j += tile_size) {
            for (int ii = i; ii < i + tile_size && ii < n; ii++) {
                /* Prefetch with different locality hints */
                __builtin_prefetch(&matrix[(ii + 1) * n + j], 0, 3);
                for (int jj = j; jj < j + tile_size && jj < n; jj++) {
                    matrix[ii * n + jj] = matrix[ii * n + jj] * 1.5;
                }
            }
        }
    }
}

/* Constructor to force early CPUID queries */
__attribute__((constructor))
static void init_cpu_features() {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Execute CPUID leaf 2 directly (cache descriptors) */
    unsigned int eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Also query CPUID leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 10; i++) {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
    }
}

/* Compile-time assertion for x86 features */
_Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
               "Expected 32 or 64-bit architecture");

#else
/* Dummy implementations for non-x86 */
void compute_dynamic(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

void compute_multiversion(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] + 1;
    }
}

void cache_sensitive_compute(double* matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = matrix[i] * 2.0;
    }
}
#endif

/* Main function with extensive CPU feature checks */
int main() {
#if defined(__i386__) || defined(__x86_64__)
    /* Pattern B: Extensive use of CPU builtins */
    __builtin_cpu_init();
    
    printf("CPU Feature Detection:\n");
    
    /* Check for specific Intel microarchitectures */
    if (__builtin_cpu_is("core2")) {
        printf("  Detected: Intel Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("  Detected: Intel Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("  Detected: Intel Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("  Detected: Intel Ivy Bridge\n");
    }
    
    /* Check CPU features that influence cache detection */
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("  SSSE3: %s\n", __builtin_cpu_supports("ssse3") ? "yes" : "no");
    printf("  SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("  SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    
    /* Get cache information from system */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("  L1 Cache Line Size: %ld bytes\n", cache_line);
    
    /* Pattern D: Create arrays sized to match specific cache sizes */
    const int size_8kb = 8192 / sizeof(int);      /* 8KB cache */
    const int size_16kb = 16384 / sizeof(int);    /* 16KB cache */
    const int size_32kb = 32768 / sizeof(int);    /* 32KB cache */
    const int size_64kb = 65536 / sizeof(int);    /* 64KB cache */
    
    int* data_small = (int*)malloc(size_8kb * sizeof(int));
    int* data_medium = (int*)malloc(size_16kb * sizeof(int));
    int* data_large = (int*)malloc(size_32kb * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < size_8kb; i++) data_small[i] = i % 100;
    for (int i = 0; i < size_16kb; i++) data_medium[i] = i % 100;
    for (int i = 0; i < size_32kb; i++) data_large[i] = i % 100;
    
    /* Execute all computation patterns */
    compute_core2(data_small, size_8kb);
    compute_nehalem(data_medium, size_16kb);
    compute_sandybridge(data_large, size_32kb);
    compute_ivybridge(data_small, size_8kb);
    
    compute_dynamic(data_medium, size_16kb);
    compute_multiversion(data_large, size_32kb);
    
    /* Create matrix for cache-sensitive computation */
    const int matrix_size = 256;
    double* matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (double)(i % 100) / 10.0;
    }
    
    cache_sensitive_compute(matrix, matrix_size);
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (int i = 0; i < size_8kb; i++) checksum += data_small[i];
    for (int i = 0; i < size_16kb; i++) checksum += data_medium[i];
    for (int i = 0; i < size_32kb; i++) checksum += data_large[i];
    
    printf("  Final checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(data_small);
    free(data_medium);
    free(data_large);
    free(matrix);
    
    return 0;
#else
    printf("Non-x86 architecture detected. Running dummy computation.\n");
    
    int dummy_data[100];
    for (int i = 0; i < 100; i++) dummy_data[i] = i;
    
    compute_dynamic(dummy_data, 100);
    compute_multiversion(dummy_data, 100);
    
    double dummy_matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            dummy_matrix[i][j] = i * 10.0 + j;
        }
    }
    
    cache_sensitive_compute(&dummy_matrix[0][0], 10);
    
    return 0;
#endif
}
