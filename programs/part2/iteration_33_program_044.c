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

/* Early CPU initialization - runs before main */
__attribute__((constructor))
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a comprehensive list of CPU features to trigger cache detection */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "fma", "fma4", "xop",
        "aes", "pclmul", "sha", "gfni", "vaes", "vpclmulqdq",
        "movbe", "rdrnd", "rdseed", "adx", "bmi", "bmi2", "lzcnt",
        "f16c", "fsgsbase", "fxsr", "xsave", "xsaveopt", "xsavec",
        "xsaves", "mwaitx", "clzero", "pku", "ospke", "rdpid",
        "sgx", "cldemote", "ptwrite", "serialize", "tsxldtrk",
        "amx-bf16", "amx-tile", "amx-int8", "uintr", "hreset"
    };
    
    const char *models[] = {
        "intel", "amd", "athlon", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "generic"
    };
    
    /* Store feature detection results */
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Store model detection results */
    for (size_t i = 0; i < sizeof(models)/sizeof(models[0]); i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Cache-sensitive memory access patterns */
__attribute__((target("arch=core2")))
static void cache_test_core2(void) {
    const size_t size = 1024 * 1024 * 16; /* 16MB - larger than typical L2 */
    char *buffer = malloc(size);
    volatile char *vbuffer = buffer;
    
    if (!buffer) return;
    
    /* Fill with pattern */
    memset(buffer, 0xAA, size);
    
    /* Access with different strides to trigger cache logic */
    for (size_t stride = 32; stride <= 64; stride *= 2) {
        for (size_t i = 0; i < size; i += stride) {
            sink = vbuffer[i];
        }
    }
    
    free(buffer);
}

__attribute__((target("arch=sandybridge")))
static void cache_test_sandybridge(void) {
    const size_t size = 1024 * 1024 * 32; /* 32MB */
    char *buffer = malloc(size);
    volatile char *vbuffer = buffer;
    
    if (!buffer) return;
    
    memset(buffer, 0x55, size);
    
    /* Different access pattern */
    for (size_t block = 0; block < 64; block++) {
        for (size_t i = block * 1024; i < size; i += 64 * 1024) {
            sink = vbuffer[i % size];
        }
    }
    
    free(buffer);
}

__attribute__((target("arch=skylake")))
static void cache_test_skylake(void) {
    const size_t size = 1024 * 1024 * 64; /* 64MB */
    char *buffer = malloc(size);
    volatile char *vbuffer = buffer;
    
    if (!buffer) return;
    
    memset(buffer, 0xCC, size);
    
    /* Random-like access pattern */
    size_t prime = 15485863; /* A large prime */
    for (size_t i = 0; i < 1000000; i++) {
        sink = vbuffer[(i * prime) % size];
    }
    
    free(buffer);
}

/* AVX2-optimized memory test */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    const size_t size = 1024 * 1024 * 8;
    char *buffer = malloc(size);
    
    if (!buffer) return;
    
    /* Use AVX2 operations if supported */
    if (__builtin_cpu_supports("avx2")) {
        /* Simulate some vector work */
        for (size_t i = 0; i < size; i += 256) {
            /* Force memory access */
            buffer[i] = i & 0xFF;
        }
    }
    
    free(buffer);
}
#pragma GCC pop_options

/* AVX512-optimized memory test */
#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    const size_t size = 1024 * 1024 * 16;
    char *buffer = malloc(size);
    
    if (!buffer) return;
    
    if (__builtin_cpu_supports("avx512f")) {
        /* Different access pattern for AVX512 */
        for (size_t i = 0; i < size; i += 512) {
            buffer[i] = (i >> 8) & 0xFF;
        }
    }
    
    free(buffer);
}
#pragma GCC pop_options

/* Timing-based cache detection */
static void timing_cache_test(void) {
    const size_t sizes[] = {8 * 1024, 16 * 1024, 32 * 1024, 64 * 1024,
                           128 * 1024, 256 * 1024, 512 * 1024,
                           1024 * 1024, 2 * 1024 * 1024,
                           4 * 1024 * 1024, 8 * 1024 * 1024,
                           16 * 1024 * 1024, 32 * 1024 * 1024};
    
    for (size_t s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        size_t size = sizes[s];
        char *buffer = malloc(size);
        
        if (!buffer) continue;
        
        memset(buffer, 0, size);
        
        /* Time the access */
        uint64_t start = __builtin_ia32_rdtsc();
        
        volatile char *v = buffer;
        for (size_t i = 0; i < size; i += 64) {
            sink = v[i];
        }
        
        uint64_t end = __builtin_ia32_rdtsc();
        uint64_t delta = end - start;
        
        /* Use timing to branch - forces compiler to consider cache effects */
        if (delta < 1000000) {
            cpu_features[100 + s] = 1; /* Fast access - likely in cache */
        } else {
            cpu_features[100 + s] = 0; /* Slow access - likely cache miss */
        }
        
        free(buffer);
    }
}

int main(void) {
    int checksum = 0;
    
    /* Execute all cache tests */
    cache_test_core2();
    cache_test_sandybridge();
    cache_test_skylake();
    avx2_cache_test();
    avx512_cache_test();
    timing_cache_test();
    
    /* Compute checksum from all CPU feature flags */
    for (int i = 0; i < 256; i++) {
        checksum ^= cpu_features[i] * (i + 1);
    }
    
    for (int i = 0; i < 32; i++) {
        checksum ^= cpu_models[i] * (i + 1001);
    }
    
    /* Prevent dead code elimination */
    printf("CPU checksum: %d\n", checksum);
    
    /* Additional CPUID queries in main */
    if (__builtin_cpu_supports("sse4.2")) {
        checksum += 1;
    }
    if (__builtin_cpu_supports("avx")) {
        checksum += 2;
    }
    if (__builtin_cpu_supports("avx2")) {
        checksum += 4;
    }
    if (__builtin_cpu_supports("avx512f")) {
        checksum += 8;
    }
    
    /* Query specific CPU models */
    if (__builtin_cpu_is("intel")) {
        checksum += 16;
    }
    if (__builtin_cpu_is("amd")) {
        checksum += 32;
    }
    if (__builtin_cpu_is("core2")) {
        checksum += 64;
    }
    if (__builtin_cpu_is("sandybridge")) {
        checksum += 128;
    }
    if (__builtin_cpu_is("skylake")) {
        checksum += 256;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
