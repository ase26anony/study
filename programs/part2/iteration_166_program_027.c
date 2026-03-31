/*
 * test_target.c - Test program to trigger GCC driver CPU cache detection
 * 
 * This program uses multiple techniques to force the GCC driver to parse
 * Intel CPUID cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.) during
 * compilation and runtime CPU feature detection.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* ============================================
   PATTERN 1: Multiple target attributes for different Intel architectures
   ============================================ */

/* Function with target attribute for Core 2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Matrix multiplication tuned for Core 2 cache */
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
    /* Matrix multiplication tuned for Nehalem cache */
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

/* Function with target attribute for Sandy Bridge architecture */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Matrix multiplication tuned for Sandy Bridge cache */
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

/* Function with target attribute for Ivy Bridge architecture */
__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Matrix multiplication tuned for Ivy Bridge cache */
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

/* ============================================
   PATTERN 2: ifunc resolver for runtime dispatch
   ============================================ */

/* Base implementation */
static void default_matrix_multiply(int n, double *A, double *B, double *C) {
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

/* Resolver function for ifunc */
static void (*resolve_matrix_multiply(void))(int, double*, double*, double*) {
    /* These builtins cause GCC driver to initialize CPU cache data structures */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return ivybridge_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("avx")) {
        return sandybridge_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return core2_optimized_matrix_multiply;
    }
    
    return default_matrix_multiply;
}

/* ifunc declaration */
void dynamic_matrix_multiply(int n, double *A, double *B, double *C)
    __attribute__((ifunc("resolve_matrix_multiply")));

/* ============================================
   PATTERN 3: Multi-architecture compilation with target_clones
   ============================================ */

__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void cache_aware_computation(double *data, size_t size) {
    /* Loop with cache-friendly access pattern */
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        /* Use __builtin_prefetch with different locality hints */
        if (i + 16 < size) {
            __builtin_prefetch(&data[i + 16], 0, 3); /* High temporal locality */
        }
        sum += data[i] * 0.5;
    }
    data[0] = sum; /* Store result */
}

/* ============================================
   PATTERN 4: Direct CPUID queries via inline assembly
   ============================================ */

static void query_cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB Descriptors */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (2), "c" (0));
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (4), "c" (i));
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_cache_info(void) {
    query_cpuid_cache_info();
}

/* ============================================
   PATTERN 5: Cache-sensitive computations with specific sizes
   ============================================ */

/* Arrays sized to match specific cache line sizes */
#define CACHE_LINE_32_SIZE  (32 / sizeof(double))
#define CACHE_LINE_64_SIZE  (64 / sizeof(double))

/* Arrays sized to match specific cache sizes (in KB) */
#define CACHE_8KB_SIZE      (8 * 1024 / sizeof(double))
#define CACHE_16KB_SIZE     (16 * 1024 / sizeof(double))
#define CACHE_32KB_SIZE     (32 * 1024 / sizeof(double))
#define CACHE_256KB_SIZE    (256 * 1024 / sizeof(double))

static void cache_line_aligned_computation(void) {
    /* Allocate aligned memory for cache line testing */
    double *data32 = aligned_alloc(32, CACHE_LINE_32_SIZE * sizeof(double));
    double *data64 = aligned_alloc(64, CACHE_LINE_64_SIZE * sizeof(double));
    
    if (data32 && data64) {
        /* Initialize data */
        for (int i = 0; i < CACHE_LINE_32_SIZE; i++) data32[i] = i * 1.0;
        for (int i = 0; i < CACHE_LINE_64_SIZE; i++) data64[i] = i * 2.0;
        
        /* Perform computations */
        cache_aware_computation(data32, CACHE_LINE_32_SIZE);
        cache_aware_computation(data64, CACHE_LINE_64_SIZE);
        
        free(data32);
        free(data64);
    }
}

static void cache_size_specific_computation(void) {
    /* Test different cache sizes */
    double *cache8kb = malloc(CACHE_8KB_SIZE * sizeof(double));
    double *cache16kb = malloc(CACHE_16KB_SIZE * sizeof(double));
    double *cache32kb = malloc(CACHE_32KB_SIZE * sizeof(double));
    double *cache256kb = malloc(CACHE_256KB_SIZE * sizeof(double));
    
    if (cache8kb && cache16kb && cache32kb && cache256kb) {
        /* Initialize and compute */
        for (size_t i = 0; i < CACHE_8KB_SIZE; i++) cache8kb[i] = i * 0.1;
        for (size_t i = 0; i < CACHE_16KB_SIZE; i++) cache16kb[i] = i * 0.2;
        for (size_t i = 0; i < CACHE_32KB_SIZE; i++) cache32kb[i] = i * 0.3;
        for (size_t i = 0; i < CACHE_256KB_SIZE; i++) cache256kb[i] = i * 0.4;
        
        cache_aware_computation(cache8kb, CACHE_8KB_SIZE);
        cache_aware_computation(cache16kb, CACHE_16KB_SIZE);
        cache_aware_computation(cache32kb, CACHE_32KB_SIZE);
        cache_aware_computation(cache256kb, CACHE_256KB_SIZE);
        
        free(cache8kb);
        free(cache16kb);
        free(cache32kb);
        free(cache256kb);
    }
}

#else /* Non-x86 fallback */

/* Dummy implementations for non-x86 targets */
void dynamic_matrix_multiply(int n, double *A, double *B, double *C) {
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

void cache_aware_computation(double *data, size_t size) {
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += data[i] * 0.5;
    }
    data[0] = sum;
}

#endif /* x86 check */

/* ============================================
   Main function with extensive CPU feature checks
   ============================================ */

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Force GCC driver to initialize CPU cache detection */
    __builtin_cpu_init();
    
    /* Extensive CPU feature checks - each may trigger cache detection */
    printf("CPU Feature Detection:\n");
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("  SSSE3: %s\n", __builtin_cpu_supports("ssse3") ? "yes" : "no");
    printf("  SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("  SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("  AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    
    /* CPU model checks */
    printf("\nCPU Model Detection:\n");
    printf("  Intel Core 2: %s\n", __builtin_cpu_is("core2") ? "yes" : "no");
    printf("  Intel Nehalem: %s\n", __builtin_cpu_is("nehalem") ? "yes" : "no");
    printf("  Intel Sandy Bridge: %s\n", __builtin_cpu_is("sandybridge") ? "yes" : "no");
    printf("  Intel Ivy Bridge: %s\n", __builtin_cpu_is("ivybridge") ? "yes" : "no");
    
    /* Runtime cache line size query */
    printf("\nCache Information:\n");
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    printf("  L1 Cache Line Size: %ld bytes\n", sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
#endif
    
    /* Perform cache-sensitive computations */
    printf("\nPerforming cache-sensitive computations...\n");
    
    /* Test 1: Matrix multiplication with ifunc dispatch */
    const int MATRIX_SIZE = 64;
    double *A = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *B = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *C = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            A[i] = (double)rand() / RAND_MAX;
            B[i] = (double)rand() / RAND_MAX;
        }
        
        /* Use ifunc-dispatched function */
        dynamic_matrix_multiply(MATRIX_SIZE, A, B, C);
        
        /* Also call architecture-specific versions directly */
        core2_optimized_matrix_multiply(MATRIX_SIZE, A, B, C);
        nehalem_optimized_matrix_multiply(MATRIX_SIZE, A, B, C);
        sandybridge_optimized_matrix_multiply(MATRIX_SIZE, A, B, C);
        ivybridge_optimized_matrix_multiply(MATRIX_SIZE, A, B, C);
        
        free(A);
        free(B);
        free(C);
    }
    
    /* Test 2: Cache line aligned computations */
    cache_line_aligned_computation();
    
    /* Test 3: Cache size specific computations */
    cache_size_specific_computation();
    
    printf("\nAll tests completed successfully.\n");
    
#else
    printf("Non-x86 architecture detected. Running generic tests.\n");
    
    /* Generic fallback test */
    const int SIZE = 100;
    double *data = malloc(SIZE * sizeof(double));
    if (data) {
        for (int i = 0; i < SIZE; i++) {
            data[i] = i * 1.0;
        }
        cache_aware_computation(data, SIZE);
        free(data);
    }
#endif
    
    return 0;
}
