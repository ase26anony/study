/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[64];
static int cpu_models[16];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - mimics driver startup */
__attribute__((constructor(101)))
static void init_cpu_detection(void) {
    /* Force driver to initialize CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache descriptor parsing */
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
    cpu_features[10] = __builtin_cpu_supports("avx512vl");
    cpu_features[11] = __builtin_cpu_supports("avx512bw");
    cpu_features[12] = __builtin_cpu_supports("avx512dq");
    cpu_features[13] = __builtin_cpu_supports("fma");
    cpu_features[14] = __builtin_cpu_supports("aes");
    cpu_features[15] = __builtin_cpu_supports("pclmul");
    cpu_features[16] = __builtin_cpu_supports("rdrand");
    cpu_features[17] = __builtin_cpu_supports("rdseed");
    cpu_features[18] = __builtin_cpu_supports("sha");
    cpu_features[19] = __builtin_cpu_supports("xsave");
    cpu_features[20] = __builtin_cpu_supports("xsavec");
    cpu_features[21] = __builtin_cpu_supports("xsaves");
    cpu_features[22] = __builtin_cpu_supports("osxsave");
    
    /* Query CPU models to trigger different cache configuration paths */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("atom");
    cpu_models[3] = __builtin_cpu_is("core2");
    cpu_models[4] = __builtin_cpu_is("nehalem");
    cpu_models[5] = __builtin_cpu_is("sandybridge");
    cpu_models[6] = __builtin_cpu_is("ivybridge");
    cpu_models[7] = __builtin_cpu_is("haswell");
    cpu_models[8] = __builtin_cpu_is("broadwell");
    cpu_models[9] = __builtin_cpu_is("skylake");
    cpu_models[10] = __builtin_cpu_is("cannonlake");
    cpu_models[11] = __builtin_cpu_is("icelake");
    cpu_models[12] = __builtin_cpu_is("tigerlake");
    cpu_models[13] = __builtin_cpu_is("alderlake");
    cpu_models[14] = __builtin_cpu_is("znver1");
    cpu_models[15] = __builtin_cpu_is("znver2");
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void test_cache_core2(void) {
    /* Access pattern sensitive to 32-byte cache lines */
    volatile char buffer[32768]; /* 32KB */
    for (int i = 0; i < sizeof(buffer); i += 32) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=sandybridge")))
static void test_cache_sandybridge(void) {
    /* Access pattern sensitive to 64-byte cache lines */
    volatile char buffer[65536]; /* 64KB */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=haswell")))
static void test_cache_haswell(void) {
    /* Larger access pattern for L2 cache */
    volatile char buffer[262144]; /* 256KB */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=skylake")))
static void test_cache_skylake(void) {
    /* Even larger pattern */
    volatile char buffer[1048576]; /* 1MB */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

/* AVX2-optimized function to trigger feature-specific cache logic */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void test_avx2_cache(void) {
    __m256i data[1024]; /* 32KB with 256-bit vectors */
    __m256i sum = _mm256_setzero_si256();
    
    for (int i = 0; i < 1024; i++) {
        sum = _mm256_add_epi32(sum, data[i]);
    }
    
    volatile __m256i result = sum;
    (void)result;
}
#pragma GCC pop_options

/* AVX512-optimized function */
#pragma GCC push_options
#pragma GCC target("avx512f")
static void test_avx512_cache(void) {
    __m512i data[512]; /* 32KB with 512-bit vectors */
    __m512i sum = _mm512_setzero_si512();
    
    for (int i = 0; i < 512; i++) {
        sum = _mm512_add_epi32(sum, data[i]);
    }
    
    volatile __m512i result = sum;
    (void)result;
}
#pragma GCC pop_options

/* Cache size estimation via timing */
static uint64_t estimate_cache_size(void) {
    const size_t max_size = 8 * 1024 * 1024; /* 8MB */
    volatile char *buffer = malloc(max_size);
    uint64_t start, end;
    
    if (!buffer) return 0;
    
    /* Touch all pages first */
    for (size_t i = 0; i < max_size; i += 4096) {
        buffer[i] = 0;
    }
    
    /* Time sequential access with different strides */
    uint64_t times[4] = {0};
    
    for (int stride_idx = 0; stride_idx < 4; stride_idx++) {
        size_t stride = (stride_idx == 0) ? 32 : 
                       (stride_idx == 1) ? 64 : 
                       (stride_idx == 2) ? 128 : 256;
        
        start = __builtin_ia32_rdtsc();
        for (size_t i = 0; i < max_size; i += stride) {
            sink = buffer[i];
        }
        end = __builtin_ia32_rdtsc();
        times[stride_idx] = end - start;
    }
    
    free((void*)buffer);
    
    /* Return a checksum based on timing ratios */
    return (times[0] << 48) | (times[1] << 32) | (times[2] << 16) | times[3];
}

/* Main test function */
int main(void) {
    uint64_t cache_timing = 0;
    int checksum = 0;
    
    /* Call all target-specific functions */
    test_cache_core2();
    test_cache_sandybridge();
    test_cache_haswell();
    test_cache_skylake();
    
    /* Call SIMD functions if supported */
    if (cpu_features[7]) { /* AVX */
        test_avx2_cache();
    }
    if (cpu_features[9]) { /* AVX512F */
        test_avx512_cache();
    }
    
    /* Perform cache timing estimation */
    cache_timing = estimate_cache_size();
    
    /* Compute checksum from CPU features and timing */
    for (int i = 0; i < 23; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    for (int i = 0; i < 16; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    checksum ^= (cache_timing >> 32) ^ (cache_timing & 0xFFFFFFFF);
    
    /* Output result (prevents dead code elimination) */
    printf("CPU detection checksum: 0x%08x\n", checksum);
    printf("Cache timing signature: 0x%016llx\n", (unsigned long long)cache_timing);
    
    /* Additional feature queries to ensure driver paths are taken */
    if (__builtin_cpu_supports("cmov")) checksum++;
    if (__builtin_cpu_supports("cx8")) checksum++;
    if (__builtin_cpu_supports("fxsr")) checksum++;
    if (__builtin_cpu_supports("mmx")) checksum++;
    if (__builtin_cpu_supports("syscall")) checksum++;
    if (__builtin_cpu_supports("sse")) checksum++;
    if (__builtin_cpu_supports("sse2")) checksum++;
    if (__builtin_cpu_supports("sse3")) checksum++;
    if (__builtin_cpu_supports("ssse3")) checksum++;
    if (__builtin_cpu_supports("sse4.1")) checksum++;
    if (__builtin_cpu_supports("sse4.2")) checksum++;
    if (__builtin_cpu_supports("popcnt")) checksum++;
    if (__builtin_cpu_supports("abm")) checksum++;
    if (__builtin_cpu_supports("lzcnt")) checksum++;
    if (__builtin_cpu_supports("bmi")) checksum++;
    if (__builtin_cpu_supports("bmi2")) checksum++;
    if (__builtin_cpu_supports("tbm")) checksum++;
    if (__builtin_cpu_supports("avx")) checksum++;
    if (__builtin_cpu_supports("avx2")) checksum++;
    if (__builtin_cpu_supports("f16c")) checksum++;
    if (__builtin_cpu_supports("fma")) checksum++;
    if (__builtin_cpu_supports("fma4")) checksum++;
    if (__builtin_cpu_supports("xop")) checksum++;
    
    return checksum & 0xFF;
}
