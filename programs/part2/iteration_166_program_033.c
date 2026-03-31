/*
 * test_target.c - Test program to trigger GCC driver CPU cache detection
 * 
 * This program uses multiple techniques to force the GCC driver to execute
 * CPUID cache descriptor parsing logic (lines 127-244 in driver-i386.cc).
 * It combines function multiversioning, ifunc resolvers, inline assembly,
 * and cache-sensitive computations to maximize coverage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__)

/* ==================== PATTERN 1: TARGET ATTRIBUTES ==================== */

/* Function with target attribute for Core2 microarchitecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int *data, int size) {
    /* Use cache-friendly access pattern */
    for (int i = 0; i < size; i += 16) {
        data[i] = i * 2;
    }
}

/* Function with target attribute for Nehalem microarchitecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int *data, int size) {
    /* Different stride to potentially trigger different cache behavior */
    for (int i = 0; i < size; i += 32) {
        data[i] = i * 3;
    }
}

/* Function with target attribute for Sandy Bridge microarchitecture */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int *data, int size) {
    /* Another access pattern */
    for (int i = 0; i < size; i += 64) {
        data[i] = i * 5;
    }
}

/* Function with target attribute for Ivy Bridge microarchitecture */
__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_compute(int *data, int size) {
    /* Yet another pattern */
    for (int i = 0; i < size; i += 128) {
        data[i] = i * 7;
    }
}

/* ==================== PATTERN 2: IFUNC RESOLVER ==================== */

/* Base implementation */
static void generic_compute(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i;
    }
}

/* Resolver function for ifunc */
static void (*resolve_compute(void)) (int *, int) {
    /* Force CPU detection by checking multiple features */
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized_compute;
    } else if (__builtin_cpu_supports("avx")) {
        return sandybridge_optimized_compute;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_compute;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return core2_optimized_compute;
    } else if (__builtin_cpu_supports("ssse3")) {
        return core2_optimized_compute;
    } else if (__builtin_cpu_supports("sse3")) {
        return core2_optimized_compute;
    }
    return generic_compute;
}

/* ifunc function that will trigger resolver at load time */
void ifunc_compute(int *data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* ==================== PATTERN 3: MULTIVERSIONING ==================== */

/* Function with multiple target clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multiversion_compute(int *data, int size) {
    /* Cache-sensitive computation */
    int sum = 0;
    for (int i = 0; i < size; i += 8) {  /* 8 * 4 bytes = 32 bytes = typical cache line */
        sum += data[i];
    }
    data[0] = sum;
}

/* ==================== PATTERN 4: INLINE ASSEMBLY CPUID ==================== */

static void cpuid_query(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile (
        "cpuid\n\t"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

/* Query CPUID leaf 2 (cache descriptors) */
static void query_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB Descriptor information */
    cpuid_query(2, &eax, &ebx, &ecx, &edx);
    
    /* The driver may sync its cache model when it sees CPUID executed */
    volatile uint32_t dummy = eax + ebx + ecx + edx;
    (void)dummy;
}

/* Query CPUID leaf 4 (deterministic cache parameters) */
static void query_cpuid_leaf4(uint32_t cache_level) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    asm volatile (
        "cpuid\n\t"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(cache_level)
    );
    
    volatile uint32_t dummy = eax + ebx + ecx + edx;
    (void)dummy;
}

/* ==================== PATTERN 5: CACHE-SENSITIVE COMPUTATIONS ==================== */

/* Array sizes matching specific cache sizes from the switch cases */
#define SIZE_8KB    2048    /* 8KB / 4 bytes per int */
#define SIZE_16KB   4096    /* 16KB / 4 bytes per int */
#define SIZE_32KB   8192    /* 32KB / 4 bytes per int */
#define SIZE_64KB   16384   /* 64KB / 4 bytes per int */
#define SIZE_128KB  32768   /* 128KB / 4 bytes per int */
#define SIZE_256KB  65536   /* 256KB / 4 bytes per int */
#define SIZE_512KB  131072  /* 512KB / 4 bytes per int */
#define SIZE_1MB    262144  /* 1MB / 4 bytes per int */
#define SIZE_2MB    524288  /* 2MB / 4 bytes per int */

/* Matrix multiplication with cache-aware tiling */
static void cache_aware_matrix_multiply(int *a, int *b, int *c, int n) {
    const int BLOCK_SIZE = 32; /* Try to match cache line sizes */
    
    for (int i = 0; i < n; i += BLOCK_SIZE) {
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            for (int k = 0; k < n; k += BLOCK_SIZE) {
                /* Mini-block multiplication */
                for (int ii = i; ii < i + BLOCK_SIZE && ii < n; ii++) {
                    for (int kk = k; kk < k + BLOCK_SIZE && kk < n; kk++) {
                        for (int jj = j; jj < j + BLOCK_SIZE && jj < n; jj++) {
                            c[ii * n + jj] += a[ii * n + kk] * b[kk * n + jj];
                        }
                    }
                }
            }
        }
    }
}

/* ==================== PATTERN 6: BUILTIN CPU CHECKS ==================== */

/* Constructor that runs before main to force early CPU detection */
__attribute__((constructor))
static void early_cpu_detection(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query CPUID leaves to force driver sync */
    query_cpuid_leaf2();
    query_cpuid_leaf4(0); /* L1 cache */
    query_cpuid_leaf4(1); /* L2 cache */
    query_cpuid_leaf4(2); /* L3 cache if present */
    
    /* Use prefetch hints with different cache line sizes */
    char buffer[1024];
    for (int i = 0; i < 1024; i += 32) {
        __builtin_prefetch(&buffer[i], 0, 3); /* Prefetch with locality 3 */
    }
    for (int i = 0; i < 1024; i += 64) {
        __builtin_prefetch(&buffer[i], 1, 2); /* Prefetch for write with locality 2 */
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    /* Force CPU detection via builtins */
    __builtin_cpu_init();
    
    /* Extensive CPU checks - each may trigger driver cache detection */
    printf("CPU Feature Checks (triggering detection):\n");
    
    /* Check for specific Intel microarchitectures */
    if (__builtin_cpu_is("intel")) {
        printf("  Vendor: Intel\n");
    }
    if (__builtin_cpu_is("core2")) {
        printf("  Microarchitecture: Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("  Microarchitecture: Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("  Microarchitecture: Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("  Microarchitecture: Ivy Bridge\n");
    }
    if (__builtin_cpu_is("haswell")) {
        printf("  Microarchitecture: Haswell\n");
    }
    if (__builtin_cpu_is("skylake")) {
        printf("  Microarchitecture: Skylake\n");
    }
    
    /* Check instruction set extensions */
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("  SSSE3: %s\n", __builtin_cpu_supports("ssse3") ? "yes" : "no");
    printf("  SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("  SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("  AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    
    /* Get system cache information (may cause driver to sync) */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("  System L1 cache line size: %ld bytes\n", cache_line);
    
    /* Allocate arrays of various cache-sized dimensions */
    int *data_small = malloc(SIZE_8KB * sizeof(int));
    int *data_medium = malloc(SIZE_64KB * sizeof(int));
    int *data_large = malloc(SIZE_256KB * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Call all versions of compute functions */
    printf("\nExecuting compute functions with different target attributes:\n");
    
    core2_optimized_compute(data_small, SIZE_8KB);
    nehalem_optimized_compute(data_medium, SIZE_64KB);
    sandybridge_optimized_compute(data_large, SIZE_256KB);
    ivybridge_optimized_compute(data_small, SIZE_8KB);
    
    /* Call ifunc function (triggers resolver) */
    printf("Calling ifunc function...\n");
    ifunc_compute(data_medium, SIZE_64KB);
    
    /* Call multiversion function */
    printf("Calling multiversion function...\n");
    multiversion_compute(data_large, SIZE_256KB);
    
    /* Perform cache-sensitive matrix multiplication */
    printf("Performing cache-aware matrix multiplication...\n");
    const int MATRIX_SIZE = 256; /* 256x256 matrix */
    int *matrix_a = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int *matrix_b = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int *matrix_c = calloc(MATRIX_SIZE * MATRIX_SIZE, sizeof(int));
    
    if (matrix_a && matrix_b && matrix_c) {
        /* Initialize matrices */
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            matrix_a[i] = i % 100;
            matrix_b[i] = (i + 1) % 100;
        }
        
        cache_aware_matrix_multiply(matrix_a, matrix_b, matrix_c, MATRIX_SIZE);
        
        /* Compute checksum for verification */
        long long checksum = 0;
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            checksum += matrix_c[i];
        }
        printf("  Matrix multiplication checksum: %lld\n", checksum);
        
        free(matrix_a);
        free(matrix_b);
        free(matrix_c);
    }
    
    /* Compute final result using all arrays */
    long long total_sum = 0;
    for (int i = 0; i < SIZE_8KB; i++) {
        total_sum += data_small[i];
    }
    for (int i = 0; i < SIZE_64KB; i++) {
        total_sum += data_medium[i];
    }
    for (int i = 0; i < SIZE_256KB; i++) {
        total_sum += data_large[i];
    }
    
    printf("\nTotal sum (verification): %lld\n", total_sum);
    
    /* Clean up */
    free(data_small);
    free(data_medium);
    free(data_large);
    
    return 0;
}

#else /* Non-x86 fallback */

int main(void) {
    printf("This test is designed for x86 (i386/x86_64) architectures only.\n");
    printf("Compile with -m32 or -m64 on x86 systems to trigger cache detection.\n");
    return 0;
}

#endif /* __i386__ || __x86_64__ */
