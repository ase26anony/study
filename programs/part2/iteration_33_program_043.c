/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise x86 CPU cache detection logic in GCC driver */
/* This forces execution of CPUID interpretation for various cache descriptors */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global feature flags set by constructor */
static int cpu_features[64];
static int cpu_models[16];
static volatile int sink;

/* Early CPU initialization - forces driver cache detection */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* This triggers the driver's CPUID interrogation */
    __builtin_cpu_init();
    
    /* Query every possible CPU feature to force driver to examine CPUID leaves */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vpopcntdq",
        "fma", "fma4", "xop", "aes", "pclmul", "rdrand", "rdseed",
        "sha", "xsave", "xsaveopt", "xsavec", "xsaves", "adx", "clflushopt",
        "clwb", "fsgsbase", "fxsr", "hle", "rtm", "bmi", "bmi2", "lzcnt",
        "movbe", "invpcid", "mwaitx", "pku", "ospke", "prefetchwt1",
        "prfchw", "rdpid", "sgx", "shstk", "tbm", "waitpkg", "wbnoinvd",
        "gfni", "vaes", "vpclmulqdq", "avx512vnni", "avx512bitalg",
        "avx512vbmi2", "avx512bf16", "avx512fp16", "amx-bf16", "amx-tile",
        "amx-int8"
    };
    
    for (int i = 0; i < sizeof(features)/sizeof(features[0]) && i < 64; i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Query CPU models - each may have different cache configurations */
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "tigerlake", "sapphirerapids", "alderlake",
        "rocketlake", "zen", "zen2", "zen3", "zen4"
    };
    
    for (int i = 0; i < sizeof(models)/sizeof(models[0]) && i < 16; i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = malloc(256 * 1024); /* L2 size for some Core2 */
    if (buffer) {
        for (int i = 0; i < 256 * 1024; i += 64) { /* 64-byte lines */
            buffer[i] = i;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024); /* L3 size */
    if (buffer) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            buffer[i] = i;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(2048 * 1024); /* Larger L3 */
    if (buffer) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            buffer[i] = i;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

__attribute__((target("arch=znver1"))) /* Zen */
static void zen_cache_test(void) {
    volatile char *buffer = malloc(8192 * 1024); /* Large L3 */
    if (buffer) {
        for (int i = 0; i < 8192 * 1024; i += 64) {
            buffer[i] = i;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

/* Function with pragmas to trigger different optimization paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_sensitive(void) {
    /* AVX2-optimized memory access pattern */
    volatile int *buffer = malloc(512 * 1024); /* Typical L2 */
    if (buffer) {
        for (int i = 0; i < 512 * 1024 / sizeof(int); i += 16) {
            buffer[i] = i * 3;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_sensitive(void) {
    /* AVX512-optimized with 64-byte cache lines */
    volatile long long *buffer = malloc(1024 * 1024);
    if (buffer) {
        for (int i = 0; i < 1024 * 1024 / sizeof(long long); i += 8) {
            buffer[i] = i * 5;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}
#pragma GCC pop_options

/* Cache size detection via timing */
static void measure_cache_effects(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32 MB */
    volatile char *buffer = malloc(max_size);
    uint64_t times[4] = {0};
    
    if (!buffer) return;
    
    /* Warm up */
    for (size_t i = 0; i < max_size; i += 4096) {
        buffer[i] = 1;
    }
    
    /* Measure different access patterns */
    for (int pattern = 0; pattern < 4; pattern++) {
        uint64_t start = __builtin_ia32_rdtsc();
        
        switch (pattern) {
            case 0: /* Sequential, cache-line sized */
                for (size_t i = 0; i < 1024 * 1024; i += 64) {
                    sink = buffer[i];
                }
                break;
            case 1: /* Sequential, half cache-line */
                for (size_t i = 0; i < 1024 * 1024; i += 32) {
                    sink = buffer[i];
                }
                break;
            case 2: /* Random within L1/L2 boundary */
                for (int i = 0; i < 10000; i++) {
                    sink = buffer[(i * 97) % (256 * 1024)];
                }
                break;
            case 3: /* Random within L3 boundary */
                for (int i = 0; i < 10000; i++) {
                    sink = buffer[(i * 97) % (8 * 1024 * 1024)];
                }
                break;
        }
        
        uint64_t end = __builtin_ia32_rdtsc();
        times[pattern] = end - start;
    }
    
    /* Use timing differences to prevent dead code elimination */
    if (times[0] > times[1]) {
        cpu_features[0] += 1;
    }
    if (times[2] < times[3]) {
        cpu_features[1] += 2;
    }
    
    free((void*)buffer);
}

int main(void) {
    /* Execute all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    skylake_cache_test();
    zen_cache_test();
    
    avx2_cache_sensitive();
    avx512_cache_sensitive();
    
    /* Force cache timing measurements */
    measure_cache_effects();
    
    /* Compute checksum from all detected features to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    for (int i = 0; i < 16; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    
    printf("CPU detection checksum: %d\n", checksum);
    printf("Features detected: ");
    for (int i = 0; i < 10; i++) {
        printf("%d", cpu_features[i]);
    }
    printf("\n");
    
    return checksum & 0xFF;
}
