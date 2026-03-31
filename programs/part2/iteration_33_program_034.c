/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to trigger GCC x86 driver cache detection logic */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[256];
static volatile int sink;

/* Early CPU initialization - forces driver to run CPUID and cache detection */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* This forces the driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different cache detection paths */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "avx512bf16", "avx512fp16",
        "fma", "fma4", "xop", "aes", "pclmul", "rdseed", "rdrand",
        "sha", "gfni", "vaes", "vpclmulqdq", "movbe", "fsgsbase",
        "bmi", "bmi2", "lzcnt", "fxsr", "xsave", "xsaveopt",
        "xsavec", "xsaves", "mwaitx", "clzero", "pku", "ospke",
        "waitpkg", "cldemote", "movdiri", "movdir64b", "enqcmd",
        "serialize", "tsxldtrk", "amx-bf16", "amx-tile", "amx-int8",
        "uintr", "hreset", "kl", "widekl", "avxvnni", "avx512vp2intersect",
        NULL
    };
    
    const char *models[] = {
        "intel", "amd", "athlon", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniteforge", "sierraforest", "grandridge", "clearwaterforest",
        "atom", "bonnell", "silvermont", "goldmont", "goldmont-plus",
        "tremont", "gracemont", "generic", NULL
    };
    
    /* Store feature detection results */
    int i = 0;
    for (const char **f = features; *f; f++) {
        cpu_features[i++] = __builtin_cpu_supports(*f);
    }
    
    /* Store model detection results */
    i = 0;
    for (const char **m = models; *m; m++) {
        cpu_models[i++] = __builtin_cpu_is(*m);
    }
}

/* Target-specific functions to force driver to consider different cache configurations */
__attribute__((target("arch=core2")))
static void test_core2_cache(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB array */
    if (!array) return;
    
    /* Access pattern that depends on cache line size */
    for (int i = 0; i < 1024 * 1024; i += 64) { /* 64-byte stride */
        array[i] = i & 0xFF;
    }
    
    /* Force compiler to consider cache effects */
    for (int i = 0; i < 1024 * 1024; i += 32) { /* 32-byte stride */
        sink += array[i];
    }
    
    free((void*)array);
}

__attribute__((target("arch=sandybridge")))
static void test_sandybridge_cache(void) {
    volatile char *array = malloc(2 * 1024 * 1024); /* 2MB array */
    if (!array) return;
    
    /* Different access pattern */
    for (int i = 0; i < 2 * 1024 * 1024; i += 128) {
        array[i] = i & 0xFF;
    }
    
    /* Mix of strides */
    for (int i = 0; i < 2 * 1024 * 1024; i += 64) {
        sink += array[i];
    }
    
    free((void*)array);
}

__attribute__((target("arch=skylake")))
static void test_skylake_cache(void) {
    volatile char *array = malloc(4 * 1024 * 1024); /* 4MB array */
    if (!array) return;
    
    /* Larger working set */
    for (int i = 0; i < 4 * 1024 * 1024; i += 256) {
        array[i] = i & 0xFF;
    }
    
    for (int i = 0; i < 4 * 1024 * 1024; i += 64) {
        sink += array[i];
    }
    
    free((void*)array);
}

__attribute__((target("arch=znver1")))
static void test_zen_cache(void) {
    volatile char *array = malloc(8 * 1024 * 1024); /* 8MB array */
    if (!array) return;
    
    /* Even larger working set */
    for (int i = 0; i < 8 * 1024 * 1024; i += 512) {
        array[i] = i & 0xFF;
    }
    
    for (int i = 0; i < 8 * 1024 * 1024; i += 64) {
        sink += array[i];
    }
    
    free((void*)array);
}

/* Function with pragma to trigger AVX-specific paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void test_avx2_cache(void) {
    volatile int *array = malloc(1024 * 1024 * sizeof(int));
    if (!array) return;
    
    /* Vector-friendly access pattern */
    for (int i = 0; i < 256 * 1024; i++) {
        array[i * 4] = i;
    }
    
    for (int i = 0; i < 256 * 1024; i++) {
        sink += array[i * 4];
    }
    
    free((void*)array);
}
#pragma GCC pop_options

/* Cache size estimation through timing */
static void estimate_cache_sizes(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32MB */
    volatile char *buffer = malloc(max_size);
    if (!buffer) return;
    
    /* Initialize buffer */
    for (size_t i = 0; i < max_size; i++) {
        buffer[i] = (char)(i & 0xFF);
    }
    
    /* Test different working set sizes */
    size_t sizes[] = {8 * 1024, 16 * 1024, 32 * 1024, 64 * 1024,
                      128 * 1024, 256 * 1024, 512 * 1024, 1024 * 1024,
                      2 * 1024 * 1024, 4 * 1024 * 1024, 8 * 1024 * 1024,
                      16 * 1024 * 1024, 32 * 1024 * 1024};
    
    for (size_t s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        size_t size = sizes[s];
        if (size > max_size) break;
        
        /* Time the access */
        uint64_t start = __builtin_ia32_rdtsc();
        
        /* Sequential access */
        for (size_t i = 0; i < size; i += 64) { /* 64-byte cache lines */
            sink += buffer[i];
        }
        
        uint64_t end = __builtin_ia32_rdtsc();
        uint64_t cycles = end - start;
        
        /* Use timing to branch - forces compiler to consider cache effects */
        if (cycles < 1000000) {
            /* Likely fits in cache */
            sink += 1;
        } else {
            /* Likely cache miss */
            sink += 2;
        }
    }
    
    free((void*)buffer);
}

/* Main function that orchestrates all tests */
int main(void) {
    int result = 0;
    
    /* Call all target-specific functions */
    test_core2_cache();
    test_sandybridge_cache();
    test_skylake_cache();
    test_zen_cache();
    test_avx2_cache();
    
    /* Perform cache size estimation */
    estimate_cache_sizes();
    
    /* Use CPU feature detection results to create a checksum */
    for (int i = 0; i < 64; i++) {
        if (cpu_features[i]) {
            result ^= (1 << (i % 32));
        }
    }
    
    for (int i = 0; i < 32; i++) {
        if (cpu_models[i]) {
            result += i * 3;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int output = result + sink;
    
    printf("CPU test completed. Checksum: %d\n", output);
    return 0;
}
