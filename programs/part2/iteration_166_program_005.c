/* test_cache_detection.c
 * This program is designed to trigger GCC driver's CPU cache detection
 * logic for specific Intel cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.)
 * by using multiple CPU feature detection mechanisms and optimization hints.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Multiple functions with different target attributes */
/* Each target may cause GCC to query different cache descriptors */

__attribute__((target("arch=core2")))
void core2_optimized_compute(int *data, int size) {
    /* Use prefetch hints that might encourage cache model usage */
    for (int i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 32], 0, 3); /* Low locality hint */
    }
    
    /* Simple computation */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * 3;
    }
    data[0] = sum;
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int *data, int size) {
    /* Different access pattern */
    for (int i = 0; i < size; i += 8) {
        __builtin_prefetch(&data[i + 16], 1, 2); /* Medium locality hint */
    }
    
    int sum = 0;
    for (int i = 0; i < size; i += 2) {
        sum += data[i] * 5;
    }
    data[1] = sum;
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int *data, int size) {
    /* Another pattern */
    for (int i = 0; i < size; i += 4) {
        __builtin_prefetch(&data[i + 8], 0, 1); /* High locality hint */
    }
    
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * 7;
    }
    data[2] = sum;
}

__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_compute(int *data, int size) {
    /* Yet another pattern */
    for (int i = 0; i < size; i += 32) {
        __builtin_prefetch(&data[i + 64], 1, 0); /* Highest locality hint */
    }
    
    int sum = 0;
    for (int i = size - 1; i >= 0; i--) {
        sum += data[i] * 11;
    }
    data[3] = sum;
}

/* Pattern B: Function with multiple target clones */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=haswell")))
void multiversion_compute(int *data, int size) {
    /* Array sized to match various cache sizes from uncovered lines */
    /* 8KB, 16KB, 32KB, 64KB, 128KB, 256KB, 512KB, 1MB, 2MB, 4MB, 6MB */
    int temp[256]; /* ~1KB - will be accessed in patterns */
    
    for (int i = 0; i < size && i < 256; i++) {
        temp[i] = data[i] * 2;
    }
    
    /* Mix of accesses to encourage cache model usage */
    int result = 0;
    for (int i = 0; i < 256; i += 8) { /* 32-byte stride for 32-byte lines */
        result += temp[i];
    }
    for (int i = 0; i < 256; i += 16) { /* 64-byte stride for 64-byte lines */
        result += temp[i];
    }
    
    data[4] = result;
}

/* Pattern C: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void compute_default(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    data[5] = sum;
}

static void (*resolve_compute(void))(int*, int) {
    /* Force CPU detection through builtins */
    __builtin_cpu_init();
    
    /* Check various CPU features that might correlate with cache descriptors */
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
    
    return compute_default;
}

/* ifunc function - forces CPU detection at load time */
__attribute__((ifunc("resolve_compute")))
void dynamic_compute(int *data, int size);

/* Pattern D: Direct CPUID queries */
static void query_cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (Intel) */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    uint32_t cache_index = 0;
    do {
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(cache_index));
        cache_index++;
    } while ((eax & 0x1f) != 0); /* Continue until cache type field is 0 */
    
    /* Also query leaf 1 for feature bits */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(1));
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_info(void) {
    query_cpuid_cache_info();
    __builtin_cpu_init();
}

/* Compile-time assertion for x86 features */
_Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
               "Expected 32-bit or 64-bit x86");

#endif /* x86 guard */

/* Main function with extensive CPU detection */
int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Pattern B: Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
    /* Check for specific Intel CPUs - each may have different cache descriptors */
    int is_intel = 0;
    
    /* These checks force GCC to populate its CPU cache data structures */
    if (__builtin_cpu_is("intel")) {
        is_intel = 1;
    }
    if (__builtin_cpu_is("core2")) {
        is_intel = 1;
    }
    if (__builtin_cpu_is("nehalem")) {
        is_intel = 1;
    }
    if (__builtin_cpu_is("sandybridge")) {
        is_intel = 1;
    }
    if (__builtin_cpu_is("ivybridge")) {
        is_intel = 1;
    }
    if (__builtin_cpu_is("haswell")) {
        is_intel = 1;
    }
    if (__builtin_cpu_is("broadwell")) {
        is_intel = 1;
    }
    if (__builtin_cpu_is("skylake")) {
        is_intel = 1;
    }
    
    /* Check CPU features that correlate with cache hierarchy */
    int has_sse2 = __builtin_cpu_supports("sse2");
    int has_sse3 = __builtin_cpu_supports("sse3");
    int has_ssse3 = __builtin_cpu_supports("ssse3");
    int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Runtime cache line size query - may cause driver to sync cache model */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    
    /* Create arrays sized to match various cache sizes from uncovered lines */
    /* We'll use multiple arrays to potentially trigger different cache models */
    
    /* 8KB array (matches 0x0a, 0x66 descriptors) */
    int array_8k[2048]; /* 2048 * 4 bytes = 8192 bytes */
    
    /* 16KB array (matches 0x0c, 0x0d, 0x67 descriptors) */
    int array_16k[4096]; /* 4096 * 4 = 16384 bytes */
    
    /* 32KB array (matches 0x2c, 0x68 descriptors) */
    int array_32k[8192]; /* 8192 * 4 = 32768 bytes */
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 2048; i++) array_8k[i] = i & 0xFF;
    for (int i = 0; i < 4096; i++) array_16k[i] = i & 0xFF;
    for (int i = 0; i < 8192; i++) array_32k[i] = i & 0xFF;
    
    /* Call all specialized functions to trigger target attribute processing */
    core2_optimized_compute(array_8k, 2048);
    nehalem_optimized_compute(array_16k, 4096);
    sandybridge_optimized_compute(array_32k, 8192);
    ivybridge_optimized_compute(array_8k, 2048);
    
    /* Call multiversion function */
    multiversion_compute(array_16k, 4096);
    
    /* Call ifunc function */
    dynamic_compute(array_32k, 8192);
    
    /* Pattern D: Cache-sensitive computation with tiling */
    /* Matrix multiplication-like access pattern */
    int result = 0;
    
    /* Try different tile sizes that match cache line sizes */
    for (int tile = 32; tile <= 64; tile *= 2) {
        for (int i = 0; i < 2048; i += tile) {
            int limit = i + tile;
            if (limit > 2048) limit = 2048;
            for (int j = i; j < limit; j++) {
                result += array_8k[j] * (j % 256);
            }
        }
    }
    
    /* Use __builtin_prefetch with different locality hints */
    for (int i = 0; i < 4096; i += cache_line / sizeof(int)) {
        __builtin_prefetch(&array_16k[i], 0, 0); /* Read, highest locality */
        __builtin_prefetch(&array_16k[i + cache_line / sizeof(int)], 1, 3); /* Write, lowest locality */
    }
    
    /* Final computation that uses all arrays */
    int final_sum = 0;
    for (int i = 0; i < 2048; i++) {
        final_sum += array_8k[i];
    }
    for (int i = 0; i < 4096; i += 2) {
        final_sum += array_16k[i];
    }
    for (int i = 0; i < 8192; i += 4) {
        final_sum += array_32k[i];
    }
    
    printf("CPU Detection Test Complete\n");
    printf("Cache line size: %ld bytes\n", cache_line);
    printf("SSE2: %s, SSE3: %s, SSE4.1: %s, AVX: %s\n",
           has_sse2 ? "yes" : "no",
           has_sse3 ? "yes" : "no",
           has_sse4_1 ? "yes" : "no",
           has_avx ? "yes" : "no");
    printf("Result checksum: %d\n", final_sum);
    
    return (final_sum > 0) ? 0 : 1;
#else
    /* Non-x86 fallback */
    printf("This test is designed for x86 systems only.\n");
    return 0;
#endif
}

/* Dummy functions for non-x86 compilation */
#if !defined(__i386__) && !defined(__x86_64__)
void core2_optimized_compute(int *data, int size) { (void)data; (void)size; }
void nehalem_optimized_compute(int *data, int size) { (void)data; (void)size; }
void sandybridge_optimized_compute(int *data, int size) { (void)data; (void)size; }
void ivybridge_optimized_compute(int *data, int size) { (void)data; (void)size; }
void multiversion_compute(int *data, int size) { (void)data; (void)size; }
void dynamic_compute(int *data, int size) { (void)data; (void)size; }
#endif
