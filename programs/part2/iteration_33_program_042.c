/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */
/* This forces execution of CPUID interpretation for various cache descriptors */

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
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide range of CPU features to trigger cache detection */
    const char* features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "fma", "fma4", "xop",
        "aes", "pclmul", "sha", "gfni", "vaes", "vpclmulqdq",
        "movbe", "rdrnd", "rdseed", "adx", "bmi", "bmi2", "lzcnt",
        "f16c", "fsgsbase", "rtm", "xtpr", "mwaitx", "clzero",
        "pku", "ospke", "rdpid", "sgx", "cldemote", "ptwrite",
        "serialize", "tsxldtrk", "amx-bf16", "amx-tile", "amx-int8",
        "uintr", "hreset", "kl", "widekl", "avxifma", "avxvnni",
        "avx512fp16", "avxvnniint8", "cmpccxadd", "amx-fp16",
        "prefetchi", "raoint", "amx-complex"
    };
    
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]) && i < 64; i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Query CPU models to trigger different cache descriptor paths */
    const char* models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "tigerlake", "sapphirerapids", "alderlake",
        "rocketlake", "graniterapids", "graniteforge", "zen",
        "zen2", "zen3", "zen4", "barcelona", "shanghai", "istanbul",
        "bulldozer", "piledriver", "steamroller", "excavator",
        "znver1", "znver2", "znver3", "znver4", "bonnell",
        "silvermont", "goldmont", "goldmont-plus", "tremont",
        "gracemont", "knl", "knm"
    };
    
    for (size_t i = 0; i < sizeof(models)/sizeof(models[0]) && i < 16; i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = malloc(256 * 1024); /* 256KB - typical L2 for Core2 */
    if (buffer) {
        for (int i = 0; i < 256 * 1024; i += 64) { /* 64-byte lines */
            buffer[i] = i & 0xFF;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024); /* 1MB - typical L3 for Sandy Bridge */
    if (buffer) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        sink = buffer[64];
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(2048 * 1024); /* 2MB - typical L3 for Skylake */
    if (buffer) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        sink = buffer[128];
        free((void*)buffer);
    }
}

__attribute__((target("arch=znver3")))
static void zen3_cache_test(void) {
    volatile char *buffer = malloc(32 * 1024); /* 32KB L1D for Zen 3 */
    if (buffer) {
        for (int i = 0; i < 32 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        sink = buffer[256];
        free((void*)buffer);
    }
}

/* Function with pragmas to trigger different optimization paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_sensitive(void) {
    /* Large working set to exceed L1/L2 cache */
    const size_t size = 4 * 1024 * 1024; /* 4MB */
    volatile char *data = malloc(size);
    if (!data) return;
    
    /* Access pattern that depends on cache line size */
    for (size_t i = 0; i < size; i += 128) { /* Two cache lines at a time */
        data[i] = 1;
        data[i + 64] = 2; /* Second cache line */
    }
    
    /* Random access to trigger cache misses */
    for (int i = 0; i < 1000; i++) {
        size_t idx = (i * 997) % size; /* Pseudo-random index */
        sink = data[idx];
    }
    
    free((void*)data);
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_sensitive(void) {
    /* Even larger working set */
    const size_t size = 16 * 1024 * 1024; /* 16MB */
    volatile char *data = malloc(size);
    if (!data) return;
    
    /* Strided access with different strides */
    for (int stride = 64; stride <= 512; stride *= 2) {
        for (size_t i = 0; i < size; i += stride) {
            data[i] = stride & 0xFF;
        }
    }
    
    free((void*)data);
}
#pragma GCC pop_options

/* Timing-based cache detection */
static void measure_cache_effects(void) {
#define ARRAY_SIZE (8 * 1024 * 1024) /* 8MB */
    static volatile char big_array[ARRAY_SIZE];
    
    /* Initialize */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        big_array[i] = (char)(i & 0xFF);
    }
    
    /* Measure time for sequential access (cache friendly) */
    uint64_t start, end;
    
    /* Use rdtsc for timing - forces CPU to be in a known state */
    start = __builtin_ia32_rdtsc();
    for (size_t i = 0; i < ARRAY_SIZE; i += 64) {
        sink = big_array[i];
    }
    end = __builtin_ia32_rdtsc();
    uint64_t seq_time = end - start;
    
    /* Measure time for random access (cache unfriendly) */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < 100000; i++) {
        size_t idx = (i * 10007) % ARRAY_SIZE; /* Prime stride */
        sink = big_array[idx];
    }
    end = __builtin_ia32_rdtsc();
    uint64_t rand_time = end - start;
    
    /* Use timing ratio to branch - compiler may use cache size info */
    if (rand_time > seq_time * 3) {
        /* Likely smaller caches */
        sink = 1;
    } else {
        /* Likely larger caches */
        sink = 2;
    }
}

/* Main function that exercises all paths */
int main(void) {
    printf("Testing GCC x86 driver cache detection...\n");
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    skylake_cache_test();
    zen3_cache_test();
    
    /* Call vectorized cache-sensitive functions */
    avx2_cache_sensitive();
    avx512_cache_sensitive();
    
    /* Perform timing-based measurements */
    measure_cache_effects();
    
    /* Use CPU feature detection results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= cpu_features[i] * i;
    }
    for (int i = 0; i < 16; i++) {
        checksum ^= cpu_models[i] * (100 + i);
    }
    
    printf("CPU feature checksum: %d\n", checksum);
    printf("Test completed - driver cache detection should have been exercised.\n");
    
    return checksum & 1; /* Return non-deterministic exit code */
}

/* Additional test cases for specific cache descriptor values */
/* These functions are designed to hint at specific cache configurations */

__attribute__((target("arch=atom")))
static void test_atom_cache(void) {
    /* Atom typically has specific cache configurations */
    volatile int array[16 * 1024]; /* 64KB when int is 4 bytes */
    for (int i = 0; i < 16 * 1024; i += 16) { /* 64-byte cache lines */
        array[i] = i;
    }
    sink = array[0];
}

__attribute__((target("arch=knl")))
static void test_knl_cache(void) {
    /* Knights Landing has large caches */
    volatile char *huge = malloc(32 * 1024 * 1024); /* 32MB */
    if (huge) {
        for (size_t i = 0; i < 32 * 1024 * 1024; i += 256) {
            huge[i] = i & 0xFF;
        }
        sink = huge[1024 * 1024];
        free((void*)huge);
    }
}

/* Force inclusion of all test functions */
static void call_all_tests(void) {
    test_atom_cache();
    test_knl_cache();
}

/* Compile-time check for cache-related builtins */
_Static_assert(sizeof(__builtin_ia32_rdtsc()) == sizeof(uint64_t),
               "rdtsc must return 64-bit value");
