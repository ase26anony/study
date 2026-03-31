/*
 * test_cache_detection.c
 * Comprehensive test to trigger Intel CPU cache descriptor parsing
 * in GCC driver (driver-i386.cc lines 127-244)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
static void (*compute_func)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
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
    } else if (__builtin_cpu_supports("sse4.1")) {
        return compute_core2;
    }
    
    return compute_default;
}

void compute_optimized(int* data, int size)
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Direct CPUID queries */
static void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
    }
}

/* Pattern D: Cache-sensitive computations with prefetching */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge")))
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
                    matrix[ii * n + jj] = matrix[ii * n + jj] * 1.5;
                }
            }
        }
    }
}

/* Constructor to force early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    __builtin_cpu_init();
    query_cpuid_cache_info();
    
    /* Force driver to check various CPU features */
    if (__builtin_cpu_is("core2")) {
        printf("CPU: Core2 detected\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU: Nehalem detected\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU: Sandy Bridge detected\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU: Ivy Bridge detected\n");
    }
}

/* Array sizes that match specific cache sizes from the switch cases */
#define SIZE_8KB    (8 * 1024 / sizeof(int))      /* 0x0a */
#define SIZE_16KB   (16 * 1024 / sizeof(int))     /* 0x0c, 0x0d */
#define SIZE_24KB   (24 * 1024 / sizeof(int))     /* 0x0e */
#define SIZE_32KB   (32 * 1024 / sizeof(int))     /* 0x2c, 0x68 */
#define SIZE_128KB  (128 * 1024 / sizeof(int))    /* 0x39, 0x3b, 0x41, 0x79 */
#define SIZE_256KB  (256 * 1024 / sizeof(int))    /* 0x21, 0x3c, 0x42, 0x7a */
#define SIZE_512KB  (512 * 1024 / sizeof(int))    /* 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
#define SIZE_1MB    (1024 * 1024 / sizeof(int))   /* 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
#define SIZE_2MB    (2048 * 1024 / sizeof(int))   /* 0x45, 0x7d, 0x85 */
#define SIZE_3MB    (3072 * 1024 / sizeof(int))   /* 0x48 */
#define SIZE_4MB    (4096 * 1024 / sizeof(int))   /* 0x49 */
#define SIZE_6MB    (6144 * 1024 / sizeof(int))   /* 0x4e */

#else
/* Non-x86 fallback */
void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}
#endif

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Runtime validation of cache detection */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("Detected cache line size: %ld bytes\n", cache_line);
    
    /* Compile-time assertion for x86 features */
    #ifdef __SSE2__
    printf("SSE2 supported at compile time\n");
    #endif
    
    /* Test with arrays of various cache sizes */
    int* test_data = malloc(SIZE_1MB * sizeof(int));
    if (!test_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE_1MB; i++) {
        test_data[i] = i % 256;
    }
    
    /* Execute all computation patterns to trigger different code paths */
    compute_core2(test_data, SIZE_16KB);
    compute_nehalem(test_data, SIZE_32KB);
    compute_sandybridge(test_data, SIZE_128KB);
    compute_ivybridge(test_data, SIZE_256KB);
    
    /* Use ifunc-resolved function */
    compute_optimized(test_data, SIZE_512KB);
    
    /* Cache-sensitive matrix computation */
    int matrix_size = 256;
    double* matrix = malloc(matrix_size * matrix_size * sizeof(double));
    if (matrix) {
        for (int i = 0; i < matrix_size * matrix_size; i++) {
            matrix[i] = (double)i / 1000.0;
        }
        cache_sensitive_computation(matrix, matrix_size);
        free(matrix);
    }
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE_1MB; i++) {
        checksum += test_data[i];
    }
    printf("Computed checksum: %llu\n", checksum);
    
    free(test_data);
    
    /* Additional CPU feature checks */
    __builtin_cpu_init();
    printf("CPU features: ");
    if (__builtin_cpu_supports("sse")) printf("SSE ");
    if (__builtin_cpu_supports("sse2")) printf("SSE2 ");
    if (__builtin_cpu_supports("sse3")) printf("SSE3 ");
    if (__builtin_cpu_supports("ssse3")) printf("SSSE3 ");
    if (__builtin_cpu_supports("sse4.1")) printf("SSE4.1 ");
    if (__builtin_cpu_supports("sse4.2")) printf("SSE4.2 ");
    if (__builtin_cpu_supports("avx")) printf("AVX ");
    if (__builtin_cpu_supports("avx2")) printf("AVX2 ");
    printf("\n");
    
    return 0;
#else
    printf("Non-x86 architecture - using fallback implementation\n");
    int test_data[100];
    for (int i = 0; i < 100; i++) {
        test_data[i] = i;
    }
    compute_default(test_data, 100);
    return 0;
#endif
}
