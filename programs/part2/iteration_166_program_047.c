/*
 * test_target.c - Comprehensive test to trigger GCC driver CPU cache detection
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

/* ========== PATTERN 1: Multiple Target Attributes ========== */

/* Function with target attribute for Core 2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
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

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Tiled matrix multiplication optimized for Nehalem cache */
    const int TILE = 32; /* Matches typical cache line sizes */
    for (int i = 0; i < n; i += TILE) {
        for (int j = 0; j < n; j += TILE) {
            for (int k = 0; k < n; k += TILE) {
                for (int ii = i; ii < i + TILE && ii < n; ii++) {
                    for (int jj = j; jj < j + TILE && jj < n; jj++) {
                        double sum = C[ii * n + jj];
                        for (int kk = k; kk < k + TILE && kk < n; kk++) {
                            sum += A[ii * n + kk] * B[kk * n + jj];
                        }
                        C[ii * n + jj] = sum;
                    }
                }
            }
        }
    }
}

/* Function with target attribute for Sandy Bridge architecture */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Different tiling strategy for Sandy Bridge */
    const int TILE = 64; /* Matches 64-byte cache lines */
    for (int i = 0; i < n; i += TILE) {
        for (int j = 0; j < n; j += TILE) {
            for (int k = 0; k < n; k += TILE) {
                for (int ii = i; ii < i + TILE && ii < n; ii++) {
                    for (int jj = j; jj < j + TILE && jj < n; jj++) {
                        double sum = C[ii * n + jj];
                        for (int kk = k; kk < k + TILE && kk < n; kk++) {
                            sum += A[ii * n + kk] * B[kk * n + jj];
                        }
                        C[ii * n + jj] = sum;
                    }
                }
            }
        }
    }
}

/* ========== PATTERN 2: IFUNC for Runtime Dispatch ========== */

/* Base implementation */
static void default_matrix_multiply(int n, double *A, double *B, double *C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
}

/* Resolver function for ifunc */
static void (*resolve_matrix_multiply(void))(int, double*, double*, double*) {
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("sse2")) {
        return core2_optimized_matrix_multiply;
    }
    
    return default_matrix_multiply;
}

/* IFUNC declaration - forces driver to detect CPU features */
void dynamic_matrix_multiply(int n, double *A, double *B, double *C)
    __attribute__((ifunc("resolve_matrix_multiply")));

/* ========== PATTERN 3: Multi-Versioned Function ========== */

/* Function with multiple target clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multiversion_matrix_multiply(int n, double *A, double *B, double *C) {
    const int TILE = 16; /* Matches 8KB/16KB cache sizes from uncovered lines */
    for (int i = 0; i < n; i += TILE) {
        for (int j = 0; j < n; j += TILE) {
            for (int k = 0; k < n; k += TILE) {
                for (int ii = i; ii < i + TILE && ii < n; ii++) {
                    for (int jj = j; jj < j + TILE && jj < n; jj++) {
                        double sum = C[ii * n + jj];
                        for (int kk = k; kk < k + TILE && kk < n; kk++) {
                            sum += A[ii * n + kk] * B[kk * n + jj];
                        }
                        C[ii * n + jj] = sum;
                    }
                }
            }
        }
    }
}

/* ========== PATTERN 4: Direct CPUID Queries ========== */

/* Constructor that runs CPUID queries before main */
__attribute__((constructor))
static void init_cpu_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 0 - Get vendor string */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    
    /* CPUID leaf 1 - Get feature bits */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    /* CPUID leaf 2 - Cache descriptors (triggers the switch-case block) */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(2));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(4), "c"(i));
    }
    
    printf("[Constructor] CPUID queries executed\n");
}

/* ========== PATTERN 5: Cache-Sensitive Computations ========== */

/* Array sizes that match specific cache sizes from uncovered lines */
#define SIZE_8KB    1024    /* 8KB / sizeof(double) */
#define SIZE_16KB   2048    /* 16KB / sizeof(double) */
#define SIZE_32KB   4096    /* 32KB / sizeof(double) */
#define SIZE_64KB   8192    /* 64KB / sizeof(double) */
#define SIZE_128KB  16384   /* 128KB / sizeof(double) */
#define SIZE_256KB  32768   /* 256KB / sizeof(double) */
#define SIZE_512KB  65536   /* 512KB / sizeof(double) */
#define SIZE_1MB    131072  /* 1MB / sizeof(double) */
#define SIZE_2MB    262144  /* 2MB / sizeof(double) */

/* Prefetch patterns for different cache line sizes */
static void cache_line_optimized_sum(double *array, int size, double *result) {
    double sum = 0.0;
    
    /* Use prefetch for 32-byte cache lines */
    for (int i = 0; i < size; i += 4) { /* 4 doubles = 32 bytes */
        __builtin_prefetch(&array[i + 32], 0, 3); /* High temporal locality */
        sum += array[i];
    }
    
    /* Use prefetch for 64-byte cache lines */
    for (int i = 0; i < size; i += 8) { /* 8 doubles = 64 bytes */
        __builtin_prefetch(&array[i + 64], 0, 2); /* Medium temporal locality */
        sum += array[i + 1];
    }
    
    *result = sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* PATTERN B: Extensive use of CPU detection builtins */
    printf("CPU Detection Results:\n");
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
    
    printf("CPU Feature Support:\n");
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
    
    /* Get system cache information */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 Cache Line Size: %ld bytes\n", cache_line);
    
    /* Compile-time assertion for x86 */
    #if !defined(__i386__) && !defined(__x86_64__)
    #error "This test requires x86 architecture"
    #endif
    
    /* Create test matrices */
    const int MATRIX_SIZE = 128;
    double *A = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *B = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *C1 = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *C2 = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *C3 = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    
    if (!A || !B || !C1 || !C2 || !C3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        A[i] = (double)rand() / RAND_MAX;
        B[i] = (double)rand() / RAND_MAX;
        C1[i] = 0.0;
        C2[i] = 0.0;
        C3[i] = 0.0;
    }
    
    /* Execute different matrix multiplication implementations */
    printf("\nExecuting matrix multiplications...\n");
    
    /* Call multi-versioned function */
    multiversion_matrix_multiply(MATRIX_SIZE, A, B, C1);
    
    /* Call ifunc-resolved function */
    dynamic_matrix_multiply(MATRIX_SIZE, A, B, C2);
    
    /* Call target-specific functions directly */
    core2_optimized_matrix_multiply(MATRIX_SIZE, A, B, C3);
    nehalem_optimized_matrix_multiply(MATRIX_SIZE, A, B, C3);
    sandybridge_optimized_matrix_multiply(MATRIX_SIZE, A, B, C3);
    
    /* PATTERN D: Cache-sensitive computations with various sizes */
    printf("\nPerforming cache-sensitive computations...\n");
    
    double *cache_test_arrays[8];
    double results[8] = {0};
    
    /* Allocate arrays matching cache sizes from uncovered lines */
    cache_test_arrays[0] = malloc(SIZE_8KB * sizeof(double));
    cache_test_arrays[1] = malloc(SIZE_16KB * sizeof(double));
    cache_test_arrays[2] = malloc(SIZE_32KB * sizeof(double));
    cache_test_arrays[3] = malloc(SIZE_64KB * sizeof(double));
    cache_test_arrays[4] = malloc(SIZE_128KB * sizeof(double));
    cache_test_arrays[5] = malloc(SIZE_256KB * sizeof(double));
    cache_test_arrays[6] = malloc(SIZE_512KB * sizeof(double));
    cache_test_arrays[7] = malloc(SIZE_1MB * sizeof(double));
    
    /* Initialize and process each array */
    for (int i = 0; i < 8; i++) {
        if (cache_test_arrays[i]) {
            int size = (i == 0) ? SIZE_8KB :
                      (i == 1) ? SIZE_16KB :
                      (i == 2) ? SIZE_32KB :
                      (i == 3) ? SIZE_64KB :
                      (i == 4) ? SIZE_128KB :
                      (i == 5) ? SIZE_256KB :
                      (i == 6) ? SIZE_512KB : SIZE_1MB;
            
            for (int j = 0; j < size; j++) {
                cache_test_arrays[i][j] = (double)rand() / RAND_MAX;
            }
            
            cache_line_optimized_sum(cache_test_arrays[i], size, &results[i]);
        }
    }
    
    /* Compute final checksum */
    double final_checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        final_checksum += C1[i] + C2[i] + C3[i];
    }
    
    for (int i = 0; i < 8; i++) {
        final_checksum += results[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(A); free(B); free(C1); free(C2); free(C3);
    for (int i = 0; i < 8; i++) {
        free(cache_test_arrays[i]);
    }
    
    return 0;
}

#else /* Non-x86 fallback */

int main(void) {
    printf("This test is designed for x86 architecture only\n");
    printf("Compile with: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c\n");
    return 0;
}

#endif /* __i386__ || __x86_64__ */
