/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - forces driver to run cache detection */
__attribute__((constructor))
static void init_cpu(void) {
    /* This triggers the driver's CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query all possible CPU features to maximize driver execution */
    cpu_features[0] = __builtin_cpu_supports("cmov");
    cpu_features[1] = __builtin_cpu_supports("mmx");
    cpu_features[2] = __builtin_cpu_supports("sse");
    cpu_features[3] = __builtin_cpu_supports("sse2");
    cpu_features[4] = __builtin_cpu_supports("sse3");
    cpu_features[5] = __builtin_cpu_supports("ssse3");
    cpu_features[6] = __builtin_cpu_supports("sse4.1");
    cpu_features[7] = __builtin_cpu_supports("sse4.2");
    cpu_features[8] = __builtin_cpu_supports("avx");
    cpu_features[9] = __builtin_cpu_supports("avx2");
    cpu_features[10] = __builtin_cpu_supports("avx512f");
    cpu_features[11] = __builtin_cpu_supports("fma");
    cpu_features[12] = __builtin_cpu_supports("aes");
    cpu_features[13] = __builtin_cpu_supports("pclmul");
    cpu_features[14] = __builtin_cpu_supports("rdrand");
    cpu_features[15] = __builtin_cpu_supports("rdseed");
    cpu_features[16] = __builtin_cpu_supports("sha");
    cpu_features[17] = __builtin_cpu_supports("xsave");
    cpu_features[18] = __builtin_cpu_supports("xsaveopt");
    cpu_features[19] = __builtin_cpu_supports("xsavec");
    cpu_features[20] = __builtin_cpu_supports("xsaves");
    
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
    /* Memory access pattern sensitive to cache parameters */
    volatile char buffer[32768]; /* 32KB - typical L1 size */
    for (int i = 0; i < sizeof(buffer); i += 64) { /* 64-byte line */
        sink = buffer[i];
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char buffer[262144]; /* 256KB - typical L2 size */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char buffer[1048576]; /* 1MB - typical L3 slice */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char buffer[2097152]; /* 2MB */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

/* AVX2 version to trigger different optimization paths */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void avx2_cache_test(void) {
    /* Use AVX2 instructions that might be cache-sensitive */
    volatile __m256i data[1024];
    __m256i sum = _mm256_setzero_si256();
    
    for (int i = 0; i < 1024; i++) {
        sum = _mm256_add_epi32(sum, data[i]);
    }
    
    volatile int result[8];
    _mm256_storeu_si256((__m256i*)result, sum);
    sink = result[0];
}
#pragma GCC pop_options

/* SSE4.2 version */
#pragma GCC push_options
#pragma GCC target("sse4.2")
#include <nmmintrin.h>
static void sse42_cache_test(void) {
    volatile __m128i data[2048];
    __m128i sum = _mm_setzero_si128();
    
    for (int i = 0; i < 2048; i++) {
        sum = _mm_add_epi32(sum, data[i]);
    }
    
    volatile int result[4];
    _mm_storeu_si128((__m128i*)result, sum);
    sink = result[0];
}
#pragma GCC pop_options

/* Cache size estimation via timing */
static uint64_t estimate_cache_size(void) {
    /* Large array that exceeds typical L3 cache */
    static volatile char probe_array[8 * 1024 * 1024]; /* 8MB */
    const int iterations = 100;
    uint64_t start, end;
    
    /* Time sequential access with different strides */
    uint64_t times[4] = {0};
    
    for (int stride_idx = 0; stride_idx < 4; stride_idx++) {
        int stride = (stride_idx == 0) ? 32 : 
                    (stride_idx == 1) ? 64 : 
                    (stride_idx == 2) ? 128 : 256;
        
        start = __builtin_ia32_rdtsc();
        for (int iter = 0; iter < iterations; iter++) {
            for (int i = 0; i < sizeof(probe_array); i += stride) {
                sink = probe_array[i];
            }
        }
        end = __builtin_ia32_rdtsc();
        times[stride_idx] = end - start;
    }
    
    /* Return a hash of timing results */
    return times[0] ^ times[1] ^ times[2] ^ times[3];
}

int main(void) {
    uint64_t cache_timing_hash = 0;
    
    /* Execute all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    
    /* Execute instruction-set-specific tests if supported */
    if (cpu_features[8]) { /* AVX */
        avx2_cache_test();
    }
    if (cpu_features[7]) { /* SSE4.2 */
        sse42_cache_test();
    }
    
    /* Perform cache timing analysis */
    cache_timing_hash = estimate_cache_size();
    
    /* Compute checksum from all detected features and timing */
    int checksum = 0;
    for (int i = 0; i < 21; i++) {
        checksum = (checksum << 1) | cpu_features[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum = (checksum << 1) | cpu_models[i];
    }
    checksum ^= (cache_timing_hash & 0xFFFFFFFF);
    
    /* Output result to prevent dead code elimination */
    printf("CPU detection checksum: 0x%08x\n", checksum);
    printf("Cache timing hash: 0x%016llx\n", 
           (unsigned long long)cache_timing_hash);
    
    /* Force compiler to consider all code paths */
    if (checksum == 0x12345678) { /* Unlikely constant */
        printf("Impossible branch taken!\n");
    }
    
    return 0;
}
