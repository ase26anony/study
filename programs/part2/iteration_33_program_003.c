/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[256];
static volatile int sink;

/* Early CPU initialization to force driver cache detection */
__attribute__((constructor(101)))
static void early_cpu_init(void) {
    /* This forces the driver to execute CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different paths */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124fmaps",
        "avx5124vnniw", "avx512vp2intersect", "fma", "fma4", "xop",
        "aes", "pclmul", "sha", "gfni", "vaes", "vpclmulqdq",
        "movbe", "rdrnd", "rdseed", "adx", "lzcnt", "bmi", "bmi2",
        "f16c", "fsgsbase", "rtm", "xtpr", "pdpe1gb", "abm", "sse4a",
        "misalignsse", "3dnow", "3dnowa", "sse5", "svm", "cx16",
        "prfchw", "rdpid", "clzero", "mwaitx", "clflushopt", "clwb",
        "pku", "ospke", "waitpkg", "cldemote", "movdiri", "movdir64b",
        "enqcmd", "serialize", "tsxldtrk", "amx-bf16", "amx-tile",
        "amx-int8", "uintr", "hreset", "kl", "widekl", "avxvnni",
        "avx512fp16", "avxifma", "avxvnniint8", "avxneconvert",
        "cmpccxadd", "amx-fp16", "prefetchi", "raoint", "amx-complex",
        NULL
    };
    
    /* Check each feature */
    for (int i = 0; features[i]; i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Check various CPU models to trigger different cache mappings */
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniterapids-d", "sierraforest", "grandridge", "clearwaterforest",
        "knl", "knm", "bonnell", "silvermont", "goldmont", "goldmont-plus",
        "tremont", "gracemont", "generic", "k8", "k8-sse3", "opteron",
        "athlon64", "athlon-fx", "k10", "barcelona", "amdfam10", "bulldozer",
        "piledriver", "steamroller", "excavator", "btver1", "btver2",
        "bdver1", "bdver2", "bdver3", "bdver4", "znver1", "znver2",
        "znver3", "znver4", NULL
    };
    
    /* Query each model */
    for (int i = 0; models[i]; i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024);
    if (!buffer) return;
    
    /* Access pattern that depends on cache line size */
    for (int i = 0; i < 1024 * 1024; i += 64) {
        buffer[i] = i & 0xFF;
    }
    
    sink = buffer[0];
    free((void*)buffer);
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(2 * 1024 * 1024);
    if (!buffer) return;
    
    /* Different stride to test cache */
    for (int i = 0; i < 2 * 1024 * 1024; i += 128) {
        buffer[i] = i & 0xFF;
    }
    
    sink = buffer[0];
    free((void*)buffer);
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *buffer = malloc(4 * 1024 * 1024);
    if (!buffer) return;
    
    /* Larger working set */
    for (int i = 0; i < 4 * 1024 * 1024; i += 256) {
        buffer[i] = i & 0xFF;
    }
    
    sink = buffer[0];
    free((void*)buffer);
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(8 * 1024 * 1024);
    if (!buffer) return;
    
    /* Even larger working set */
    for (int i = 0; i < 8 * 1024 * 1024; i += 512) {
        buffer[i] = i & 0xFF;
    }
    
    sink = buffer[0];
    free((void*)buffer);
}

__attribute__((target("arch=znver2")))
static void zen2_cache_test(void) {
    volatile char *buffer = malloc(16 * 1024 * 1024);
    if (!buffer) return;
    
    /* Very large working set */
    for (int i = 0; i < 16 * 1024 * 1024; i += 1024) {
        buffer[i] = i & 0xFF;
    }
    
    sink = buffer[0];
    free((void*)buffer);
}

/* Cache size detection through timing */
static uint64_t detect_cache_size(void) {
    const size_t max_size = 32 * 1024 * 1024;
    volatile char *buffer = malloc(max_size);
    uint64_t start, end;
    
    if (!buffer) return 0;
    
    /* Initialize buffer */
    memset((void*)buffer, 0, max_size);
    
    /* Time access to different sized regions */
    uint64_t min_time = UINT64_MAX;
    size_t detected_size = 0;
    
    for (size_t size = 4 * 1024; size <= max_size; size *= 2) {
        /* Warm up cache */
        for (size_t i = 0; i < size; i += 64) {
            sink = buffer[i];
        }
        
        /* Time sequential access */
        start = __builtin_ia32_rdtsc();
        for (size_t i = 0; i < size; i += 64) {
            sink = buffer[i];
        }
        end = __builtin_ia32_rdtsc();
        
        uint64_t time = end - start;
        if (time < min_time) {
            min_time = time;
            detected_size = size;
        }
    }
    
    free((void*)buffer);
    return detected_size;
}

/* Main function that exercises all paths */
int main(void) {
    uint64_t cache_size;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen2_cache_test();
    
    /* Try to detect cache size through timing */
    cache_size = detect_cache_size();
    
    /* Use CPU feature detection results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= cpu_features[i];
        checksum ^= cpu_models[i];
    }
    
    checksum ^= (cache_size >> 32) ^ (cache_size & 0xFFFFFFFF);
    
    /* Output something to prevent optimization */
    printf("CPU checksum: %d, Detected cache: %lu KB\n", 
           checksum, cache_size / 1024);
    
    return 0;
}

/* Force inclusion of various pragmas to trigger driver */
#pragma GCC target("avx2")
static void avx2_test(void) {
    volatile int x = 0;
    for (int i = 0; i < 1000; i++) {
        x += i;
    }
    sink = x;
}

#pragma GCC target("avx512f")
static void avx512_test(void) {
    volatile int x = 0;
    for (int i = 0; i < 1000; i++) {
        x += i;
    }
    sink = x;
}

#pragma GCC target("sse4.2")
static void sse42_test(void) {
    volatile int x = 0;
    for (int i = 0; i < 1000; i++) {
        x += i;
    }
    sink = x;
}
