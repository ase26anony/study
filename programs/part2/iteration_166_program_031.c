/*
 * test_cache_detection.c
 * 
 * This program is designed to trigger GCC driver's CPU cache detection logic
 * by using various CPU feature queries, function multiversioning, and inline
 * assembly that forces CPUID execution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* ========== PATTERN 1: Function Multiversioning with Target Attributes ========== */

/* Function 1: Core2 microarchitecture - may trigger cache descriptors like 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Function 2: Nehalem microarchitecture - may trigger cache descriptors like 0x78, 0x79, 0x7a */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 3;
    }
}

/* Function 3: Sandy Bridge microarchitecture - may trigger cache descriptors like 0x0a, 0x0c, 0x0d */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 11;
    }
}

/* Function 4: Ivy Bridge microarchitecture - may trigger various L2 cache descriptors */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 5;
    }
}

/* ========== PATTERN 2: ifunc resolver for runtime dispatch ========== */

/* Base implementation */
static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

/* Resolver function - forces CPU feature detection */
static void (*resolve_compute(void))(int*, int) {
    /* These builtins cause GCC to detect CPU features */
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

/* ifunc function that will be resolved at runtime */
void compute_ifunc(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* ========== PATTERN 3: Target clones for multiple architectures ========== */

/* Function with multiple target clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void compute_multiversion(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 13 + 17;
    }
}

/* ========== PATTERN 4: Inline assembly with CPUID ========== */

/* Execute CPUID leaf 2 (cache descriptors) */
static void cpuid_leaf2(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - cache information */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Access results to prevent optimization */
    volatile unsigned int dummy = eax + ebx + ecx + edx;
    (void)dummy;
}

/* Execute CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_leaf4(int cache_level) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 4 - deterministic cache parameters */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(cache_level)
    );
    
    /* Access results to prevent optimization */
    volatile unsigned int dummy = eax + ebx + ecx + edx;
    (void)dummy;
}

/* ========== PATTERN 5: Cache-sensitive computations ========== */

/* Array sizes matching specific cache sizes from uncovered lines */
#define SIZE_8KB    2048    /* 8KB / 4 bytes per int */
#define SIZE_16KB   4096    /* 16KB / 4 bytes per int */
#define SIZE_32KB   8192    /* 32KB / 4 bytes per int */
#define SIZE_64KB   16384   /* 64KB / 4 bytes per int */
#define SIZE_128KB  32768   /* 128KB / 4 bytes per int */
#define SIZE_256KB  65536   /* 256KB / 4 bytes per int */
#define SIZE_512KB  131072  /* 512KB / 4 bytes per int */
#define SIZE_1MB    262144  /* 1MB / 4 bytes per int */
#define SIZE_2MB    524288  /* 2MB / 4 bytes per int */

/* Prefetch hints with different locality levels */
static void cache_sensitive_computation(int* data, int size) {
    /* Use prefetch with explicit cache line hints */
    for (int i = 0; i < size; i += 16) { /* 16 ints = 64 bytes (common cache line) */
        __builtin_prefetch(&data[i + 32], 0, 0); /* PREFETCHT0 - all cache levels */
        __builtin_prefetch(&data[i + 64], 0, 1); /* PREFETCHT1 - L2 cache */
        __builtin_prefetch(&data[i + 96], 0, 2); /* PREFETCHT2 - L2 cache, non-temporal */
        __builtin_prefetch(&data[i + 128], 0, 3); /* PREFETCHNTA - non-temporal */
        
        data[i] = data[i] * 3 + data[i + 8] * 7;
    }
}

/* Matrix multiplication with cache-aware tiling */
static void matrix_multiply_cache_aware(int size) {
    /* Use stack arrays of various cache-sized dimensions */
    int tile_size = 32; /* Common optimization for 32KB L1 cache */
    
    /* Declare arrays that might trigger cache size detection */
    int A[32][32], B[32][32], C[32][32];
    
    for (int i = 0; i < tile_size; i++) {
        for (int j = 0; j < tile_size; j++) {
            A[i][j] = i + j;
            B[i][j] = i - j;
            C[i][j] = 0;
        }
    }
    
    /* Tiled matrix multiplication */
    for (int i = 0; i < tile_size; i++) {
        for (int k = 0; k < tile_size; k++) {
            for (int j = 0; j < tile_size; j++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < tile_size; i++) {
        for (int j = 0; j < tile_size; j++) {
            checksum += C[i][j];
        }
    }
    
    volatile int dummy = checksum;
    (void)dummy;
}

/* ========== PATTERN 6: Constructor for early CPU detection ========== */

/* Run CPUID queries before main() */
__attribute__((constructor))
static void early_cpu_detection(void) {
    printf("Early CPU detection running...\n");
    
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Query CPUID leaf 2 (cache descriptors) */
    cpuid_leaf2();
    
    /* Query CPUID leaf 4 for cache levels 1 and 2 */
    cpuid_leaf4(0); /* L1 cache */
    cpuid_leaf4(1); /* L2 cache */
    
    /* Check various CPU features to trigger detection */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    (void)has_sse2; (void)has_sse3; (void)has_ssse3;
    (void)has_sse4_1; (void)has_sse4_2; (void)has_avx; (void)has_avx2;
}

#else
/* Non-x86 fallback implementations */
void compute_ifunc(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

void compute_multiversion(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3;
    }
}

static void early_cpu_detection(void) {
    printf("Non-x86 CPU - skipping CPU-specific detection\n");
}
#endif

/* ========== Main function with extensive CPU feature checks ========== */

int main(void) {
    printf("Starting cache detection test program\n");
    
#if defined(__i386__) || defined(__x86_64__)
    /* PATTERN B: Extensive use of CPU builtins */
    __builtin_cpu_init();
    
    /* Check for specific Intel CPUs - these cause driver to initialize cache data */
    if (__builtin_cpu_is("intel")) {
        printf("CPU vendor: Intel\n");
        
        /* Check various Intel microarchitectures */
        if (__builtin_cpu_is("core2")) {
            printf("Microarchitecture: Core 2\n");
        }
        if (__builtin_cpu_is("nehalem")) {
            printf("Microarchitecture: Nehalem\n");
        }
        if (__builtin_cpu_is("sandybridge")) {
            printf("Microarchitecture: Sandy Bridge\n");
        }
        if (__builtin_cpu_is("ivybridge")) {
            printf("Microarchitecture: Ivy Bridge\n");
        }
        if (__builtin_cpu_is("haswell")) {
            printf("Microarchitecture: Haswell\n");
        }
        if (__builtin_cpu_is("skylake")) {
            printf("Microarchitecture: Skylake\n");
        }
    }
    
    /* Check CPU features comprehensively */
    const char* features[] = {
        "mmx", "sse", "sse2", "sse3", "ssse3", 
        "sse4.1", "sse4.2", "avx", "avx2", "fma"
    };
    
    printf("CPU features: ");
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf("%s ", features[i]);
        }
    }
    printf("\n");
    
    /* Get cache line size via sysconf */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
    
    /* Execute CPUID inline assembly */
    cpuid_leaf2();
    cpuid_leaf4(0);
    cpuid_leaf4(1);
    
    /* Compile-time assertion for SSE2 (common for x86-64) */
    _Static_assert(sizeof(void*) != 8 || __builtin_cpu_supports("sse2"), 
                   "SSE2 required for 64-bit x86");
#endif
    
    /* Allocate arrays of various cache-sized dimensions */
    int* data_small = malloc(SIZE_8KB * sizeof(int));
    int* data_medium = malloc(SIZE_64KB * sizeof(int));
    int* data_large = malloc(SIZE_256KB * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < SIZE_8KB; i++) data_small[i] = i % 100;
    for (int i = 0; i < SIZE_64KB; i++) data_medium[i] = i % 100;
    for (int i = 0; i < SIZE_256KB; i++) data_large[i] = i % 100;
    
    /* Call all versions of compute functions */
#if defined(__i386__) || defined(__x86_64__)
    compute_core2(data_small, SIZE_8KB);
    compute_nehalem(data_medium, SIZE_64KB);
    compute_sandybridge(data_large, SIZE_256KB);
    compute_ivybridge(data_small, SIZE_8KB);
#endif
    
    /* Call ifunc version */
    compute_ifunc(data_medium, SIZE_64KB);
    
    /* Call multiversion function */
    compute_multiversion(data_large, SIZE_256KB);
    
    /* Perform cache-sensitive computations */
#if defined(__i386__) || defined(__x86_64__)
    cache_sensitive_computation(data_small, SIZE_8KB);
    cache_sensitive_computation(data_medium, SIZE_64KB);
    cache_sensitive_computation(data_large, SIZE_256KB);
    
    matrix_multiply_cache_aware(32);
#endif
    
    /* Compute final checksum for validation */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE_8KB; i++) checksum += data_small[i];
    for (int i = 0; i < SIZE_64KB; i++) checksum += data_medium[i];
    for (int i = 0; i < SIZE_256KB; i++) checksum += data_large[i];
    
    printf("Final checksum: %llu\n", checksum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(data_small);
    free(data_medium);
    free(data_large);
    
    return 0;
}
