/* test_cache_detection.c - Comprehensive test to trigger Intel CPU cache descriptor parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Multiple target attributes for different Intel architectures */
/* Function 1: Core2 target - may trigger descriptors like 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Function 2: Nehalem target - may trigger descriptors like 0x0a, 0x0c, 0x0d */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 3;
    }
}

/* Function 3: Sandy Bridge target - may trigger descriptors like 0x2c, 0x3a, 0x3b */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 11;
    }
}

/* Function 4: Ivy Bridge target - may trigger descriptors like 0x78, 0x79, 0x7a */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 5;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void (*resolve_compute(void)) (int*, int) {
    /* These builtins cause driver to initialize CPU cache data structures */
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
    return compute_core2; /* default */
}

/* ifunc attribute causes resolver to run before main */
void compute_dispatch(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Multi-versioned function using target_clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multi_version_compute(float* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 1.5f;
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void early_cpu_detection(void) {
    /* Force CPUID leaf 2 query (cache descriptors) */
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB Descriptor information */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    asm volatile (
        "xor %%ecx, %%ecx\n\t"
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(0)
    );
    
    printf("[Constructor] CPUID executed for cache detection\n");
}

/* Pattern D: Cache-sensitive computation with prefetch hints */
void cache_optimized_computation(void) {
    /* Arrays sized to match specific cache sizes from uncovered lines */
    
    /* 8KB array (matches 0x0a, 0x66 cases) */
    int array_8k[2048]; /* 2048 * 4 bytes = 8192 bytes */
    
    /* 16KB array (matches 0x0c, 0x0d, 0x60, 0x67 cases) */
    int array_16k[4096]; /* 4096 * 4 bytes = 16384 bytes */
    
    /* 32KB array (matches 0x2c, 0x68 cases) */
    int array_32k[8192]; /* 8192 * 4 bytes = 32768 bytes */
    
    /* Initialize arrays */
    for (int i = 0; i < 2048; i++) array_8k[i] = i;
    for (int i = 0; i < 4096; i++) array_16k[i] = i;
    for (int i = 0; i < 8192; i++) array_32k[i] = i;
    
    /* Use prefetch with locality hints */
    for (int i = 0; i < 2048; i += 8) {
        __builtin_prefetch(&array_8k[i + 16], 0, 3); /* High temporal locality */
    }
    
    /* Matrix-style access pattern to exercise cache */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            sum += array_16k[i * 64 + j];
        }
    }
    
    /* Another loop with different stride */
    for (int i = 0; i < 8192; i += 64) { /* 64-byte stride for cache lines */
        __builtin_prefetch(&array_32k[i + 128], 1, 2); /* Medium locality, write */
        array_32k[i] *= 2;
    }
}

#else
/* Non-x86 fallback implementations */
void compute_core2(int* data, int size) { (void)data; (void)size; }
void compute_nehalem(int* data, int size) { (void)data; (void)size; }
void compute_sandybridge(int* data, int size) { (void)data; (void)size; }
void compute_ivybridge(int* data, int size) { (void)data; (void)size; }
void compute_dispatch(int* data, int size) { (void)data; (void)size; }
void multi_version_compute(float* data, int size) { (void)data; (void)size; }
void cache_optimized_computation(void) {}
#endif

/* Main function with extensive CPU feature checks */
int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Pattern B: Extensive use of builtins for CPU detection */
    __builtin_cpu_init();
    
    printf("=== CPU Feature Detection ===\n");
    
    /* Check for specific Intel CPUs - these cause driver cache initialization */
    if (__builtin_cpu_is("core2")) {
        printf("CPU: Intel Core2\n");
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
    
    /* Check CPU features that correlate with cache descriptors */
    printf("SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    
    /* Runtime cache line size query */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 Cache Line Size: %ld bytes\n", cache_line);
    
    /* Compile-time assertion for x86 */
    #if defined(__i386__) || defined(__x86_64__)
        /* This should always be true for x86 with GCC */
        _Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
    #endif
    
    /* Create test data */
    int int_data[1024];
    float float_data[1024];
    
    for (int i = 0; i < 1024; i++) {
        int_data[i] = i;
        float_data[i] = i * 0.5f;
    }
    
    /* Call all versions to ensure they're compiled */
    compute_core2(int_data, 1024);
    compute_nehalem(int_data, 1024);
    compute_sandybridge(int_data, 1024);
    compute_ivybridge(int_data, 1024);
    
    /* Use ifunc dispatch */
    compute_dispatch(int_data, 1024);
    
    /* Use multi-versioned function */
    multi_version_compute(float_data, 1024);
    
    /* Perform cache-optimized computation */
    cache_optimized_computation();
    
    /* Compute checksum for verification */
    unsigned long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += int_data[i];
        checksum += (unsigned long)(float_data[i] * 1000);
    }
    
    printf("Checksum: %lu\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
#else
    printf("Non-x86 architecture - skipping CPU cache detection tests\n");
    return 0;
#endif
}
