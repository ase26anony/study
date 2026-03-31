/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - forces driver cache detection */
__attribute__((constructor))
static void init_cpu(void) {
    /* This forces the driver to execute CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query various CPU features - each may trigger cache descriptor lookups */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("aes");
    cpu_features[10] = __builtin_cpu_supports("pclmul");
    cpu_features[11] = __builtin_cpu_supports("rdrand");
    cpu_features[12] = __builtin_cpu_supports("rdseed");
    cpu_features[13] = __builtin_cpu_supports("sha");
    cpu_features[14] = __builtin_cpu_supports("fma");
    cpu_features[15] = __builtin_cpu_supports("f16c");
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *array = malloc(256 * 1024); /* 256KB - typical L2 size */
    if (array) {
        for (int i = 0; i < 256 * 1024; i += 64) { /* 64-byte stride */
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB */
    if (array) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *array = malloc(2048 * 1024); /* 2MB */
    if (array) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(4096 * 1024); /* 4MB */
    if (array) {
        for (int i = 0; i < 4096 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

/* Cache size detection through timing */
static int detect_cache_sensitivity(void) {
    const size_t max_size = 16 * 1024 * 1024; /* 16MB */
    volatile char *array = malloc(max_size);
    uint64_t times[4] = {0};
    
    if (!array) return 0;
    
    /* Warm up */
    for (size_t i = 0; i < max_size; i += 64) {
        sink = array[i];
    }
    
    /* Time different access patterns that may trigger cache queries */
    for (int pattern = 0; pattern < 4; pattern++) {
        uint64_t start = __builtin_ia32_rdtsc();
        
        switch (pattern) {
            case 0: /* Sequential 32-byte stride */
                for (size_t i = 0; i < max_size; i += 32) {
                    sink = array[i];
                }
                break;
            case 1: /* Sequential 64-byte stride */
                for (size_t i = 0; i < max_size; i += 64) {
                    sink = array[i];
                }
                break;
            case 2: /* Random access within L1 range */
                for (int i = 0; i < 10000; i++) {
                    sink = array[(i * 13) & 0x3FFF]; /* 16KB range */
                }
                break;
            case 3: /* Random access within L2 range */
                for (int i = 0; i < 10000; i++) {
                    sink = array[(i * 17) & 0x7FFFF]; /* 512KB range */
                }
                break;
        }
        
        uint64_t end = __builtin_ia32_rdtsc();
        times[pattern] = end - start;
    }
    
    free((void*)array);
    
    /* Return a hash of timing ratios - forces compiler to keep all code */
    return (int)((times[1] * 1000 / (times[0] + 1)) ^ 
                 (times[3] * 1000 / (times[2] + 1)));
}

/* AVX-optimized function to trigger AVX-specific cache paths */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void avx2_cache_test(void) {
    volatile __m256i *array = malloc(1024 * sizeof(__m256i));
    if (array) {
        __m256i sum = _mm256_setzero_si256();
        for (int i = 0; i < 1024; i++) {
            sum = _mm256_add_epi32(sum, array[i]);
        }
        sink = _mm256_extract_epi32(sum, 0);
        free((void*)array);
    }
}
#pragma GCC pop_options

int main(void) {
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    avx2_cache_test();
    
    /* Use CPU feature flags to create execution branches */
    if (cpu_features[0]) checksum |= 1;   /* SSE */
    if (cpu_features[1]) checksum |= 2;   /* SSE2 */
    if (cpu_features[6]) checksum |= 4;   /* AVX */
    if (cpu_features[7]) checksum |= 8;   /* AVX2 */
    
    /* Use CPU model flags */
    if (cpu_models[0]) checksum |= 0x10;  /* Intel */
    if (cpu_models[1]) checksum |= 0x20;  /* AMD */
    if (cpu_models[4]) checksum |= 0x40;  /* Sandy Bridge */
    
    /* Perform cache-sensitive timing */
    int timing_hash = detect_cache_sensitivity();
    checksum ^= timing_hash;
    
    /* Force compiler to keep all feature detection code */
    for (int i = 0; i < 16; i++) {
        if (cpu_features[i]) checksum += i;
    }
    
    printf("CPU checksum: %d\n", checksum);
    
    /* Additional CPUID-like queries to hit more driver paths */
    if (__builtin_cpu_supports("mmx")) checksum++;
    if (__builtin_cpu_supports("3dnow")) checksum++;
    if (__builtin_cpu_supports("popcnt")) checksum++;
    if (__builtin_cpu_supports("bmi")) checksum++;
    if (__builtin_cpu_supports("bmi2")) checksum++;
    if (__builtin_cpu_supports("lzcnt")) checksum++;
    
    return checksum & 0xFF; /* Return non-zero to ensure execution */
}
