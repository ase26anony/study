/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test to trigger x86 CPU cache descriptor detection in GCC driver */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features_detected = 0;
static int cpu_model_matches = 0;
static volatile int anti_opt = 0;

/* Early CPU initialization - runs before main */
static void __attribute__((constructor)) init_cpu(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
    /* Each call may cause the driver to examine CPUID and cache descriptors */
    
    /* Intel-specific features */
    if (__builtin_cpu_supports("avx512f")) cpu_features_detected |= 1;
    if (__builtin_cpu_supports("avx512dq")) cpu_features_detected |= 2;
    if (__builtin_cpu_supports("avx512cd")) cpu_features_detected |= 4;
    if (__builtin_cpu_supports("avx512bw")) cpu_features_detected |= 8;
    if (__builtin_cpu_supports("avx512vl")) cpu_features_detected |= 16;
    
    if (__builtin_cpu_supports("avx2")) cpu_features_detected |= 32;
    if (__builtin_cpu_supports("avx")) cpu_features_detected |= 64;
    if (__builtin_cpu_supports("sse4.2")) cpu_features_detected |= 128;
    if (__builtin_cpu_supports("sse4.1")) cpu_features_detected |= 256;
    if (__builtin_cpu_supports("ssse3")) cpu_features_detected |= 512;
    
    if (__builtin_cpu_supports("sse3")) cpu_features_detected |= 1024;
    if (__builtin_cpu_supports("sse2")) cpu_features_detected |= 2048;
    if (__builtin_cpu_supports("sse")) cpu_features_detected |= 4096;
    
    /* Additional features that might be associated with specific cache configs */
    if (__builtin_cpu_supports("aes")) cpu_features_detected |= 8192;
    if (__builtin_cpu_supports("pclmul")) cpu_features_detected |= 16384;
    if (__builtin_cpu_supports("rdrand")) cpu_features_detected |= 32768;
    if (__builtin_cpu_supports("rdseed")) cpu_features_detected |= 65536;
    if (__builtin_cpu_supports("sha")) cpu_features_detected |= 131072;
    
    if (__builtin_cpu_supports("fma")) cpu_features_detected |= 262144;
    if (__builtin_cpu_supports("f16c")) cpu_features_detected |= 524288;
    if (__builtin_cpu_supports("bmi2")) cpu_features_detected |= 1048576;
    if (__builtin_cpu_supports("adx")) cpu_features_detected |= 2097152;
    
    /* Check various CPU models - each may have different cache configurations */
    /* This forces the driver to examine CPUID information thoroughly */
    if (__builtin_cpu_is("intel")) cpu_model_matches |= 1;
    if (__builtin_cpu_is("amd")) cpu_model_matches |= 2;
    if (__builtin_cpu_is("atom")) cpu_model_matches |= 4;
    if (__builtin_cpu_is("core2")) cpu_model_matches |= 8;
    if (__builtin_cpu_is("nehalem")) cpu_model_matches |= 16;
    if (__builtin_cpu_is("sandybridge")) cpu_model_matches |= 32;
    if (__builtin_cpu_is("ivybridge")) cpu_model_matches |= 64;
    if (__builtin_cpu_is("haswell")) cpu_model_matches |= 128;
    if (__builtin_cpu_is("broadwell")) cpu_model_matches |= 256;
    if (__builtin_cpu_is("skylake")) cpu_model_matches |= 512;
    if (__builtin_cpu_is("cannonlake")) cpu_model_matches |= 1024;
    if (__builtin_cpu_is("icelake")) cpu_model_matches |= 2048;
    if (__builtin_cpu_is("tigerlake")) cpu_model_matches |= 4096;
    if (__builtin_cpu_is("alderlake")) cpu_model_matches |= 8192;
    
    if (__builtin_cpu_is("amdfam10")) cpu_model_matches |= 16384;
    if (__builtin_cpu_is("barcelona")) cpu_model_matches |= 32768;
    if (__builtin_cpu_is("shanghai")) cpu_model_matches |= 65536;
    if (__builtin_cpu_is("istanbul")) cpu_model_matches |= 131072;
    if (__builtin_cpu_is("bulldozer")) cpu_model_matches |= 262144;
    if (__builtin_cpu_is("piledriver")) cpu_model_matches |= 524288;
    if (__builtin_cpu_is("zen")) cpu_model_matches |= 1048576;
    if (__builtin_cpu_is("zen2")) cpu_model_matches |= 2097152;
    if (__builtin_cpu_is("zen3")) cpu_model_matches |= 4194304;
}

/* Target-specific functions to force consideration of different microarchitectures */
static void __attribute__((target("arch=core2"))) 
core2_cache_test(void) {
    /* This function will be compiled with Core2 target, potentially 
       triggering cache descriptor 0x66 (L1: 8KB, 4-way, 64B line) */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i;
    }
    anti_opt += sum;
}

static void __attribute__((target("arch=sandybridge"))) 
sandybridge_cache_test(void) {
    /* Sandy Bridge might trigger various cache descriptors */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * 2;
    }
    anti_opt += sum;
}

static void __attribute__((target("arch=haswell"))) 
haswell_cache_test(void) {
    /* Haswell architecture */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * 3;
    }
    anti_opt += sum;
}

static void __attribute__((target("arch=skylake"))) 
skylake_cache_test(void) {
    /* Skylake architecture */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * 4;
    }
    anti_opt += sum;
}

static void __attribute__((target("arch=znver1"))) 
zen_cache_test(void) {
    /* AMD Zen architecture */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * 5;
    }
    anti_opt += sum;
}

/* Cache-sensitive memory access patterns */
static void cache_sensitive_access(void) {
    /* Use a large buffer that exceeds typical L2 cache sizes */
    const size_t buffer_size = 4 * 1024 * 1024; /* 4 MB */
    char *buffer = (char*)malloc(buffer_size);
    
    if (!buffer) return;
    
    /* Initialize buffer */
    memset(buffer, 1, buffer_size);
    
    /* Access with different strides to be sensitive to cache line size */
    volatile int sum = 0;
    
    /* Stride = 32 bytes (potential cache line size) */
    for (size_t i = 0; i < buffer_size; i += 32) {
        sum += buffer[i];
    }
    
    /* Stride = 64 bytes (typical cache line size) */
    for (size_t i = 0; i < buffer_size; i += 64) {
        sum += buffer[i];
    }
    
    /* Random access pattern to defeat prefetching */
    for (int j = 0; j < 10000; j++) {
        size_t idx = (j * 97) % buffer_size; /* Simple pseudo-random */
        sum += buffer[idx];
    }
    
    anti_opt += sum;
    free(buffer);
}

/* Use pragmas to target different instruction sets */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    /* AVX2 code that might trigger different optimization paths */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * 6;
    }
    anti_opt += sum;
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    /* AVX-512 code */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * 7;
    }
    anti_opt += sum;
}
#pragma GCC pop_options

int main(void) {
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    
    /* Call instruction-set specific functions */
    avx2_cache_test();
    avx512_cache_test();
    
    /* Perform cache-sensitive memory access */
    cache_sensitive_access();
    
    /* Use CPU feature detection results to create output checksum */
    /* This prevents dead code elimination */
    int result = cpu_features_detected ^ cpu_model_matches;
    result += anti_opt;
    
    /* Print something to ensure the program runs */
    printf("CPU features: 0x%x, CPU models: 0x%x, Result: %d\n", 
           cpu_features_detected, cpu_model_matches, result);
    
    return result == 0 ? 0 : 1;
}
