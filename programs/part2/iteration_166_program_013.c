/* test_cache_detection.c - Comprehensive test for GCC driver CPU cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target attributes */
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
    /* These builtins cause driver to initialize CPU cache data */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return compute_core2;
    }
    return compute_core2;
}

void compute_optimized(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Direct CPUID queries */
static void query_cpuid_cache_info() {
    unsigned int eax, ebx, ecx, edx;
    
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

/* Pattern D: Cache-sensitive computations with prefetching */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=skylake")))
void cache_sensitive_computation(double* matrix, int n) {
    /* Tile sizes that match various cache line sizes */
    const int tile32 = 32 / sizeof(double);  /* 32-byte cache line */
    const int tile64 = 64 / sizeof(double);  /* 64-byte cache line */
    
    for (int i = 0; i < n; i += tile64) {
        for (int j = 0; j < n; j += tile64) {
            for (int ii = i; ii < i + tile64 && ii < n; ii++) {
                /* Prefetch with different locality hints */
                __builtin_prefetch(&matrix[(ii + 1) * n + j], 0, 3);
                for (int jj = j; jj < j + tile64 && jj < n; jj++) {
                    matrix[ii * n + jj] = matrix[ii * n + jj] * 1.01;
                }
            }
        }
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_cache() {
    query_cpuid_cache_info();
    __builtin_cpu_init();
}

#else
/* Non-x86 fallback implementations */
void compute_optimized(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

void cache_sensitive_computation(double* matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = matrix[i] * 1.01;
    }
}
#endif

/* Pattern B continued: Extensive use of CPU detection builtins */
static void detect_cpu_features() {
#if defined(__i386__) || defined(__x86_64__)
    /* Force driver to check many CPU features */
    if (__builtin_cpu_is("intel")) {
        printf("CPU vendor: Intel\n");
    }
    
    if (__builtin_cpu_is("core2")) {
        printf("CPU family: Core 2\n");
    }
    
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU family: Nehalem\n");
    }
    
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU family: Sandy Bridge\n");
    }
    
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU family: Ivy Bridge\n");
    }
    
    /* Check various instruction sets */
    const char* features[] = {
        "sse", "sse2", "sse3", "ssse3", "sse4.1", "sse4.2",
        "avx", "avx2", "fma", "aes", "pclmul"
    };
    
    printf("CPU features: ");
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf("%s ", features[i]);
        }
    }
    printf("\n");
#endif
}

/* Array sizes that match specific cache sizes from the switch cases */
#define ARRAY_8KB   2048    /* 8KB / sizeof(int) */
#define ARRAY_16KB  4096    /* 16KB / sizeof(int) */
#define ARRAY_32KB  8192    /* 32KB / sizeof(int) */
#define ARRAY_64KB  16384   /* 64KB / sizeof(int) */

int main() {
    printf("Testing GCC driver cache detection logic\n");
    
    /* Force CPU feature detection */
    detect_cpu_features();
    
#if defined(__i386__) || defined(__x86_64__)
    /* Compile-time assertion for x86 */
    _Static_assert(__builtin_cpu_supports("sse2"), 
                   "SSE2 required for x86 optimization");
    
    /* Print system cache info for verification */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 cache line size: %ld bytes\n", cache_line);
#endif
    
    /* Test with different array sizes to trigger various cache models */
    int* data_small = malloc(ARRAY_8KB * sizeof(int));
    int* data_medium = malloc(ARRAY_16KB * sizeof(int));
    int* data_large = malloc(ARRAY_32KB * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_8KB; i++) data_small[i] = i;
    for (int i = 0; i < ARRAY_16KB; i++) data_medium[i] = i;
    for (int i = 0; i < ARRAY_32KB; i++) data_large[i] = i;
    
    /* Execute multiversioned functions */
    compute_optimized(data_small, ARRAY_8KB);
    compute_optimized(data_medium, ARRAY_16KB);
    compute_optimized(data_large, ARRAY_32KB);
    
    /* Cache-sensitive matrix computation */
    const int matrix_size = 256;  /* 256x256 matrix */
    double* matrix = malloc(matrix_size * matrix_size * sizeof(double));
    if (matrix) {
        for (int i = 0; i < matrix_size * matrix_size; i++) {
            matrix[i] = (double)i;
        }
        cache_sensitive_computation(matrix, matrix_size);
        
        /* Compute checksum for verification */
        double checksum = 0.0;
        for (int i = 0; i < matrix_size * matrix_size; i++) {
            checksum += matrix[i];
        }
        printf("Matrix checksum: %f\n", checksum);
        free(matrix);
    }
    
    /* Compute final result */
    int result = 0;
    for (int i = 0; i < ARRAY_8KB; i++) result ^= data_small[i];
    for (int i = 0; i < ARRAY_16KB; i++) result ^= data_medium[i];
    for (int i = 0; i < ARRAY_32KB; i++) result ^= data_large[i];
    
    printf("Final result: %d\n", result);
    
    free(data_small);
    free(data_medium);
    free(data_large);
    
    return 0;
}
