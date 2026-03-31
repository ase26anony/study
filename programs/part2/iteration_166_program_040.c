/*
 * test_cache_detection.c
 * 
 * This test aims to trigger GCC driver's CPU cache detection logic
 * for Intel CPUID cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.)
 * by using multiple techniques to force CPU feature detection.
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
        data[i] = data[i] * 3 + 1;
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
        data[i] = data[i] * 7 + 3;
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
    /* Force CPU detection through builtins */
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
        return compute_default;
    }
}

/* ifunc function declaration */
void compute_dynamic(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Inline assembly CPUID queries */
static void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (2), "c" (0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (4), "c" (i));
    }
}

/* Pattern D: Cache-sensitive computations */
#define CACHE_LINE_32 32
#define CACHE_LINE_64 64
#define L1_8KB  8192
#define L1_16KB 16384
#define L1_32KB 32768
#define L2_128KB 131072
#define L2_256KB 262144
#define L2_512KB 524288
#define L2_1MB  1048576
#define L2_2MB  2097152

/* Compile-time assertion for SSE2 */
_Static_assert(__builtin_cpu_supports("sse2"), 
               "SSE2 required for x86 target");

/* Constructor to run early CPU detection */
__attribute__((constructor))
static void init_cpu_cache(void) {
    __builtin_cpu_init();
    query_cpuid_cache_info();
    
    /* Force driver to detect various CPU features */
    if (__builtin_cpu_is("intel")) {
        /* Check for specific Intel CPUs to trigger different cache paths */
        if (__builtin_cpu_is("core2")) {
            /* Will trigger cache descriptors for Core 2 */
        }
        if (__builtin_cpu_is("nehalem")) {
            /* Will trigger cache descriptors for Nehalem */
        }
        if (__builtin_cpu_is("sandybridge")) {
            /* Will trigger cache descriptors for Sandy Bridge */
        }
        if (__builtin_cpu_is("ivybridge")) {
            /* Will trigger cache descriptors for Ivy Bridge */
        }
    }
}

/* Cache-optimized matrix multiplication */
static void cache_optimized_matmul(int n, int* A, int* B, int* C) {
    /* Tile size based on typical L1 cache */
    const int tile = 32;
    
    for (int i = 0; i < n; i += tile) {
        for (int j = 0; j < n; j += tile) {
            for (int k = 0; k < n; k += tile) {
                for (int ii = i; ii < i + tile && ii < n; ii++) {
                    for (int kk = k; kk < k + tile && kk < n; kk++) {
                        /* Prefetch with different locality hints */
                        __builtin_prefetch(&B[kk * n + j], 0, 3); /* High temporal locality */
                        __builtin_prefetch(&A[ii * n + kk], 0, 1); /* Low temporal locality */
                        
                        int a = A[ii * n + kk];
                        for (int jj = j; jj < j + tile && jj < n; jj++) {
                            C[ii * n + jj] += a * B[kk * n + jj];
                        }
                    }
                }
            }
        }
    }
}

/* Test different cache-sized arrays */
static void test_cache_sizes(void) {
    /* Test L1 cache-sized arrays */
    int* l1_8kb = malloc(L1_8KB / sizeof(int));
    int* l1_16kb = malloc(L1_16KB / sizeof(int));
    int* l1_32kb = malloc(L1_32KB / sizeof(int));
    
    /* Test L2 cache-sized arrays */
    int* l2_128kb = malloc(L2_128KB / sizeof(int));
    int* l2_256kb = malloc(L2_256KB / sizeof(int));
    int* l2_512kb = malloc(L2_512KB / sizeof(int));
    int* l2_1mb = malloc(L2_1MB / sizeof(int));
    int* l2_2mb = malloc(L2_2MB / sizeof(int));
    
    if (l1_8kb && l1_16kb && l1_32kb && 
        l2_128kb && l2_256kb && l2_512kb && l2_1mb && l2_2mb) {
        
        /* Initialize arrays */
        for (int i = 0; i < L1_8KB / sizeof(int); i++) l1_8kb[i] = i;
        for (int i = 0; i < L1_16KB / sizeof(int); i++) l1_16kb[i] = i;
        for (int i = 0; i < L1_32KB / sizeof(int); i++) l1_32kb[i] = i;
        
        /* Process with different target functions */
        compute_core2(l1_8kb, L1_8KB / sizeof(int));
        compute_nehalem(l1_16kb, L1_16KB / sizeof(int));
        compute_sandybridge(l1_32kb, L1_32KB / sizeof(int));
        compute_ivybridge(l2_128kb, L2_128KB / sizeof(int));
        
        /* Use ifunc version */
        compute_dynamic(l2_256kb, L2_256KB / sizeof(int));
    }
    
    free(l1_8kb);
    free(l1_16kb);
    free(l1_32kb);
    free(l2_128kb);
    free(l2_256kb);
    free(l2_512kb);
    free(l2_1mb);
    free(l2_2mb);
}

#else
/* Non-x86 fallback */
static void query_cpuid_cache_info(void) {}
static void test_cache_sizes(void) {}
static void compute_dynamic(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = data[i] * 2;
}
#endif

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Print cache information if available */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("Detected cache line size: %ld bytes\n", cache_line);
    
    /* Query CPUID directly */
    query_cpuid_cache_info();
    
    /* Test with __builtin_cpu_is checks */
    if (__builtin_cpu_is("intel")) {
        printf("Intel CPU detected\n");
        
        if (__builtin_cpu_is("core2")) {
            printf("Core 2 microarchitecture\n");
        }
        if (__builtin_cpu_is("nehalem")) {
            printf("Nehalem microarchitecture\n");
        }
        if (__builtin_cpu_is("sandybridge")) {
            printf("Sandy Bridge microarchitecture\n");
        }
        if (__builtin_cpu_is("ivybridge")) {
            printf("Ivy Bridge microarchitecture\n");
        }
    }
    
    /* Test cache-sensitive computations */
    test_cache_sizes();
    
    /* Perform cache-optimized matrix multiplication */
    const int n = 128;
    int* A = malloc(n * n * sizeof(int));
    int* B = malloc(n * n * sizeof(int));
    int* C = calloc(n * n, sizeof(int));
    
    if (A && B && C) {
        for (int i = 0; i < n * n; i++) {
            A[i] = i % 100;
            B[i] = (i + 1) % 100;
        }
        
        cache_optimized_matmul(n, A, B, C);
        
        /* Compute checksum */
        long long checksum = 0;
        for (int i = 0; i < n * n; i++) {
            checksum += C[i];
        }
        printf("Matrix multiplication checksum: %lld\n", checksum);
    }
    
    free(A);
    free(B);
    free(C);
    
    printf("Test completed successfully\n");
#else
    printf("Non-x86 architecture - cache detection test skipped\n");
#endif
    
    return 0;
}
