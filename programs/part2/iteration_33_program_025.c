/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink;

/* Early CPU initialization - forces driver to run CPUID and cache detection */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* This forces the driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query various CPU features to trigger different cache detection paths */
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
static void test_core2_cache(void) {
    volatile char *buffer = malloc(1024 * 1024);
    if (buffer) {
        /* Access with 64-byte stride (typical cache line) */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void test_sandybridge_cache(void) {
    volatile char *buffer = malloc(2 * 1024 * 1024);
    if (buffer) {
        /* Access with 32-byte stride */
        for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void test_haswell_cache(void) {
    volatile char *buffer = malloc(4 * 1024 * 1024);
    if (buffer) {
        /* Mixed stride pattern */
        for (int i = 0; i < 4 * 1024 * 1024; i += 128) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void test_skylake_cache(void) {
    volatile char *buffer = malloc(8 * 1024 * 1024);
    if (buffer) {
        /* Random-like access pattern */
        for (int i = 0; i < 8 * 1024 * 1024; i += 256) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

/* AVX2-optimized memory test to trigger AVX-specific cache paths */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void test_avx2_cache(void) {
    volatile float *buffer = aligned_alloc(32, 1024 * 1024);
    if (buffer) {
        __m256 accum = _mm256_setzero_ps();
        for (int i = 0; i < 1024 * 1024 / 8; i += 8) {
            __m256 data = _mm256_load_ps((float*)&buffer[i]);
            accum = _mm256_add_ps(accum, data);
        }
        sink = _mm256_movemask_ps(accum);
        free((void*)buffer);
    }
}
#pragma GCC pop_options

/* AES-specific test to trigger different cache detection */
#pragma GCC push_options
#pragma GCC target("aes,pclmul")
#include <wmmintrin.h>
static void test_aes_cache(void) {
    volatile char *buffer = malloc(512 * 1024);
    if (buffer) {
        __m128i key = _mm_set1_epi8(0xAA);
        __m128i data = _mm_load_si128((__m128i*)buffer);
        __m128i result = _mm_aesenc_si128(data, key);
        sink = _mm_extract_epi32(result, 0);
        free((void*)buffer);
    }
}
#pragma GCC pop_options

/* Main test function with cache-sensitive timing */
int main(void) {
    unsigned long long start, end;
    int checksum = 0;
    
    /* Call all target-specific functions */
    test_core2_cache();
    test_sandybridge_cache();
    test_haswell_cache();
    test_skylake_cache();
    
    /* Call feature-specific tests if supported */
    if (cpu_features[7]) {  /* AVX2 */
        test_avx2_cache();
    }
    if (cpu_features[9] && cpu_features[10]) {  /* AES + PCLMUL */
        test_aes_cache();
    }
    
    /* Perform cache-size-sensitive memory test */
    volatile char *large_buffer = malloc(16 * 1024 * 1024);
    if (large_buffer) {
        /* Time linear access (cache-friendly) */
        start = __builtin_ia32_rdtsc();
        for (int i = 0; i < 16 * 1024 * 1024; i += 64) {
            sink = large_buffer[i];
        }
        end = __builtin_ia32_rdtsc();
        unsigned long long linear_time = end - start;
        
        /* Time random access (cache-unfriendly) */
        start = __builtin_ia32_rdtsc();
        for (int i = 0; i < 1000000; i++) {
            int idx = (i * 97) % (16 * 1024 * 1024);
            sink = large_buffer[idx];
        }
        end = __builtin_ia32_rdtsc();
        unsigned long long random_time = end - start;
        
        /* Use timing ratio to infer cache behavior */
        if (random_time > 3 * linear_time) {
            checksum |= 1;  /* Likely large cache */
        }
        
        free((void*)large_buffer);
    }
    
    /* Compute final checksum from all CPU feature flags */
    for (int i = 0; i < 16; i++) {
        if (cpu_features[i]) {
            checksum ^= (1 << (i % 16));
        }
    }
    for (int i = 0; i < 8; i++) {
        if (cpu_models[i]) {
            checksum ^= (1 << (16 + (i % 16)));
        }
    }
    
    /* Prevent dead code elimination */
    volatile int result = checksum;
    
    printf("CPU detection test completed. Checksum: %d\n", checksum);
    return 0;
}
