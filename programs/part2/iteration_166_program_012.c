/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPU cache detection
 * Specifically targets the switch-case block in driver-i386.cc lines 127-244
 * Compile with: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 * For 32-bit: gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__)

/* ============================================
   PATTERN A: Function Multiversioning with Target Attributes
   ============================================ */

/* Core2 architecture - likely triggers cache descriptors: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, etc. */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Nehalem architecture - triggers different cache descriptors */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 3;
    }
}

/* Sandy Bridge architecture */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 11;
    }
}

/* Ivy Bridge architecture */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 7;
    }
}

/* ============================================
   PATTERN B: ifunc for Runtime Dispatch
   ============================================ */

typedef void (*compute_func_t)(int*, int);

static compute_func_t resolve_compute() {
    /* These builtins force GCC driver to initialize CPU cache data structures */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse3")) {
        return compute_core2;
    }
    return compute_core2;
}

void compute_dispatch(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* ============================================
   PATTERN C: Inline Assembly with CPUID
   ============================================ */

static void cpuid_query(uint32_t leaf, uint32_t* eax, uint32_t* ebx, 
                       uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid\n\t"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

static void query_cache_descriptors() {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID Leaf 2 - Cache and TLB Descriptors */
    cpuid_query(2, &eax, &ebx, &ecx, &edx);
    
    /* CPUID Leaf 4 - Deterministic Cache Parameters */
    for (int i = 0; i < 4; i++) {
        cpuid_query(4, &eax, &ebx, &ecx, &edx);
        /* Force compiler to use results */
        asm volatile ("" : : "r"(eax), "r"(ebx), "r"(ecx), "r"(edx));
    }
}

/* ============================================
   PATTERN D: Cache-Sensitive Computations
   ============================================ */

/* Array sizes matching specific cache sizes from the switch cases */
#define SIZE_8KB    2048    /* 8KB / 4 bytes per int */
#define SIZE_16KB   4096
#define SIZE_32KB   8192
#define SIZE_64KB   16384
#define SIZE_128KB  32768
#define SIZE_256KB  65536
#define SIZE_512KB  131072
#define SIZE_1MB    262144
#define SIZE_2MB    524288
#define SIZE_4MB    1048576
#define SIZE_6MB    1572864

__attribute__((constructor))
static void init_cache_detection() {
    /* Force early CPU detection in GCC driver */
    __builtin_cpu_init();
    query_cache_descriptors();
    
    /* Check for various Intel CPUs to trigger different cache paths */
    if (__builtin_cpu_is("core2")) {
        printf("Detected Core2 - should trigger cache descriptors: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("Detected Nehalem - should trigger cache descriptors: 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("Detected Sandy Bridge - should trigger cache descriptors: 0x41-0x45, 0x78-0x7f\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("Detected Ivy Bridge - should trigger cache descriptors: 0x48, 0x49, 0x4e\n");
    }
}

/* Matrix multiplication with cache-aware tiling */
static void cache_sensitive_matrix_multiply(int size) {
    /* Use volatile to prevent optimization */
    volatile int* A = (volatile int*)malloc(size * size * sizeof(int));
    volatile int* B = (volatile int*)malloc(size * size * sizeof(int));
    volatile int* C = (volatile int*)malloc(size * size * sizeof(int));
    
    if (!A || !B || !C) {
        free((void*)A); free((void*)B); free((void*)C);
        return;
    }
    
    /* Initialize */
    for (int i = 0; i < size * size; i++) {
        A[i] = i % 256;
        B[i] = (i + 1) % 256;
        C[i] = 0;
    }
    
    /* Tiled matrix multiplication - encourages cache optimization */
    const int TILE = 32; /* Try different tile sizes: 32, 64 */
    for (int i = 0; i < size; i += TILE) {
        for (int j = 0; j < size; j += TILE) {
            for (int k = 0; k < size; k += TILE) {
                for (int ii = i; ii < i + TILE && ii < size; ii++) {
                    for (int jj = j; jj < j + TILE && jj < size; jj++) {
                        int sum = C[ii * size + jj];
                        for (int kk = k; kk < k + TILE && kk < size; kk++) {
                            sum += A[ii * size + kk] * B[kk * size + jj];
                        }
                        C[ii * size + jj] = sum;
                    }
                }
            }
        }
    }
    
    /* Use __builtin_prefetch with different locality hints */
    for (int i = 0; i < size * size; i += 64) { /* 64-byte stride for cache lines */
        __builtin_prefetch(&C[i], 0, 3); /* Read, high temporal locality */
    }
    
    free((void*)A); free((void*)B); free((void*)C);
}

/* ============================================
   Main Function
   ============================================ */

int main() {
    printf("=== GCC Driver Cache Detection Test ===\n");
    
    /* Force CPU detection via builtins */
    __builtin_cpu_init();
    
    /* Compile-time assertion for x86 features */
    #if defined(__i386__) || defined(__x86_64__)
    static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
    #endif
    
    /* Query system cache information */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 cache line size: %ld bytes\n", cache_line);
    
    /* Test with different array sizes to trigger various cache descriptor cases */
    int* data_small = (int*)malloc(SIZE_8KB * sizeof(int));
    int* data_medium = (int*)malloc(SIZE_64KB * sizeof(int));
    int* data_large = (int*)malloc(SIZE_256KB * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < SIZE_8KB; i++) data_small[i] = i;
    for (int i = 0; i < SIZE_64KB; i++) data_medium[i] = i % 1024;
    for (int i = 0; i < SIZE_256KB; i++) data_large[i] = i % 4096;
    
    /* Execute all compute variants to trigger different target attributes */
    compute_core2(data_small, SIZE_8KB);
    compute_nehalem(data_medium, SIZE_64KB);
    compute_sandybridge(data_large, SIZE_256KB);
    compute_ivybridge(data_small, SIZE_8KB);
    
    /* Use ifunc dispatch */
    compute_dispatch(data_medium, SIZE_64KB);
    
    /* Perform cache-sensitive computations */
    printf("Performing cache-sensitive matrix multiply...\n");
    cache_sensitive_matrix_multiply(128); /* 128x128 matrix */
    
    /* Verify computation with checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE_8KB; i++) checksum += data_small[i];
    for (int i = 0; i < SIZE_64KB; i++) checksum += data_medium[i];
    for (int i = 0; i < SIZE_256KB; i++) checksum += data_large[i];
    
    printf("Final checksum: %llu\n", checksum);
    printf("Test completed successfully\n");
    
    free(data_small);
    free(data_medium);
    free(data_large);
    
    return 0;
}

#else /* Non-x86 fallback */

int main() {
    printf("This test is for x86 architectures only\n");
    return 0;
}

#endif /* __i386__ || __x86_64__ */
