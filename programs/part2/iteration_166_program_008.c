/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC driver's CPU cache detection logic
 * for specific Intel cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 * It uses multiple techniques to force the driver to query CPUID and parse
 * cache descriptors during compilation.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Multiple functions with different target attributes */
/* Each target attribute may cause driver to detect different cache descriptors */

/* Function for Core2 architecture - may trigger descriptors like 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = array[i] * 3 + 7;
    }
}

/* Function for Nehalem architecture - may trigger descriptors like 0x0a, 0x0c, 0x0d */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int *array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 8) {
        /* Use prefetch to hint at cache usage */
        __builtin_prefetch(&array[i + 32], 0, 3);
        sum += array[i];
    }
}

/* Function for Sandy Bridge architecture */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int *array, int size) {
    /* Matrix-style computation that benefits from cache optimization */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 64; j++) {
            array[i] = (array[i] * j) >> 2;
        }
    }
}

/* Function for Ivy Bridge architecture */
__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_compute(int *array, int size) {
    /* Different access pattern */
    for (int i = size - 1; i >= 0; i--) {
        array[i] = array[i] ^ 0x5A5A5A5A;
    }
}

/* Pattern B: Function with multiple target clones */
/* This forces driver to detect CPU features for each clone */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=ivybridge")))
int multiarch_sum(int *array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

/* Pattern C: Direct CPUID queries */
static void cpuid_query_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Query CPUID leaf 2 - Cache descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Query CPUID leaf 4 - Deterministic cache parameters */
    uint32_t cache_index = 0;
    do {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(cache_index)
        );
        cache_index++;
    } while ((eax & 0x1F) != 0); /* Continue until cache type = 0 */
}

/* Pattern D: Cache-sensitive computations with specific sizes */
/* Arrays sized to match specific cache line sizes and cache sizes */

/* 8KB array - matches L1 cache size for some descriptors (0x0a) */
#define ARRAY_8KB (8192 / sizeof(int))
static int array_8kb[ARRAY_8KB];

/* 16KB array - matches L1 cache size for descriptors (0x0c, 0x0d) */
#define ARRAY_16KB (16384 / sizeof(int))
static int array_16kb[ARRAY_16KB];

/* 32KB array - matches L1 cache size for descriptor 0x2c */
#define ARRAY_32KB (32768 / sizeof(int))
static int array_32kb[ARRAY_32KB];

/* 64KB array - matches some L2 cache sizes */
#define ARRAY_64KB (65536 / sizeof(int))
static int array_64kb[ARRAY_64KB];

/* Constructor that runs before main - forces early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    /* Initialize CPU detection builtins */
    __builtin_cpu_init();
    
    /* Query cache info directly */
    cpuid_query_cache_info();
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < ARRAY_8KB; i++) array_8kb[i] = i;
    for (int i = 0; i < ARRAY_16KB; i++) array_16kb[i] = i * 2;
    for (int i = 0; i < ARRAY_32KB; i++) array_32kb[i] = i * 3;
    for (int i = 0; i < ARRAY_64KB; i++) array_64kb[i] = i * 5;
}

/* Resolver for ifunc pattern */
static void (*resolve_compute_func(void))(int*, int) {
    /* Check CPU features to select implementation */
    if (__builtin_cpu_supports("avx2")) {
        return ivybridge_optimized_compute;
    } else if (__builtin_cpu_supports("avx")) {
        return sandybridge_optimized_compute;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_compute;
    } else {
        return core2_optimized_compute;
    }
}

/* ifunc for runtime dispatch - forces driver CPU detection */
void optimized_compute(int *array, int size) 
    __attribute__((ifunc("resolve_compute_func")));

#else
/* Non-x86 fallback implementations */
void core2_optimized_compute(int *array, int size) {
    for (int i = 0; i < size; i++) array[i] = array[i] * 2;
}

void nehalem_optimized_compute(int *array, int size) {
    for (int i = 0; i < size; i++) array[i] = array[i] + 1;
}

void sandybridge_optimized_compute(int *array, int size) {
    for (int i = 0; i < size; i++) array[i] = array[i] - 1;
}

void ivybridge_optimized_compute(int *array, int size) {
    for (int i = 0; i < size; i++) array[i] = ~array[i];
}

int multiarch_sum(int *array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) sum += array[i];
    return sum;
}

void optimized_compute(int *array, int size) {
    core2_optimized_compute(array, size);
}

static void init_cpu_detection(void) {
    /* Nothing to do on non-x86 */
}

#define ARRAY_8KB 2048
#define ARRAY_16KB 4096
#define ARRAY_32KB 8192
#define ARRAY_64KB 16384
static int array_8kb[ARRAY_8KB];
static int array_16kb[ARRAY_16KB];
static int array_32kb[ARRAY_32KB];
static int array_64kb[ARRAY_64KB];

#endif

/* Main function with extensive CPU feature checks */
int main(void) {
    /* Initialize arrays if constructor didn't run */
    init_cpu_detection();
    
#if defined(__i386__) || defined(__x86_64__)
    /* Pattern B: Extensive use of CPU detection builtins */
    /* Each check may cause driver to initialize cache data structures */
    
    printf("CPU Feature Detection:\n");
    
    if (__builtin_cpu_is("intel")) {
        printf("  Vendor: Intel\n");
    }
    
    if (__builtin_cpu_is("core2")) {
        printf("  Microarchitecture: Core 2\n");
        /* May trigger cache descriptors: 0x66, 0x67, 0x68 */
    }
    
    if (__builtin_cpu_is("nehalem")) {
        printf("  Microarchitecture: Nehalem\n");
        /* May trigger cache descriptors: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, etc. */
    }
    
    if (__builtin_cpu_is("sandybridge")) {
        printf("  Microarchitecture: Sandy Bridge\n");
    }
    
    if (__builtin_cpu_is("ivybridge")) {
        printf("  Microarchitecture: Ivy Bridge\n");
    }
    
    /* Check specific features that correlate with cache descriptors */
    printf("CPU Features:\n");
    if (__builtin_cpu_supports("sse2")) {
        printf("  SSE2: Yes\n");
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        printf("  SSE4.2: Yes\n");
    }
    
    if (__builtin_cpu_supports("avx")) {
        printf("  AVX: Yes\n");
    }
    
    if (__builtin_cpu_supports("avx2")) {
        printf("  AVX2: Yes\n");
    }
    
    /* Get cache line size via system call - validates detection */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 cache line size: %ld bytes\n", cache_line);
    
    /* Compile-time assertion for x86 */
    #if defined(__i386__) || defined(__x86_64__)
    _Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
    #endif
#endif

    /* Perform cache-sensitive computations */
    printf("\nPerforming cache-sensitive computations:\n");
    
    /* Use different array sizes to potentially trigger different cache descriptors */
    printf("1. Processing 8KB array (may trigger 0x0a descriptor)...\n");
    optimized_compute(array_8kb, ARRAY_8KB);
    int sum1 = multiarch_sum(array_8kb, ARRAY_8KB);
    
    printf("2. Processing 16KB array (may trigger 0x0c, 0x0d descriptors)...\n");
    core2_optimized_compute(array_16kb, ARRAY_16KB);
    int sum2 = multiarch_sum(array_16kb, ARRAY_16KB);
    
    printf("3. Processing 32KB array (may trigger 0x2c descriptor)...\n");
    nehalem_optimized_compute(array_32kb, ARRAY_32KB);
    int sum3 = multiarch_sum(array_32kb, ARRAY_32KB);
    
    printf("4. Processing 64KB array (may trigger L2 cache descriptors)...\n");
    sandybridge_optimized_compute(array_64kb, ARRAY_64KB);
    int sum4 = multiarch_sum(array_64kb, ARRAY_64KB);
    
    /* Matrix multiplication - cache-optimized computation */
    printf("\nPerforming matrix multiplication (cache-optimized):\n");
    #define MATRIX_SIZE 128
    int matrix_a[MATRIX_SIZE][MATRIX_SIZE];
    int matrix_b[MATRIX_SIZE][MATRIX_SIZE];
    int matrix_c[MATRIX_SIZE][MATRIX_SIZE] = {0};
    
    /* Initialize matrices */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = i + j;
            matrix_b[i][j] = i - j;
        }
    }
    
    /* Cache-optimized matrix multiplication with tiling */
    /* Tile size chosen based on common cache line sizes */
    const int TILE_SIZE = 32; /* Matches 32-byte cache lines */
    for (int i = 0; i < MATRIX_SIZE; i += TILE_SIZE) {
        for (int j = 0; j < MATRIX_SIZE; j += TILE_SIZE) {
            for (int k = 0; k < MATRIX_SIZE; k += TILE_SIZE) {
                for (int ii = i; ii < i + TILE_SIZE && ii < MATRIX_SIZE; ii++) {
                    for (int jj = j; jj < j + TILE_SIZE && jj < MATRIX_SIZE; jj++) {
                        for (int kk = k; kk < k + TILE_SIZE && kk < MATRIX_SIZE; kk++) {
                            matrix_c[ii][jj] += matrix_a[ii][kk] * matrix_b[kk][jj];
                        }
                    }
                }
            }
        }
    }
    
    /* Calculate checksum for verification */
    int final_checksum = sum1 + sum2 + sum3 + sum4;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            final_checksum += matrix_c[i][j];
        }
    }
    
    printf("\nFinal checksum: %d\n", final_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
