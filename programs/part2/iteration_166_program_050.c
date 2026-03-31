/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPU cache detection logic
 * for various Intel cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.)
 * found in driver-i386.cc lines 127-244.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cpuid.h>

/* Compile-time validation for x86 */
#if defined(__i386__) || defined(__x86_64__)
#define IS_X86 1
#else
#define IS_X86 0
#endif

/* Runtime cache information structure */
struct cache_info {
    int line_size;
    int l1_size;
    int l2_size;
    int l1_assoc;
    int l2_assoc;
};

/* Global cache info populated by constructor */
static struct cache_info g_cache_info = {0};

/* ============================================
   PATTERN A: Function Multiversioning with Target Attributes
   ============================================ */

/* Base implementation */
__attribute__((target("default")))
void matrix_multiply_default(int n, double* A, double* B, double* C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

/* Core2 target - may trigger cache descriptors: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, etc. */
__attribute__((target("arch=core2")))
void matrix_multiply_core2(int n, double* A, double* B, double* C) {
    /* Tile for L1 cache (Core2 typically has 32KB L1) */
    const int tile = 32;
    for (int i = 0; i < n; i += tile) {
        for (int j = 0; j < n; j += tile) {
            for (int k = 0; k < n; k += tile) {
                for (int ii = i; ii < i + tile && ii < n; ii++) {
                    for (int jj = j; jj < j + tile && jj < n; jj++) {
                        double sum = C[ii * n + jj];
                        for (int kk = k; kk < k + tile && kk < n; kk++) {
                            sum += A[ii * n + kk] * B[kk * n + jj];
                        }
                        C[ii * n + jj] = sum;
                    }
                }
            }
        }
    }
}

/* Nehalem target - may trigger cache descriptors: 0x2c, 0x3a-0x3e, 0x41-0x45 */
__attribute__((target("arch=nehalem")))
void matrix_multiply_nehalem(int n, double* A, double* B, double* C) {
    /* Different tiling strategy for Nehalem */
    const int tile = 64;  /* Larger tile for Nehalem's cache */
    for (int i = 0; i < n; i += tile) {
        for (int j = 0; j < n; j += tile) {
            for (int k = 0; k < n; k += tile) {
                int i_end = (i + tile < n) ? i + tile : n;
                int j_end = (j + tile < n) ? j + tile : n;
                int k_end = (k + tile < n) ? k + tile : n;
                
                for (int ii = i; ii < i_end; ii++) {
                    for (int jj = j; jj < j_end; jj++) {
                        double sum = C[ii * n + jj];
                        for (int kk = k; kk < k_end; kk++) {
                            sum += A[ii * n + kk] * B[kk * n + jj];
                        }
                        C[ii * n + jj] = sum;
                    }
                }
            }
        }
    }
}

/* Sandy Bridge target - may trigger cache descriptors: 0x48, 0x49, 0x4e */
__attribute__((target("arch=sandybridge")))
void matrix_multiply_sandybridge(int n, double* A, double* B, double* C) {
    /* AVX-optimized tiling */
    const int tile = 128;
    for (int i = 0; i < n; i += tile) {
        for (int j = 0; j < n; j += tile) {
            for (int k = 0; k < n; k += tile) {
                int i_end = (i + tile < n) ? i + tile : n;
                int j_end = (j + tile < n) ? j + tile : n;
                int k_end = (k + tile < n) ? k + tile : n;
                
                for (int ii = i; ii < i_end; ii++) {
                    double* a_row = &A[ii * n];
                    for (int jj = j; jj < j_end; jj++) {
                        double sum = C[ii * n + jj];
                        double* b_col = &B[jj];
                        for (int kk = k; kk < k_end; kk++) {
                            sum += a_row[kk] * b_col[kk * n];
                        }
                        C[ii * n + jj] = sum;
                    }
                }
            }
        }
    }
}

/* Ivy Bridge target - may trigger additional cache descriptors */
__attribute__((target("arch=ivybridge")))
void matrix_multiply_ivybridge(int n, double* A, double* B, double* C) {
    /* Similar to Sandy Bridge but with potential AVX2 */
    matrix_multiply_sandybridge(n, A, B, C);
}

/* ============================================
   PATTERN B: ifunc resolver for runtime dispatch
   ============================================ */

typedef void (*mm_func_t)(int, double*, double*, double*);

/* Resolver function - forces CPU detection */
static mm_func_t resolve_matrix_multiply(void) {
    /* These builtins cause GCC to initialize CPU cache data structures */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return matrix_multiply_ivybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return matrix_multiply_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return matrix_multiply_nehalem;
    } else if (__builtin_cpu_supports("ssse3")) {
        return matrix_multiply_core2;
    } else {
        return matrix_multiply_default;
    }
}

/* ifunc function - will trigger resolver at load time */
void matrix_multiply(int n, double* A, double* B, double* C) 
    __attribute__((ifunc("resolve_matrix_multiply")));

/* ============================================
   PATTERN C: Direct CPUID queries
   ============================================ */

#if IS_X86
static void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* Query CPUID leaf 2 - Cache descriptors (Intel) */
    __cpuid(2, eax, ebx, ecx, edx);
    
    /* Query CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; ; i++) {
        __cpuid_count(4, i, eax, ebx, ecx, edx);
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break;  /* No more caches */
        }
        
        int cache_level = (eax >> 5) & 0x7;
        int line_size = (ebx & 0xFFF) + 1;
        int partitions = ((ebx >> 12) & 0x3FF) + 1;
        int associativity = ((ebx >> 22) & 0x3FF) + 1;
        int sets = ecx + 1;
        
        int size = line_size * partitions * associativity * sets;
        
        if (cache_level == 1 && cache_type == 1) {  /* L1 data cache */
            g_cache_info.l1_size = size / 1024;
            g_cache_info.l1_assoc = associativity;
            g_cache_info.line_size = line_size;
        } else if (cache_level == 2 && cache_type == 3) {  /* L2 unified cache */
            g_cache_info.l2_size = size / 1024;
            g_cache_info.l2_assoc = associativity;
        }
    }
}
#endif

/* ============================================
   PATTERN D: Cache-sensitive computations
   ============================================ */

/* Array sized to match specific cache sizes from uncovered lines */
#define ARRAY_8KB   1024   /* 8KB / sizeof(double) */
#define ARRAY_16KB  2048   /* 16KB / sizeof(double) */
#define ARRAY_32KB  4096   /* 32KB / sizeof(double) */
#define ARRAY_64KB  8192   /* 64KB / sizeof(double) */
#define ARRAY_128KB 16384  /* 128KB / sizeof(double) */
#define ARRAY_256KB 32768  /* 256KB / sizeof(double) */

static double compute_checksum(double* data, size_t size) {
    double sum = 0.0;
    
    /* Use prefetch hints with different locality levels */
    for (size_t i = 0; i < size; i++) {
        if (i + 8 < size) {
            __builtin_prefetch(&data[i + 8], 0, 3);  /* High temporal locality */
        }
        sum += data[i] * 0.5;
    }
    
    return sum;
}

static void test_cache_sized_arrays(void) {
    /* Allocate arrays matching cache sizes from uncovered lines */
    double* array_8kb = malloc(ARRAY_8KB * sizeof(double));
    double* array_16kb = malloc(ARRAY_16KB * sizeof(double));
    double* array_32kb = malloc(ARRAY_32KB * sizeof(double));
    double* array_64kb = malloc(ARRAY_64KB * sizeof(double));
    
    if (!array_8kb || !array_16kb || !array_32kb || !array_64kb) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_64KB; i++) {
        double val = (i % 256) * 0.01;
        if (i < ARRAY_8KB) array_8kb[i] = val;
        if (i < ARRAY_16KB) array_16kb[i] = val;
        if (i < ARRAY_32KB) array_32kb[i] = val;
        array_64kb[i] = val;
    }
    
    /* Compute checksums - these loops may trigger cache model usage */
    double sum1 = compute_checksum(array_8kb, ARRAY_8KB);
    double sum2 = compute_checksum(array_16kb, ARRAY_16KB);
    double sum3 = compute_checksum(array_32kb, ARRAY_32KB);
    double sum4 = compute_checksum(array_64kb, ARRAY_64KB);
    
    /* Use results to prevent optimization */
    volatile double total = sum1 + sum2 + sum3 + sum4;
    (void)total;
    
    free(array_8kb);
    free(array_16kb);
    free(array_32kb);
    free(array_64kb);
}

/* ============================================
   Constructor and main program
   ============================================ */

/* Constructor runs before main - forces early CPU detection */
__attribute__((constructor))
static void init_cache_detection(void) {
#if IS_X86
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Query CPUID directly */
    query_cpuid_cache_info();
    
    /* Check various CPU features to trigger detection */
    int has_sse2 = __builtin_cpu_supports("sse2");
    int has_sse3 = __builtin_cpu_supports("sse3");
    int has_ssse3 = __builtin_cpu_supports("ssse3");
    int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Use results to prevent optimization */
    volatile int features = has_sse2 + has_sse3 + has_ssse3 + 
                           has_sse4_1 + has_sse4_2 + has_avx + has_avx2;
    (void)features;
    
    /* Get system cache info */
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (line_size > 0) {
        g_cache_info.line_size = (int)line_size;
    }
#endif
}

int main(void) {
    printf("Cache Detection Test Program\n");
    printf("============================\n");
    
#if IS_X86
    /* PATTERN B: Extensive use of CPU builtins */
    __builtin_cpu_init();
    
    /* Check for specific Intel CPUs */
    if (__builtin_cpu_is("intel")) {
        printf("CPU Vendor: Intel\n");
        
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
    
    /* Print detected cache info */
    printf("Detected cache line size: %d bytes\n", g_cache_info.line_size);
    if (g_cache_info.l1_size > 0) {
        printf("L1 Data Cache: %d KB, %d-way associative\n", 
               g_cache_info.l1_size, g_cache_info.l1_assoc);
    }
    if (g_cache_info.l2_size > 0) {
        printf("L2 Cache: %d KB, %d-way associative\n", 
               g_cache_info.l2_size, g_cache_info.l2_assoc);
    }
#else
    printf("Non-x86 system - cache detection not applicable\n");
#endif
    
    /* PATTERN D: Cache-sensitive computations */
    printf("\nRunning cache-sensitive computations...\n");
    test_cache_sized_arrays();
    
    /* Matrix multiplication test */
    printf("Running matrix multiplication with ifunc dispatch...\n");
    const int n = 128;
    double* A = malloc(n * n * sizeof(double));
    double* B = malloc(n * n * sizeof(double));
    double* C = malloc(n * n * sizeof(double));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < n * n; i++) {
            A[i] = (i % 100) * 0.01;
            B[i] = ((i + 1) % 100) * 0.01;
            C[i] = 0.0;
        }
        
        /* This will use the ifunc-resolved function */
        matrix_multiply(n, A, B, C);
        
        /* Compute checksum of result */
        double checksum = 0.0;
        for (int i = 0; i < n * n; i++) {
            checksum += C[i];
        }
        printf("Matrix multiplication checksum: %f\n", checksum);
    }
    
    free(A);
    free(B);
    free(C);
    
    printf("\nTest completed successfully.\n");
    return 0;
}

/* Fallback dummy functions for non-x86 */
#if !IS_X86
void matrix_multiply_default(int n, double* A, double* B, double* C) {
    (void)n; (void)A; (void)B; (void)C;
}
void matrix_multiply_core2(int n, double* A, double* B, double* C) {
    (void)n; (void)A; (void)B; (void)C;
}
void matrix_multiply_nehalem(int n, double* A, double* B, double* C) {
    (void)n; (void)A; (void)B; (void)C;
}
void matrix_multiply_sandybridge(int n, double* A, double* B, double* C) {
    (void)n; (void)A; (void)B; (void)C;
}
void matrix_multiply_ivybridge(int n, double* A, double* B, double* C) {
    (void)n; (void)A; (void)B; (void)C;
}
#endif
