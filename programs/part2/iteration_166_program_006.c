/* test_cache_detection.c
 * Comprehensive test to trigger GCC driver's CPUID cache descriptor parsing
 * Targets specific Intel cache descriptor bytes: 0x0a, 0x0c, 0x0d, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with target attributes */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 + 11;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 13;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 + 17;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

static void (*resolve_compute(void))(int*, int) {
    /* Force CPU detection through builtins */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_core2;
    } else {
        return compute_default;
    }
}

__attribute__((ifunc("resolve_compute")))
void compute_dynamic(int* data, int size);

/* Pattern C: Direct CPUID inline assembly */
void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
    }
}

/* Pattern D: Cache-sensitive computations */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=haswell")))
void cache_sensitive_computation(void) {
    /* Arrays sized to match specific cache sizes from uncovered lines */
    const int size_8kb = 2048;      /* 8KB / sizeof(int) for 0x0a */
    const int size_16kb = 4096;     /* 16KB / sizeof(int) for 0x0c, 0x0d */
    const int size_32kb = 8192;     /* 32KB / sizeof(int) for 0x2c */
    const int size_256kb = 65536;   /* 256KB / sizeof(int) for 0x21 */
    const int size_1mb = 262144;    /* 1MB / sizeof(int) for 0x24 */
    
    static int data_8kb[2048] __attribute__((aligned(64)));
    static int data_16kb[4096] __attribute__((aligned(64)));
    static int data_32kb[8192] __attribute__((aligned(64)));
    static int data_256kb[65536] __attribute__((aligned(64)));
    static int data_1mb[262144] __attribute__((aligned(64)));
    
    /* Prefetch hints with different locality levels */
    for (int i = 0; i < size_8kb; i += 16) {
        __builtin_prefetch(&data_8kb[i], 0, 3);  /* Low temporal locality */
    }
    
    for (int i = 0; i < size_16kb; i += 16) {
        __builtin_prefetch(&data_16kb[i], 1, 1);  /* High temporal locality */
    }
    
    /* Matrix multiplication with tiling optimized for cache */
    const int tile_size = 32;  /* Assuming 64-byte cache lines */
    for (int i = 0; i < 256; i += tile_size) {
        for (int j = 0; j < 256; j += tile_size) {
            for (int k = 0; k < 256; k += tile_size) {
                for (int ii = i; ii < i + tile_size && ii < 256; ii++) {
                    for (int jj = j; jj < j + tile_size && jj < 256; jj++) {
                        for (int kk = k; kk < k + tile_size && kk < 256; kk++) {
                            data_256kb[ii * 256 + jj] += 
                                data_16kb[ii * 256 + kk] * 
                                data_8kb[kk * 256 + jj];
                        }
                    }
                }
            }
        }
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_detection(void) {
    printf("Initializing CPU detection...\n");
    __builtin_cpu_init();
    query_cpuid_cache_info();
    
    /* Check various CPU features to trigger detection */
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
}

/* Runtime validation of cache parameters */
static void validate_cache_parameters(void) {
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
#endif
    
#ifdef _SC_LEVEL1_DCACHE_SIZE
    long cache_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    printf("L1 cache size: %ld KB\n", cache_size / 1024);
#endif
    
    /* Compile-time assertion for x86 */
    _Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
                   "Expected 32-bit or 64-bit x86");
}

#else
/* Non-x86 fallback */
void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

void cache_sensitive_computation(void) {
    printf("Non-x86 architecture - using generic computation\n");
}

void validate_cache_parameters(void) {
    printf("Cache validation not available on non-x86\n");
}
#endif

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Pattern B: Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
    /* Check for specific Intel CPUs */
    if (__builtin_cpu_is("core2")) {
        printf("CPU: Intel Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU: Intel Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU: Intel Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU: Intel Ivy Bridge\n");
    }
    if (__builtin_cpu_is("haswell")) {
        printf("CPU: Intel Haswell\n");
    }
    if (__builtin_cpu_is("skylake")) {
        printf("CPU: Intel Skylake\n");
    }
    
    validate_cache_parameters();
    
    /* Execute cache-sensitive computation */
    cache_sensitive_computation();
    
    /* Test dynamic dispatch */
    int test_data[1024];
    for (int i = 0; i < 1024; i++) {
        test_data[i] = i;
    }
    
    compute_dynamic(test_data, 1024);
    
    /* Verify computation */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += test_data[i];
    }
    printf("Computation checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
#else
    printf("This test is designed for x86 architectures\n");
    return 0;
#endif
}
