/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[32];
static volatile int sink;

/* Early CPU initialization - forces driver to run CPUID */
__attribute__((constructor))
static void init_cpu(void) {
    /* This forces the driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query every possible CPU feature to trigger cache detection */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "fma", "fma4", "xop",
        "aes", "pclmul", "sha", "gfni", "vaes", "vpclmulqdq",
        "movbe", "rdrnd", "rdseed", "adx", "bmi", "bmi2", "lzcnt",
        "f16c", "fsgsbase", "rtm", "xtpr", "pku", "ospke", "cldemote",
        "waitpkg", "movdiri", "movdir64b", "enqcmd", "serialize",
        "tsxldtrk", "pconfig", "wbnoinvd", "amx-bf16", "amx-tile",
        "amx-int8", "avxifma", "avx-ne-convert", "avx-vnni-int8",
        "cmpccxadd", "wrmsrns", "msrlist", "amx-fp16", "prefetchi",
        "raoint", "amx-complex", "avx-vnni-int16", "sha512", "sm3",
        "sm4", "avx512bf16", "avx512fp16", "hreset", "kl", "widekl",
        "uintr", "hle", "rtm", "xtpr", "mwaitx", "clzero", "monitorx",
        "clwb", "mcommit", "sev", "sev-es", "sev-snp", "invpcid",
        "pconfig", "wbnoinvd", NULL
    };
    
    /* Check each feature - each call may trigger cache detection */
    for (int i = 0; features[i]; i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Check CPU models - each may have different cache configurations */
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniterapids-d", "sierraforest", "grandridge", "clearwaterforest",
        "bonnell", "silvermont", "goldmont", "goldmont-plus",
        "tremont", "gracemont", "generic", NULL
    };
    
    for (int i = 0; models[i]; i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *array = malloc(1024 * 1024);
    if (!array) return;
    
    /* Access pattern that depends on cache line size */
    for (int i = 0; i < 1024 * 1024; i += 64) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(2 * 1024 * 1024);
    if (!array) return;
    
    /* Different stride for different cache architectures */
    for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *array = malloc(4 * 1024 * 1024);
    if (!array) return;
    
    /* Mix of strides to trigger different cache logic */
    for (int i = 0; i < 4 * 1024 * 1024; i += 128) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(8 * 1024 * 1024);
    if (!array) return;
    
    /* Large array to potentially exceed L2 cache */
    for (int i = 0; i < 8 * 1024 * 1024; i += 256) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

/* Function to perform cache-sensitive timing */
static uint64_t measure_cache_access(int size_kb, int stride) {
    volatile char *array = malloc(size_kb * 1024);
    if (!array) return 0;
    
    uint64_t start, end;
    
    /* Use rdtsc for timing - forces CPU to actually execute */
    start = __builtin_ia32_rdtsc();
    
    /* Access pattern that depends on cache characteristics */
    for (int i = 0; i < size_kb * 1024; i += stride) {
        array[i] = i;
    }
    
    end = __builtin_ia32_rdtsc();
    
    sink = array[0];
    free((void*)array);
    
    return end - start;
}

/* Main function that uses all the CPU feature detection */
int main(void) {
    uint64_t checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    
    /* Measure with different cache sizes and strides */
    uint64_t timings[16];
    
    /* Test various sizes that match the uncovered cache descriptors */
    int test_sizes[] = {8, 16, 24, 32, 128, 256, 384, 512, 1024, 2048, 3072, 4096, 6144};
    int test_strides[] = {32, 64};
    
    int timing_idx = 0;
    for (int i = 0; i < sizeof(test_sizes)/sizeof(test_sizes[0]) && timing_idx < 16; i++) {
        for (int j = 0; j < sizeof(test_strides)/sizeof(test_strides[0]) && timing_idx < 16; j++) {
            timings[timing_idx] = measure_cache_access(test_sizes[i], test_strides[j]);
            timing_idx++;
        }
    }
    
    /* Create checksum from feature flags and timings */
    for (int i = 0; i < 32; i++) {
        checksum ^= (cpu_features[i] << i);
    }
    
    for (int i = 0; i < 16; i++) {
        checksum ^= (timings[i] << (i % 64));
    }
    
    /* Use checksum to prevent dead code elimination */
    sink = checksum;
    
    /* Print something to satisfy test framework */
    printf("CPU detection test completed. Checksum: %llu\n", 
           (unsigned long long)checksum);
    
    return 0;
}

/* Force inclusion of various pragmas to trigger driver paths */
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
