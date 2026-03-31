/* { dg-do run { target i?86-*-* x86_64-*-* } } */
/* { dg-options "-O2 -march=native -mtune=generic" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[64];
static int cpu_models[16];
static volatile int timing_results[8];

/* Early CPU initialization - runs before main() */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("avx512vl");
    cpu_features[10] = __builtin_cpu_supports("avx512bw");
    cpu_features[11] = __builtin_cpu_supports("avx512dq");
    cpu_features[12] = __builtin_cpu_supports("aes");
    cpu_features[13] = __builtin_cpu_supports("pclmul");
    cpu_features[14] = __builtin_cpu_supports("rdrand");
    cpu_features[15] = __builtin_cpu_supports("rdseed");
    cpu_features[16] = __builtin_cpu_supports("sha");
    cpu_features[17] = __builtin_cpu_supports("fma");
    cpu_features[18] = __builtin_cpu_supports("fma4");
    cpu_features[19] = __builtin_cpu_supports("xop");
    cpu_features[20] = __builtin_cpu_supports("bmi");
    cpu_features[21] = __builtin_cpu_supports("bmi2");
    cpu_features[22] = __builtin_cpu_supports("adx");
    cpu_features[23] = __builtin_cpu_supports("clflushopt");
    cpu_features[24] = __builtin_cpu_supports("clwb");
    cpu_features[25] = __builtin_cpu_supports("fsgsbase");
    cpu_features[26] = __builtin_cpu_supports("invpcid");
    cpu_features[27] = __builtin_cpu_supports("mwaitx");
    cpu_features[28] = __builtin_cpu_supports("movbe");
    cpu_features[29] = __builtin_cpu_supports("pku");
    cpu_features[30] = __builtin_cpu_supports("prefetchwt1");
    cpu_features[31] = __builtin_cpu_supports("rtm");
    cpu_features[32] = __builtin_cpu_supports("sgx");
    cpu_features[33] = __builtin_cpu_supports("shstk");
    cpu_features[34] = __builtin_cpu_supports("tbm");
    cpu_features[35] = __builtin_cpu_supports("vaes");
    cpu_features[36] = __builtin_cpu_supports("vpclmulqdq");
    cpu_features[37] = __builtin_cpu_supports("waitpkg");
    cpu_features[38] = __builtin_cpu_supports("wbnoinvd");
    cpu_features[39] = __builtin_cpu_supports("xsave");
    cpu_features[40] = __builtin_cpu_supports("xsavec");
    cpu_features[41] = __builtin_cpu_supports("xsaveopt");
    cpu_features[42] = __builtin_cpu_supports("xsaves");
    
    /* Query CPU models to trigger different cache descriptor paths */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("atom");
    cpu_models[3] = __builtin_cpu_is("core2");
    cpu_models[4] = __builtin_cpu_is("nehalem");
    cpu_models[5] = __builtin_cpu_is("sandybridge");
    cpu_models[6] = __builtin_cpu_is("haswell");
    cpu_models[7] = __builtin_cpu_is("skylake");
    cpu_models[8] = __builtin_cpu_is("cannonlake");
    cpu_models[9] = __builtin_cpu_is("icelake");
    cpu_models[10] = __builtin_cpu_is("tigerlake");
    cpu_models[11] = __builtin_cpu_is("alderlake");
    cpu_models[12] = __builtin_cpu_is("znver1");
    cpu_models[13] = __builtin_cpu_is("znver2");
    cpu_models[14] = __builtin_cpu_is("znver3");
    cpu_models[15] = __builtin_cpu_is("znver4");
}

/* Cache-sensitive memory access patterns */
static void cache_sensitive_test(void) {
    /* Large array that exceeds typical L2 cache */
    volatile char *buffer = malloc(4 * 1024 * 1024); /* 4MB */
    uint64_t start, end;
    
    if (!buffer) return;
    
    /* Test with 32-byte stride (typical cache line) */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < 4 * 1024 * 1024; i += 32) {
        buffer[i] = i & 0xFF;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[0] = (int)(end - start);
    
    /* Test with 64-byte stride (another common cache line) */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < 4 * 1024 * 1024; i += 64) {
        buffer[i] = i & 0xFF;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[1] = (int)(end - start);
    
    /* Random access pattern to stress cache */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < 100000; i++) {
        int idx = (i * 97) & (4 * 1024 * 1024 - 1);
        buffer[idx] = i & 0xFF;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[2] = (int)(end - start);
    
    free((void*)buffer);
}

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static void core2_specific_test(void) {
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * cpu_features[i % 43];
    }
    timing_results[3] = sum;
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_specific_test(void) {
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * cpu_models[i % 16];
    }
    timing_results[4] = sum;
}

__attribute__((target("arch=haswell")))
static void haswell_specific_test(void) {
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += (i << 2) | cpu_features[i % 43];
    }
    timing_results[5] = sum;
}

__attribute__((target("arch=skylake")))
static void skylake_specific_test(void) {
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += (i >> 2) ^ cpu_models[i % 16];
    }
    timing_results[6] = sum;
}

__attribute__((target("arch=znver2")))
static void zen2_specific_test(void) {
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i + cpu_features[i % 43] + cpu_models[i % 16];
    }
    timing_results[7] = sum;
}

/* Vectorized operations with different ISA extensions */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_test(void) {
    volatile int result = 0;
    for (int i = 0; i < 1000; i++) {
        result += cpu_features[i % 43] * 2;
    }
    /* Use result to prevent optimization */
    if (result > 0) {
        timing_results[3] += result;
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_test(void) {
    volatile int result = 0;
    for (int i = 0; i < 1000; i++) {
        result += cpu_models[i % 16] * 3;
    }
    /* Use result to prevent optimization */
    if (result > 0) {
        timing_results[4] += result;
    }
}
#pragma GCC pop_options

int main(void) {
    /* Execute all tests to trigger driver code paths */
    cache_sensitive_test();
    
    /* Call target-specific functions */
    core2_specific_test();
    sandybridge_specific_test();
    haswell_specific_test();
    skylake_specific_test();
    zen2_specific_test();
    
    /* Call vectorized functions if supported */
    if (cpu_features[6]) { /* AVX */
        avx2_test();
    }
    if (cpu_features[8]) { /* AVX512F */
        avx512_test();
    }
    
    /* Compute checksum from all results to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum ^= timing_results[i];
    }
    for (int i = 0; i < 43; i++) {
        checksum += cpu_features[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += cpu_models[i];
    }
    
    /* Output minimal result to satisfy test framework */
    printf("CPU test completed with checksum: %d\n", checksum);
    
    return 0;
}
