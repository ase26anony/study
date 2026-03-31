/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */
/* This forces execution of the switch cases for CPUID cache descriptor bytes */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Global variables to store CPU features - prevents optimization */
volatile int cpu_features[256];
volatile int cpu_models[32];
volatile int timing_results[8];

/* Early initialization to force driver cache detection */
__attribute__((constructor(101)))
static void init_cpu_detection(void) {
    /* This forces __builtin_cpu_init which triggers driver cache detection */
    __builtin_cpu_init();
    
    /* Query every possible CPU feature to maximize cache descriptor processing */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "avx512bf16", "avx512fp16",
        "fma", "fma4", "xop", "aes", "pclmul", "sha", "gfni",
        "vaes", "vpclmulqdq", "movbe", "rdrnd", "rdseed", "adx",
        "bmi", "bmi2", "lzcnt", "fxsr", "xsave", "xsaveopt",
        "xsavec", "xsaves", "mwaitx", "clzero", "pku", "ospke",
        "rdpid", "sgx", "cldemote", "ptwrite", "waitpkg", "movdiri",
        "movdir64b", "enqcmd", "serialize", "tsxldtrk", "amx-bf16",
        "amx-tile", "amx-int8", "uintr", "hreset", "kl", "widekl",
        "avxvnni", "avx512fp16", "cmpccxadd", "wrmsrns", "msrlist",
        "amx-fp16", "prefetchi", "raoint", "avxifma", "avxvnniint8",
        "avxneconvert", "cmpccxadd", NULL
    };
    
    /* Check each feature - each call may trigger cache detection */
    for (int i = 0; features[i]; i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Check CPU models - different models have different cache descriptors */
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "tigerlake", "cooperlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniterapids-d", "sierraforest", "grandridge", "clearwaterforest",
        "znver1", "znver2", "znver3", "znver4", "znver5", NULL
    };
    
    for (int i = 0; models[i]; i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force different cache considerations */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = (volatile char *)__builtin_alloca(256 * 1024);
    for (int i = 0; i < 256 * 1024; i += 64) {
        buffer[i] = i & 0xFF;
    }
    timing_results[0] = buffer[0];
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = (volatile char *)__builtin_alloca(512 * 1024);
    for (int i = 0; i < 512 * 1024; i += 64) {
        buffer[i] = i & 0xFF;
    }
    timing_results[1] = buffer[64];
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *buffer = (volatile char *)__builtin_alloca(1024 * 1024);
    for (int i = 0; i < 1024 * 1024; i += 64) {
        buffer[i] = i & 0xFF;
    }
    timing_results[2] = buffer[128];
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = (volatile char *)__builtin_alloca(2048 * 1024);
    for (int i = 0; i < 2048 * 1024; i += 64) {
        buffer[i] = i & 0xFF;
    }
    timing_results[3] = buffer[256];
}

__attribute__((target("arch=znver2")))
static void znver2_cache_test(void) {
    volatile char *buffer = (volatile char *)__builtin_alloca(1024 * 1024);
    for (int i = 0; i < 1024 * 1024; i += 64) {
        buffer[i] = i & 0xFF;
    }
    timing_results[4] = buffer[512];
}

/* Cache size detection through timing */
static void detect_cache_sizes(void) {
    const size_t max_size = 8 * 1024 * 1024; /* 8MB */
    volatile char *buffer = (volatile char *)malloc(max_size);
    uint64_t times[5] = {0};
    
    /* Prime the cache */
    for (size_t i = 0; i < max_size; i += 64) {
        buffer[i] = 0;
    }
    
    /* Time different access patterns that might trigger cache queries */
    for (int pattern = 0; pattern < 5; pattern++) {
        uint64_t start = __rdtsc();
        
        switch (pattern) {
            case 0: /* Sequential access - L1 cache friendly */
                for (size_t i = 0; i < 32 * 1024; i += 64) {
                    timing_results[5] += buffer[i];
                }
                break;
            case 1: /* Medium stride - might hit L2 */
                for (size_t i = 0; i < 256 * 1024; i += 128) {
                    timing_results[5] += buffer[i];
                }
                break;
            case 2: /* Large stride - likely L3/memory */
                for (size_t i = 0; i < max_size; i += 256) {
                    timing_results[5] += buffer[i];
                }
                break;
            case 3: /* Random access within L1 */
                for (int j = 0; j < 1000; j++) {
                    timing_results[5] += buffer[(j * 67) % (32 * 1024)];
                }
                break;
            case 4: /* Random access within L2 */
                for (int j = 0; j < 1000; j++) {
                    timing_results[5] += buffer[(j * 67) % (256 * 1024)];
                }
                break;
        }
        
        uint64_t end = __rdtsc();
        times[pattern] = end - start;
    }
    
    free((void*)buffer);
    
    /* Store timing ratios - compiler might use these for cache tuning */
    if (times[1] > times[0] * 2) timing_results[6] = 1;
    if (times[2] > times[1] * 3) timing_results[6] = 2;
    if (times[3] < times[4] / 2) timing_results[6] = 3;
}

/* Force compiler to consider different cache line sizes */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_aligned_test(void) {
    /* 32-byte aligned accesses for AVX2 */
    volatile int *buffer = (volatile int *)__builtin_alloca(1024 * 1024);
    for (int i = 0; i < 256 * 1024; i += 8) { /* 8 ints = 32 bytes */
        buffer[i] = i;
    }
    timing_results[7] = buffer[0];
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_aligned_test(void) {
    /* 64-byte aligned accesses for AVX512 */
    volatile long long *buffer = (volatile long long *)__builtin_alloca(1024 * 1024);
    for (int i = 0; i < 128 * 1024; i += 8) { /* 8 longs = 64 bytes */
        buffer[i] = i;
    }
    timing_results[7] += buffer[0];
}
#pragma GCC pop_options

int main(void) {
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    znver2_cache_test();
    
    /* Perform cache timing detection */
    detect_cache_sizes();
    
    /* Call vectorized cache tests */
    avx2_cache_aligned_test();
    avx512_cache_aligned_test();
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum ^= timing_results[i];
    }
    
    for (int i = 0; i < 256 && cpu_features[i]; i++) {
        checksum += cpu_features[i];
    }
    
    for (int i = 0; i < 32 && cpu_models[i]; i++) {
        checksum += cpu_models[i];
    }
    
    /* Use checksum to affect control flow */
    if (checksum & 1) {
        printf("CPU feature checksum: %d\n", checksum);
    } else {
        printf("Alternate path checksum: %d\n", checksum);
    }
    
    return checksum & 0xFF;
}

/* Additional test to force specific cache descriptor processing */
#ifdef FORCE_CACHE_DESC
/* This would normally be in a separate compilation unit with special flags */
__attribute__((used))
static void force_cache_descriptors(void) {
    /* Create artificial CPUID-like data to potentially trigger all cases */
    struct cache_desc {
        unsigned char desc;
        const char *name;
    } descriptors[] = {
        {0x0a, "L1: 8KB, 2-way, 32B line"},
        {0x0c, "L1: 16KB, 4-way, 32B line"},
        {0x0d, "L1: 16KB, 4-way, 64B line"},
        {0x0e, "L1: 24KB, 6-way, 64B line"},
        {0x21, "L2: 256KB, 8-way, 64B line"},
        {0x24, "L2: 1MB, 16-way, 64B line"},
        {0x2c, "L1: 32KB, 8-way, 64B line"},
        {0x39, "L2: 128KB, 4-way, 64B line"},
        {0x3a, "L2: 192KB, 6-way, 64B line"},
        {0x3b, "L2: 128KB, 2-way, 64B line"},
        {0x3c, "L2: 256KB, 4-way, 64B line"},
        {0x3d, "L2: 384KB, 6-way, 64B line"},
        {0x3e, "L2: 512KB, 4-way, 64B line"},
        {0x41, "L2: 128KB, 4-way, 32B line"},
        {0x42, "L2: 256KB, 4-way, 32B line"},
        {0x43, "L2: 512KB, 4-way, 32B line"},
        {0x44, "L2: 1MB, 4-way, 32B line"},
        {0x45, "L2: 2MB, 4-way, 32B line"},
        {0x48, "L2: 3MB, 12-way, 64B line"},
        {0x49, "L2: 4MB, 16-way, 64B line"},
        {0x4e, "L2: 6MB, 24-way, 64B line"},
        {0x60, "L1: 16KB, 8-way, 64B line"},
        {0x66, "L1: 8KB, 4-way, 64B line"},
        {0x67, "L1: 16KB, 4-way, 64B line"},
        {0x68, "L1: 32KB, 4-way, 64B line"},
        {0x78, "L2: 1MB, 4-way, 64B line"},
        {0x79, "L2: 128KB, 8-way, 64B line"},
        {0x7a, "L2: 256KB, 8-way, 64B line"},
        {0x7b, "L2: 512KB, 8-way, 64B line"},
        {0x7c, "L2: 1MB, 8-way, 64B line"},
        {0x7d, "L2: 2MB, 8-way, 64B line"},
        {0x7f, "L2: 512KB, 2-way, 64B line"},
        {0x80, "L2: 512KB, 8-way, 64B line"},
        {0x82, "L2: 256KB, 8-way, 32B line"},
        {0x83, "L2: 512KB, 8-way, 32B line"},
        {0x84, "L2: 1MB, 8-way, 32B line"},
        {0x85, "L2: 2MB, 8-way, 32B line"},
        {0x86, "L2: 512KB, 4-way, 64B line"},
        {0x87, "L2: 1MB, 8-way, 64B line"},
        {0, NULL}
    };
    
    /* Reference the descriptors to keep them in the binary */
    volatile int dummy = 0;
    for (int i = 0; descriptors[i].name; i++) {
        dummy += descriptors[i].desc;
    }
}
#endif
