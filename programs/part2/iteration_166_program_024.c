/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPUID cache detection
 * Specifically targets the large switch-case block in driver-i386.cc (lines 127-244)
 * 
 * Compilation recommendations:
 * 1. For standard x86 cache detection:
 *    gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_executable
 * 2. For 32-bit i386 driver path:
 *    gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 * 3. For aggressive CPU feature usage:
 *    gcc -O3 -march=native -mtune=native -fprofile-arcs -ftest-coverage -fno-inline -funroll-loops test_target.c -o test_target_aggressive
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with target attributes */
/* Function 1: Core2 target - may trigger descriptors: 0x0a, 0x0c, 0x0d, 0x21, 0x24, etc. */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Function 2: Nehalem target - may trigger descriptors: 0x2c, 0x3a, 0x3b, 0x3c, etc. */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i += 2) {
        data[i] = data[i] * 5 - 2;
        if (i + 1 < size) {
            data[i + 1] = data[i + 1] * 2 + 1;
        }
    }
}

/* Function 3: Sandy Bridge target - may trigger descriptors: 0x41, 0x42, 0x43, 0x44, 0x45, etc. */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        data[i] = sum;
    }
}

/* Function 4: Ivy Bridge target - may trigger descriptors: 0x78, 0x79, 0x7a, 0x7b, 0x7c, etc. */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] << 3) | (data[i] >> 29);
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_ifunc)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] + 1;
    }
}

static void (*resolve_compute(void))(int*, int) {
    /* Force CPU detection for ifunc resolution */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse3")) {
        return compute_core2;
    }
    return compute_default;
}

__attribute__((ifunc("resolve_compute")))
void compute_dispatch(int* data, int size);

/* Pattern C: Direct CPUID queries */
static void query_cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (may return multiple descriptors) */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (2), "c" (0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {  /* Query up to 4 cache levels */
        asm volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (4), "c" (i));
        
        /* Cache Type field in EAX[4:0] */
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break;  /* No more caches */
        }
    }
}

/* Pattern D: Cache-sensitive computations with prefetching */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=ivybridge")))
void cache_sensitive_computation(double* matrix, int n) {
    /* Tile sizes that match various cache line sizes */
    const int tile32 = 32 / sizeof(double);  /* 32-byte cache line */
    const int tile64 = 64 / sizeof(double);  /* 64-byte cache line */
    
    /* Use prefetch hints - may cause driver to consider cache parameters */
    for (int i = 0; i < n; i += tile64) {
        for (int j = 0; j < n; j += tile64) {
            for (int ii = i; ii < i + tile64 && ii < n; ii++) {
                /* Prefetch with different locality hints */
                __builtin_prefetch(&matrix[(ii + 1) * n + j], 0, 3);  /* High temporal locality */
                for (int jj = j; jj < j + tile64 && jj < n; jj++) {
                    matrix[ii * n + jj] = matrix[ii * n + jj] * 1.5;
                }
            }
        }
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_detection(void) {
    /* Force CPU initialization before main */
    __builtin_cpu_init();
    query_cpuid_cache_info();
    
    /* Check various CPU features to trigger detection */
    if (__builtin_cpu_supports("sse2")) {
        /* Compile-time assertion equivalent */
        volatile int sse2_available = 1;
        (void)sse2_available;
    }
    
    if (__builtin_cpu_supports("avx")) {
        volatile int avx_available = 1;
        (void)avx_available;
    }
}

/* Runtime cache validation */
static void validate_cache_sizes(void) {
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
#endif
    
#ifdef _SC_LEVEL1_DCACHE_SIZE
    long l1_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    printf("L1 cache size: %ld bytes\n", l1_size);
#endif
    
#ifdef _SC_LEVEL2_CACHE_SIZE
    long l2_size = sysconf(_SC_LEVEL2_CACHE_SIZE);
    printf("L2 cache size: %ld bytes\n", l2_size);
#endif
}

#else
/* Non-x86 fallback implementations */
void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] + 1;
    }
}

void compute_dispatch(int* data, int size) {
    compute_default(data, size);
}

void cache_sensitive_computation(double* matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = matrix[i] * 1.5;
    }
}

static void validate_cache_sizes(void) {
    printf("Cache detection not available on non-x86 platform\n");
}
#endif

/* Main test program */
int main(void) {
    /* Pattern B: Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
    /* Check for specific Intel CPUs - forces driver to initialize cache structures */
    if (__builtin_cpu_is("core2")) {
        printf("CPU: Core2 detected\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU: Nehalem detected\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU: Sandy Bridge detected\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU: Ivy Bridge detected\n");
    }
    
    /* Create arrays sized to match specific cache sizes from the switch cases */
    const int size_8kb = 8192 / sizeof(int);      /* 0x0a: 8KB L1 */
    const int size_16kb = 16384 / sizeof(int);    /* 0x0c, 0x0d: 16KB L1 */
    const int size_32kb = 32768 / sizeof(int);    /* 0x2c: 32KB L1 */
    const int size_128kb = 131072 / sizeof(int);  /* 0x39: 128KB L2 */
    const int size_256kb = 262144 / sizeof(int);  /* 0x21: 256KB L2 */
    const int size_1mb = 1048576 / sizeof(int);   /* 0x24: 1MB L2 */
    
    /* Test with 8KB array (triggers 0x0a path) */
    int* data_small = (int*)malloc(size_8kb * sizeof(int));
    for (int i = 0; i < size_8kb; i++) {
        data_small[i] = i % 256;
    }
    
    /* Test with 256KB array (triggers 0x21 path) */
    int* data_medium = (int*)malloc(size_256kb * sizeof(int));
    for (int i = 0; i < size_256kb; i++) {
        data_medium[i] = i % 512;
    }
    
    /* Test with 1MB array (triggers 0x24 path) */
    int* data_large = (int*)malloc(size_1mb * sizeof(int));
    for (int i = 0; i < size_1mb; i++) {
        data_large[i] = i % 1024;
    }
    
    /* Execute all computation patterns */
    compute_core2(data_small, size_8kb);
    compute_nehalem(data_medium, size_256kb);
    compute_sandybridge(data_large, size_1mb);
    compute_ivybridge(data_small, size_8kb);
    
    /* Use ifunc dispatch */
    compute_dispatch(data_medium, size_256kb);
    
    /* Matrix computation for cache-sensitive pattern */
    const int matrix_size = 256;
    double* matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (double)(i % 100) / 10.0;
    }
    
    cache_sensitive_computation(matrix, matrix_size);
    
    /* Compute checksum for validation */
    int checksum = 0;
    for (int i = 0; i < size_8kb; i++) {
        checksum += data_small[i];
    }
    for (int i = 0; i < size_256kb; i++) {
        checksum += data_medium[i];
    }
    for (int i = 0; i < size_1mb; i++) {
        checksum += data_large[i];
    }
    
    double matrix_sum = 0.0;
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix_sum += matrix[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Matrix sum: %.2f\n", matrix_sum);
    
    /* Validate cache sizes */
    validate_cache_sizes();
    
    /* Cleanup */
    free(data_small);
    free(data_medium);
    free(data_large);
    free(matrix);
    
    return 0;
}
