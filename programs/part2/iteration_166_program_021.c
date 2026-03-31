/*
 * test_cache_descriptors.c
 * 
 * This program is designed to trigger GCC driver's CPUID cache detection
 * logic for specific Intel cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.)
 * found in driver-i386.cc lines 127-244.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* ========== PATTERN A: Function Multiversioning with Target Attributes ========== */

/* Function 1: Core2 target - may trigger descriptors like 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 1;
    }
}

/* Function 2: Nehalem target - may trigger descriptors like 0x0a, 0x0c, 0x0d */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 2;
    }
}

/* Function 3: Sandy Bridge target - may trigger descriptors like 0x3a, 0x3b, 0x3c */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 3;
    }
}

/* Function 4: Ivy Bridge target - may trigger descriptors like 0x3d, 0x3e, 0x4e */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 5;
    }
}

/* ========== PATTERN B: ifunc resolver for runtime dispatch ========== */

/* Base implementation */
static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

/* Resolver function - forces CPU detection */
static void (*resolve_compute(void))(int*, int) {
    /* These builtins cause CPU detection to run */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("ssse3")) {
        return compute_core2;
    }
    return compute_default;
}

/* ifunc function - driver must detect CPU features for resolver */
void compute_optimized(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* ========== PATTERN C: Direct CPUID queries ========== */

static void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (may return multiple descriptors) */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (2), "c" (0));
    
    printf("CPUID leaf 2: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n", 
           eax, ebx, ecx, edx);
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 8; i++) {  /* Query up to 8 cache levels */
        ecx = i;
        asm volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (4), "c" (i));
        
        if ((eax & 0x1f) == 0)  /* Cache type field = 0 means no more caches */
            break;
            
        printf("Cache level %d: type=%u, level=%u, size=%uKB, linesize=%u\n",
               i, eax & 0x1f, (eax >> 5) & 0x7,
               ((ebx >> 22) + 1) * (((ebx >> 12) & 0x3ff) + 1) * 
               ((ebx & 0xfff) + 1) * (ecx + 1) / 1024,
               (ebx & 0xfff) + 1);
    }
}

/* ========== PATTERN D: Cache-sensitive computations ========== */

/* Array sizes matching specific cache sizes from the uncovered lines */
#define SIZE_8KB    (8 * 1024 / sizeof(int))      /* 0x0a, 0x66 */
#define SIZE_16KB   (16 * 1024 / sizeof(int))     /* 0x0c, 0x0d, 0x67 */
#define SIZE_32KB   (32 * 1024 / sizeof(int))     /* 0x2c, 0x68 */
#define SIZE_256KB  (256 * 1024 / sizeof(int))    /* 0x21, 0x3c, 0x82 */
#define SIZE_512KB  (512 * 1024 / sizeof(int))    /* 0x3e, 0x7f, 0x80, 0x83, 0x86 */
#define SIZE_1MB    (1024 * 1024 / sizeof(int))   /* 0x24, 0x78, 0x7c, 0x84 */
#define SIZE_2MB    (2048 * 1024 / sizeof(int))   /* 0x45, 0x7d, 0x85 */

/* Matrix multiplication with cache-aware tiling */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void matrix_multiply_tiled(int n, int* A, int* B, int* C) {
    const int TILE = 32;  /* Try to match cache line sizes (32 or 64) */
    
    for (int i = 0; i < n; i += TILE) {
        for (int j = 0; j < n; j += TILE) {
            for (int k = 0; k < n; k += TILE) {
                for (int ii = i; ii < i + TILE && ii < n; ii++) {
                    for (int kk = k; kk < k + TILE && kk < n; kk++) {
                        for (int jj = j; jj < j + TILE && jj < n; jj++) {
                            C[ii * n + jj] += A[ii * n + kk] * B[kk * n + jj];
                        }
                    }
                }
            }
        }
    }
}

/* Prefetch patterns for different cache line sizes */
void prefetch_patterns(int* data, int size) {
    /* Prefetch for 32-byte cache lines */
    for (int i = 0; i < size; i += 8) {  /* 8 ints = 32 bytes */
        __builtin_prefetch(&data[i], 0, 3);  /* High locality read */
    }
    
    /* Prefetch for 64-byte cache lines */
    for (int i = 0; i < size; i += 16) {  /* 16 ints = 64 bytes */
        __builtin_prefetch(&data[i], 1, 2);  /* Medium locality write */
    }
}

/* ========== Validation and Debugging ========== */

/* Constructor runs before main - forces early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    printf("=== CPU Cache Detection Initialization ===\n");
    
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check various CPU features - each causes cache detection */
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("sse4.2")) {
        printf("SSE4.2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
    }
    
    /* Check specific Intel CPUs */
    if (__builtin_cpu_is("core2")) {
        printf("CPU: Core2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU: Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU: Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU: Ivy Bridge\n");
    }
    
    /* Query system cache info */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
    
    /* Direct CPUID query */
    query_cpuid_cache_info();
    
    printf("=========================================\n");
}

#else
/* Non-x86 fallback implementations */
void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

void compute_optimized(int* data, int size) {
    compute_default(data, size);
}

void matrix_multiply_tiled(int n, int* A, int* B, int* C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
}

__attribute__((constructor))
static void init_cpu_detection(void) {
    printf("Non-x86 architecture - using generic code\n");
}
#endif

/* ========== Main test program ========== */

int main(void) {
    printf("=== Cache Descriptor Test Program ===\n");
    
    /* Allocate arrays of various cache-sized dimensions */
    int* data_small = malloc(SIZE_16KB * sizeof(int));
    int* data_medium = malloc(SIZE_256KB * sizeof(int));
    int* data_large = malloc(SIZE_1MB * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < SIZE_16KB; i++) data_small[i] = i % 100;
    for (int i = 0; i < SIZE_256KB; i++) data_medium[i] = i % 100;
    for (int i = 0; i < SIZE_1MB; i++) data_large[i] = i % 100;
    
    /* Pattern D: Cache-sensitive computations */
    printf("\n--- Running cache-sensitive computations ---\n");
    
    /* Use prefetch patterns */
    prefetch_patterns(data_small, SIZE_16KB);
    prefetch_patterns(data_medium, SIZE_256KB);
    
    /* Call ifunc-resolved function (triggers CPU detection) */
    compute_optimized(data_small, SIZE_16KB);
    
    /* Call target-specific functions directly */
    compute_core2(data_medium, 1000);
    compute_nehalem(data_medium, 1000);
    compute_sandybridge(data_medium, 1000);
    compute_ivybridge(data_medium, 1000);
    
    /* Matrix multiplication with tiling */
    const int MATRIX_SIZE = 128;
    int* A = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* B = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* C = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    if (A && B && C) {
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            A[i] = i % 10;
            B[i] = (i + 5) % 10;
            C[i] = 0;
        }
        
        matrix_multiply_tiled(MATRIX_SIZE, A, B, C);
        
        /* Compute checksum for validation */
        long long checksum = 0;
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            checksum += C[i];
        }
        printf("Matrix checksum: %lld\n", checksum);
    }
    
    /* Compute final checksum */
    long long final_sum = 0;
    for (int i = 0; i < SIZE_16KB; i++) final_sum += data_small[i];
    for (int i = 0; i < SIZE_256KB; i++) final_sum += data_medium[i];
    for (int i = 0; i < SIZE_1MB; i++) final_sum += data_large[i];
    
    printf("Final data checksum: %lld\n", final_sum);
    printf("=== Test completed successfully ===\n");
    
    /* Cleanup */
    free(data_small);
    free(data_medium);
    free(data_large);
    free(A); free(B); free(C);
    
    return 0;
}
