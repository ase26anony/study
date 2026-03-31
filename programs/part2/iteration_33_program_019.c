/* { dg-do run { target i?86-*-* x86_64-*-* } } */
/* { dg-options "-O2 -march=native -mtune=generic" } */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink;

/* Early CPU initialization - runs before main */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide range of CPU features to trigger cache detection */
    cpu_features[0] = __builtin_cpu_supports("mmx");
    cpu_features[1] = __builtin_cpu_supports("sse");
    cpu_features[2] = __builtin_cpu_supports("sse2");
    cpu_features[3] = __builtin_cpu_supports("sse3");
    cpu_features[4] = __builtin_cpu_supports("ssse3");
    cpu_features[5] = __builtin_cpu_supports("sse4.1");
    cpu_features[6] = __builtin_cpu_supports("sse4.2");
    cpu_features[7] = __builtin_cpu_supports("avx");
    cpu_features[8] = __builtin_cpu_supports("avx2");
    cpu_features[9] = __builtin_cpu_supports("avx512f");
    cpu_features[10] = __builtin_cpu_supports("fma");
    cpu_features[11] = __builtin_cpu_supports("aes");
    cpu_features[12] = __builtin_cpu_supports("pclmul");
    cpu_features[13] = __builtin_cpu_supports("rdrand");
    cpu_features[14] = __builtin_cpu_supports("rdseed");
    cpu_features[15] = __builtin_cpu_supports("sha");
    cpu_features[16] = __builtin_cpu_supports("xsave");
    cpu_features[17] = __builtin_cpu_supports("xsaveopt");
    cpu_features[18] = __builtin_cpu_supports("xsavec");
    cpu_features[19] = __builtin_cpu_supports("xsaves");
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");  /* AMD Zen */
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void test_cache_core2(void) {
    volatile char *buffer = malloc(256 * 1024);  /* 256KB - typical L2 for Core2 */
    if (buffer) {
        for (int i = 0; i < 256 * 1024; i += 64) {  /* 64-byte stride */
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void test_cache_sandybridge(void) {
    volatile char *buffer = malloc(1024 * 1024);  /* 1MB - typical L3 for Sandy Bridge */
    if (buffer) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void test_cache_haswell(void) {
    volatile char *buffer = malloc(2048 * 1024);  /* 2MB - typical L3 for Haswell */
    if (buffer) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void test_cache_skylake(void) {
    volatile char *buffer = malloc(8192 * 1024);  /* 8MB - typical L3 for Skylake */
    if (buffer) {
        for (int i = 0; i < 8192 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

/* AVX2-optimized memory test to trigger feature-specific paths */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void test_avx2_cache(void) {
    /* Allocate memory aligned to 32 bytes for AVX2 */
    volatile char *buffer = aligned_alloc(32, 1024 * 1024);
    if (buffer) {
        /* Use AVX2 instructions that might trigger cache optimization */
        __m256i zero = _mm256_setzero_si256();
        __m256i *vec_buffer = (__m256i*)buffer;
        
        for (int i = 0; i < (1024 * 1024) / 32; i++) {
            vec_buffer[i] = zero;
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

/* SSE4.2-optimized memory test */
#pragma GCC push_options
#pragma GCC target("sse4.2")
#include <nmmintrin.h>
static void test_sse42_cache(void) {
    volatile char *buffer = aligned_alloc(16, 512 * 1024);
    if (buffer) {
        __m128i zero = _mm_setzero_si128();
        __m128i *vec_buffer = (__m128i*)buffer;
        
        for (int i = 0; i < (512 * 1024) / 16; i++) {
            vec_buffer[i] = zero;
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

/* Cache size detection through timing */
static uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

static void measure_cache_access(void) {
    const size_t max_size = 32 * 1024 * 1024;  /* 32MB */
    volatile char *buffer = malloc(max_size);
    uint64_t times[5] = {0};
    
    if (!buffer) return;
    
    /* Warm up cache */
    for (size_t i = 0; i < max_size; i += 64) {
        sink = buffer[i];
    }
    
    /* Measure different access patterns that might trigger cache queries */
    for (int pattern = 0; pattern < 5; pattern++) {
        uint64_t start = rdtsc();
        
        switch (pattern) {
            case 0: /* Sequential access - cache line size 32 */
                for (size_t i = 0; i < max_size; i += 32) {
                    sink = buffer[i];
                }
                break;
            case 1: /* Sequential access - cache line size 64 */
                for (size_t i = 0; i < max_size; i += 64) {
                    sink = buffer[i];
                }
                break;
            case 2: /* Random access within L1 range */
                for (int i = 0; i < 10000; i++) {
                    sink = buffer[(i * 97) & 0x3FFF];  /* 16KB range */
                }
                break;
            case 3: /* Random access within L2 range */
                for (int i = 0; i < 10000; i++) {
                    sink = buffer[(i * 97) & 0x3FFFF];  /* 256KB range */
                }
                break;
            case 4: /* Random access within L3 range */
                for (int i = 0; i < 10000; i++) {
                    sink = buffer[(i * 97) & 0x1FFFFF];  /* 2MB range */
                }
                break;
        }
        
        uint64_t end = rdtsc();
        times[pattern] = end - start;
    }
    
    /* Use timing results to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum ^= times[i];
    }
    sink = checksum;
    
    free((void*)buffer);
}

int main(void) {
    int result = 0;
    
    /* Call all target-specific functions */
    test_cache_core2();
    test_cache_sandybridge();
    test_cache_haswell();
    test_cache_skylake();
    
    /* Call feature-specific functions if supported */
    if (cpu_features[7]) {  /* AVX */
        test_avx2_cache();
    }
    if (cpu_features[6]) {  /* SSE4.2 */
        test_sse42_cache();
    }
    
    /* Perform cache timing measurements */
    measure_cache_access();
    
    /* Compute final result based on CPU features to prevent optimization */
    for (int i = 0; i < 20; i++) {
        result ^= cpu_features[i] << i;
    }
    for (int i = 0; i < 8; i++) {
        result ^= cpu_models[i] << (20 + i);
    }
    
    /* Print something to ensure the program runs */
    printf("CPU test completed with result: %d\n", result);
    
    return 0;
}
