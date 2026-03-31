/*
 * test_target.c - Comprehensive test to trigger GCC driver CPU cache detection
 * 
 * This program uses multiple techniques to force the GCC driver to parse
 * Intel CPUID cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.) during compilation.
 * The goal is to exercise the large switch-case block in driver-i386.cc lines 127-244.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* ============================================
   PATTERN 1: Multiple target attributes to force
   CPU detection for different Intel architectures
   ============================================ */

/* Function with target attribute for Core 2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_function(int *arr, int n) {
    /* Use prefetch hints that depend on cache line size */
    for (int i = 0; i < n; i += 16) {
        __builtin_prefetch(&arr[i + 32], 0, 3); /* High temporal locality */
    }
    
    /* Computation sized for Core 2 cache */
    int sum = 0;
    for (int i = 0; i < n && i < 8192; i++) { /* 8KB L1 cache size */
        sum += arr[i] * 3;
    }
    arr[0] = sum;
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_function(int *arr, int n) {
    /* Different prefetch pattern */
    for (int i = 0; i < n; i += 8) {
        __builtin_prefetch(&arr[i + 64], 0, 1); /* Low temporal locality */
    }
    
    /* Computation sized for Nehalem cache */
    int sum = 0;
    for (int i = 0; i < n && i < 32768; i++) { /* 32KB L1 cache size */
        sum += arr[i] * 7;
    }
    arr[1] = sum;
}

/* Function with target attribute for Sandy Bridge architecture */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_function(int *arr, int n) {
    /* Vector-friendly loop with cache line alignment */
    for (int i = 0; i < n; i += 16) {
        __builtin_prefetch(&arr[i + 128], 0, 2);
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < 65536; i++) { /* 64KB L1 cache size */
        sum += arr[i] * 11;
    }
    arr[2] = sum;
}

/* Function with target attribute for Ivy Bridge architecture */
__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_function(int *arr, int n) {
    /* More aggressive prefetching */
    for (int i = 0; i < n; i += 32) {
        __builtin_prefetch(&arr[i + 256], 0, 0);
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < 131072; i++) { /* 128KB L2 cache size */
        sum += arr[i] * 13;
    }
    arr[3] = sum;
}

/* ============================================
   PATTERN 2: ifunc resolver for runtime dispatch
   This forces CPU detection during ifunc resolution
   ============================================ */

typedef void (*compute_func_t)(int*, int);

/* Resolver function that uses CPU feature detection */
static compute_func_t resolve_compute(void) {
    /* These builtins cause CPU detection to run */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return ivybridge_optimized_function;
    } else if (__builtin_cpu_supports("avx")) {
        return sandybridge_optimized_function;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_function;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return core2_optimized_function;
    }
    
    /* Default fallback */
    return core2_optimized_function;
}

/* ifunc function that will be resolved at runtime */
void ifunc_compute(int *arr, int n) 
    __attribute__((ifunc("resolve_compute")));

/* ============================================
   PATTERN 3: Function multiversioning
   ============================================ */

__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multiversion_compute(int *arr, int n) {
    /* Cache-sensitive computation */
    int block_size = 64; /* Common cache line size */
    int sum = 0;
    
    for (int i = 0; i < n; i += block_size) {
        for (int j = i; j < i + block_size && j < n; j++) {
            sum += arr[j];
        }
    }
    
    arr[4] = sum;
}

/* ============================================
   PATTERN 4: Direct CPUID inline assembly
   ============================================ */

static void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB descriptors */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (2), "c" (0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (4), "c" (0));
    
    /* Additional queries for different cache levels */
    for (int i = 1; i < 4; i++) {
        asm volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (4), "c" (i));
    }
}

/* ============================================
   PATTERN 5: Constructor that runs before main
   ============================================ */

__attribute__((constructor))
static void init_cpu_detection(void) {
    /* Force CPU initialization early */
    __builtin_cpu_init();
    
    /* Query CPUID directly */
    query_cpuid_cache_info();
    
    /* Check various CPU features to trigger detection */
    if (__builtin_cpu_supports("sse2")) {
        /* This should always be true for x86-64 */
    }
    
    if (__builtin_cpu_supports("sse3")) {
        /* Common feature check */
    }
    
    if (__builtin_cpu_supports("ssse3")) {
        /* Another feature check */
    }
    
    if (__builtin_cpu_supports("sse4.1")) {
        /* More feature checks */
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* Even more checks */
    }
    
    if (__builtin_cpu_is("core2")) {
        /* CPU model check */
    }
    
    if (__builtin_cpu_is("nehalem")) {
        /* Another CPU model check */
    }
    
    if (__builtin_cpu_is("sandybridge")) {
        /* More model checks */
    }
}

/* ============================================
   Main test function with cache-sensitive computation
   ============================================ */

int main(void) {
    /* Initialize CPU detection in main as well */
    __builtin_cpu_init();
    
    /* Extensive CPU feature and model checks */
    printf("CPU Feature Detection:\n");
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("  SSSE3: %s\n", __builtin_cpu_supports("ssse3") ? "yes" : "no");
    printf("  SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("  SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("  AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    
    /* Query system cache information */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 Cache Line Size: %ld bytes\n", cache_line);
    
    /* Create test data sized to various cache levels */
    const int data_size = 256 * 1024; /* 256KB - spans multiple cache levels */
    int *data = (int*)malloc(data_size * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < data_size; i++) {
        data[i] = i % 100;
    }
    
    /* Execute all variants of cache-sensitive functions */
    printf("\nExecuting cache-sensitive computations:\n");
    
    /* Pattern 1: Call all target-specific functions */
    core2_optimized_function(data, data_size);
    printf("  Core2 optimized function completed\n");
    
    nehalem_optimized_function(data, data_size);
    printf("  Nehalem optimized function completed\n");
    
    sandybridge_optimized_function(data, data_size);
    printf("  Sandy Bridge optimized function completed\n");
    
    ivybridge_optimized_function(data, data_size);
    printf("  Ivy Bridge optimized function completed\n");
    
    /* Pattern 2: Call ifunc function */
    ifunc_compute(data, data_size);
    printf("  ifunc dispatched function completed\n");
    
    /* Pattern 3: Call multiversion function */
    multiversion_compute(data, data_size);
    printf("  Multiversion function completed\n");
    
    /* Pattern 4: Matrix multiplication with cache blocking */
    /* This encourages the compiler to use cache size in optimizations */
    const int matrix_size = 128; /* Fits in L2 cache for many CPUs */
    int *matrix_a = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    int *matrix_b = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    int *matrix_c = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    
    if (matrix_a && matrix_b && matrix_c) {
        /* Initialize matrices */
        for (int i = 0; i < matrix_size * matrix_size; i++) {
            matrix_a[i] = i % 10;
            matrix_b[i] = (i + 5) % 10;
            matrix_c[i] = 0;
        }
        
        /* Cache-blocked matrix multiplication */
        int block_size = 16; /* Common blocking factor */
        for (int i = 0; i < matrix_size; i += block_size) {
            for (int j = 0; j < matrix_size; j += block_size) {
                for (int k = 0; k < matrix_size; k += block_size) {
                    for (int ii = i; ii < i + block_size && ii < matrix_size; ii++) {
                        for (int jj = j; jj < j + block_size && jj < matrix_size; jj++) {
                            for (int kk = k; kk < k + block_size && kk < matrix_size; kk++) {
                                matrix_c[ii * matrix_size + jj] += 
                                    matrix_a[ii * matrix_size + kk] * 
                                    matrix_b[kk * matrix_size + jj];
                            }
                        }
                    }
                }
            }
        }
        
        printf("  Cache-blocked matrix multiplication completed\n");
        
        free(matrix_a);
        free(matrix_b);
        free(matrix_c);
    }
    
    /* Compute checksum for validation */
    unsigned long long checksum = 0;
    for (int i = 0; i < data_size && i < 10000; i++) {
        checksum += data[i];
    }
    
    printf("\nValidation checksum: %llu\n", checksum);
    printf("Test completed successfully.\n");
    
    free(data);
    return 0;
}

#else /* Non-x86 fallback */

/* Dummy implementation for non-x86 systems */
int main(void) {
    printf("This test is designed for x86 systems only.\n");
    printf("On non-x86 systems, the GCC driver cache detection code won't be exercised.\n");
    return 0;
}

#endif /* __i386__ || __x86_64__ */
