/* test_cache_detection.c - Comprehensive test for Intel CPU cache descriptor detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Early CPUID query via constructor to force driver initialization */
static void __attribute__((constructor)) early_cpuid_init(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Query CPUID leaf 0 to get vendor string */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(0));
    
    /* Query CPUID leaf 1 for feature bits */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(1));
    
    /* Query CPUID leaf 2 (cache descriptors) - this is critical */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(2));
    
    /* Query CPUID leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid" 
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                      : "a"(4), "c"(i));
    }
}

/* Function multiversioning with different target architectures */
/* Core2 target - may trigger descriptors like 0x0a, 0x0c, 0x0d, 0x21, 0x24, etc. */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Nehalem target - may trigger different cache descriptors */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 + 11;
    }
}

/* Sandy Bridge target */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 13;
    }
}

/* Ivy Bridge target */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 + 17;
    }
}

/* Generic/default implementation */
void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

/* ifunc resolver for runtime dispatch */
static void (*resolve_compute(void))(int*, int) {
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_core2;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return compute_ivybridge;
    }
    
    return compute_default;
}

/* ifunc function that will trigger driver's CPU detection */
void compute_optimized(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Function with target clones for multiple architectures */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
int cache_sensitive_multiply(int* a, int* b, int* result, int n) {
    int sum = 0;
    
    /* Loop tiling for cache optimization */
    const int tile_size = 16; /* Small tile to fit in L1 */
    
    for (int i = 0; i < n; i += tile_size) {
        for (int j = 0; j < n; j += tile_size) {
            for (int ii = i; ii < i + tile_size && ii < n; ii++) {
                for (int jj = j; jj < j + tile_size && jj < n; jj++) {
                    result[ii * n + jj] = a[ii * n + jj] * b[jj * n + ii];
                    
                    /* Prefetch hints with different locality levels */
                    if (jj + 4 < n) {
                        __builtin_prefetch(&a[ii * n + jj + 4], 0, 0); /* L1 prefetch */
                        __builtin_prefetch(&b[(jj + 4) * n + ii], 0, 1); /* L2 prefetch */
                    }
                }
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n * n; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* Array sizes matching specific cache sizes to trigger cache detection */
#define SIZE_8KB    2048    /* 8KB / sizeof(int) for 32-bit ints */
#define SIZE_16KB   4096    /* 16KB / sizeof(int) */
#define SIZE_32KB   8192    /* 32KB / sizeof(int) */
#define SIZE_64KB   16384   /* 64KB / sizeof(int) */
#define SIZE_128KB  32768   /* 128KB / sizeof(int) */
#define SIZE_256KB  65536   /* 256KB / sizeof(int) */

/* Test different cache line sizes */
static void test_cache_line_sizes(void) {
    printf("Cache line sizes (via sysconf):\n");
    printf("L1 dcache line size: %ld\n", sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
    printf("L1 icache line size: %ld\n", sysconf(_SC_LEVEL1_ICACHE_LINESIZE));
    printf("L2 cache line size: %ld\n", sysconf(_SC_LEVEL2_CACHE_LINESIZE));
    printf("L3 cache line size: %ld\n", sysconf(_SC_LEVEL3_CACHE_LINESIZE));
}

/* Matrix multiplication optimized for different cache levels */
static int matrix_multiply_cache_aware(int size) {
    int *A = malloc(size * size * sizeof(int));
    int *B = malloc(size * size * sizeof(int));
    int *C = malloc(size * size * sizeof(int));
    
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return 0;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < size * size; i++) {
        A[i] = i % 100;
        B[i] = (i + 1) % 100;
        C[i] = 0;
    }
    
    /* Perform cache-sensitive multiplication */
    int result = cache_sensitive_multiply(A, B, C, size);
    
    /* Also test ifunc-based computation */
    compute_optimized(C, size * size);
    
    free(A); free(B); free(C);
    return result;
}

/* Test different array sizes to trigger various cache descriptor cases */
static void test_various_cache_sizes(void) {
    int *data_8kb = malloc(SIZE_8KB * sizeof(int));
    int *data_16kb = malloc(SIZE_16KB * sizeof(int));
    int *data_32kb = malloc(SIZE_32KB * sizeof(int));
    int *data_64kb = malloc(SIZE_64KB * sizeof(int));
    int *data_128kb = malloc(SIZE_128KB * sizeof(int));
    int *data_256kb = malloc(SIZE_256KB * sizeof(int));
    
    if (data_8kb) {
        for (int i = 0; i < SIZE_8KB; i++) data_8kb[i] = i;
        compute_optimized(data_8kb, SIZE_8KB);
        free(data_8kb);
    }
    
    if (data_16kb) {
        for (int i = 0; i < SIZE_16KB; i++) data_16kb[i] = i;
        compute_optimized(data_16kb, SIZE_16KB);
        free(data_16kb);
    }
    
    if (data_32kb) {
        for (int i = 0; i < SIZE_32KB; i++) data_32kb[i] = i;
        compute_optimized(data_32kb, SIZE_32KB);
        free(data_32kb);
    }
    
    if (data_64kb) {
        for (int i = 0; i < SIZE_64KB; i++) data_64kb[i] = i;
        compute_optimized(data_64kb, SIZE_64KB);
        free(data_64kb);
    }
    
    if (data_128kb) {
        for (int i = 0; i < SIZE_128KB; i++) data_128kb[i] = i;
        compute_optimized(data_128kb, SIZE_128KB);
        free(data_128kb);
    }
    
    if (data_256kb) {
        for (int i = 0; i < SIZE_256KB; i++) data_256kb[i] = i;
        compute_optimized(data_256kb, SIZE_256KB);
        free(data_256kb);
    }
}

#else
/* Non-x86 fallback implementations */
void compute_optimized(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

int cache_sensitive_multiply(int* a, int* b, int* result, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            result[i * n + j] = a[i * n + j] * b[j * n + i];
            sum += result[i * n + j];
        }
    }
    return sum;
}

static void test_cache_line_sizes(void) {
    printf("Cache detection not available on non-x86 platform\n");
}

static int matrix_multiply_cache_aware(int size) {
    int *A = malloc(size * size * sizeof(int));
    int *B = malloc(size * size * sizeof(int));
    int *C = malloc(size * size * sizeof(int));
    
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return 0;
    }
    
    for (int i = 0; i < size * size; i++) {
        A[i] = i % 100;
        B[i] = (i + 1) % 100;
        C[i] = 0;
    }
    
    int result = cache_sensitive_multiply(A, B, C, size);
    compute_optimized(C, size * size);
    
    free(A); free(B); free(C);
    return result;
}

static void test_various_cache_sizes(void) {
    printf("Cache size testing not available on non-x86 platform\n");
}
#endif

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Force CPU initialization and feature detection */
    __builtin_cpu_init();
    
    /* Check for various CPU features to trigger detection */
    printf("CPU Feature Detection:\n");
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("  SSSE3: %s\n", __builtin_cpu_supports("ssse3") ? "yes" : "no");
    printf("  SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("  SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("  AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    
    /* Check CPU type */
    printf("\nCPU Type Detection:\n");
    printf("  Intel Core 2: %s\n", __builtin_cpu_is("core2") ? "yes" : "no");
    printf("  Intel Nehalem: %s\n", __builtin_cpu_is("nehalem") ? "yes" : "no");
    printf("  Intel Sandy Bridge: %s\n", __builtin_cpu_is("sandybridge") ? "yes" : "no");
    printf("  Intel Ivy Bridge: %s\n", __builtin_cpu_is("ivybridge") ? "yes" : "no");
    printf("  Intel Haswell: %s\n", __builtin_cpu_is("haswell") ? "yes" : "no");
    
    /* Compile-time assertion for x86 */
    _Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
#endif
    
    /* Test cache line sizes */
    test_cache_line_sizes();
    
    /* Test with various matrix sizes to trigger different cache behaviors */
    printf("\nTesting matrix multiplication with different sizes:\n");
    
    int result1 = matrix_multiply_cache_aware(32);  /* Small - fits in L1 */
    printf("  32x32 matrix result: %d\n", result1);
    
    int result2 = matrix_multiply_cache_aware(64);  /* Medium - fits in L2 */
    printf("  64x64 matrix result: %d\n", result2);
    
    int result3 = matrix_multiply_cache_aware(128); /* Large - needs L3 */
    printf("  128x128 matrix result: %d\n", result3);
    
    /* Test various cache-sized arrays */
    printf("\nTesting various cache-sized arrays:\n");
    test_various_cache_sizes();
    
    /* Final checksum to verify computation */
    int final_array[100];
    for (int i = 0; i < 100; i++) final_array[i] = i;
    compute_optimized(final_array, 100);
    
    int checksum = 0;
    for (int i = 0; i < 100; i++) checksum += final_array[i];
    printf("\nFinal checksum: %d\n", checksum);
    
    return 0;
}
