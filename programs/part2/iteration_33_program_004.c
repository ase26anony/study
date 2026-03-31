/* Test program to exercise x86 CPU cache detection in GCC driver */
/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[256];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - runs before main */
static void __attribute__((constructor)) init_cpu(void) {
    /* Force CPUID initialization */
    __builtin_cpu_init();
    
    /* Query all possible CPU features to trigger driver logic */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "fma", "fma4", "xop",
        "f16c", "aes", "pclmul", "sha", "gfni", "vaes", "vpclmulqdq",
        "movbe", "rdrnd", "rdseed", "ptwrite", "cldemote", "waitpkg",
        "serialize", "tsxldtrk", "amx-bf16", "amx-tile", "amx-int8",
        "avxifma", "avxvnni", "avxvnniint8", "avx-ne-convert",
        "cmpccxadd", "amx-fp16", "prefetchi", "raoint", "amx-complex",
        "avx-vnni-int16", "sha512", "sm3", "sm4", "adx", "hle", "rtm",
        "xsave", "xsavec", "xsaves", "xsaveopt", "clflushopt",
        "clwb", "mwaitx", "clzero", "monitorx", "movdiri", "movdir64b",
        "pconfig", "wbnoinvd", "waitpkg", "cet", "ibt", "shstk",
        "kl", "widekl", "rdpid", "sgx", "sgx_lc", "pks", "uintr",
        "avx512bf16", "avx512fp16", "amx-fp16", "avxifma", "avxvnni",
        "avxvnniint8", "cmpccxadd", "fzrm", "fsrm", "fsrs", "fsrc",
        "lzcnt", "bmi", "bmi2", "tbm", "lwp", "fxsr", "xsave", "xsaveopt",
        "xsavec", "xsaves", "mpx", "erms", "invpcid", "pku", "ospke",
        "rdrand", "rdseed", "mwait", "mwaitx", "clzero", "pt", "umip",
        "pku", "ospke", "cldemote", "movdiri", "movdir64b", "enqcmd",
        "serialize", "tsxldtrk", "kl", "widekl", "tdx", "hreset",
        "lam", "msrlist", "avx-vnni", "avx512vp2intersect", NULL
    };
    
    const char *models[] = {
        "intel", "amd", "athlon", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "tigerlake", "cooperlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniterapids-d", "sierraforest", "grandridge", "clearwaterforest",
        "atom", "bonnell", "silvermont", "goldmont", "goldmont-plus",
        "tremont", "gracemont", "grandridge", "pantherlake", "lunarlake",
        "darkmont", "znver1", "znver2", "znver3", "znver4", "znver5",
        NULL
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

/* Cache-sensitive timing function with target attribute for Core2 */
static void __attribute__((target("arch=core2")))
time_cache_core2(void) {
    volatile uint64_t start, end;
    const size_t size = 1024 * 1024 * 16; /* 16MB > typical L2 */
    char *buffer = malloc(size);
    
    if (!buffer) return;
    
    /* Initialize with non-zero pattern */
    memset(buffer, 1, size);
    
    /* Time sequential access */
    start = __builtin_ia32_rdtsc();
    for (size_t i = 0; i < size; i += 64) { /* 64-byte stride */
        sink = buffer[i];
    }
    end = __builtin_ia32_rdtsc();
    
    /* Use timing to influence control flow */
    if ((end - start) > 1000000) {
        cpu_features[0] ^= 1;
    }
    
    free(buffer);
}

/* Cache-sensitive timing function with target attribute for Sandy Bridge */
static void __attribute__((target("arch=sandybridge")))
time_cache_sandybridge(void) {
    volatile uint64_t start, end;
    const size_t size = 1024 * 1024 * 32; /* 32MB */
    char *buffer = malloc(size);
    
    if (!buffer) return;
    
    memset(buffer, 2, size);
    
    /* Time with 32-byte stride */
    start = __builtin_ia32_rdtsc();
    for (size_t i = 0; i < size; i += 32) {
        sink = buffer[i];
    }
    end = __builtin_ia32_rdtsc();
    
    if ((end - start) > 2000000) {
        cpu_features[1] ^= 1;
    }
    
    free(buffer);
}

/* Cache-sensitive timing function with target attribute for Skylake */
static void __attribute__((target("arch=skylake")))
time_cache_skylake(void) {
    volatile uint64_t start, end;
    const size_t size = 1024 * 1024 * 64; /* 64MB */
    char *buffer = malloc(size);
    
    if (!buffer) return;
    
    memset(buffer, 3, size);
    
    /* Random access pattern */
    start = __builtin_ia32_rdtsc();
    for (size_t i = 0; i < 1000000; i++) {
        size_t idx = (i * 97) % size; /* Pseudo-random */
        sink = buffer[idx];
    }
    end = __builtin_ia32_rdtsc();
    
    if ((end - start) > 3000000) {
        cpu_features[2] ^= 1;
    }
    
    free(buffer);
}

/* Function with AVX2 target pragma */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    volatile uint64_t start, end;
    const size_t size = 1024 * 1024 * 8;
    float *buffer = malloc(size * sizeof(float));
    
    if (!buffer) return;
    
    for (size_t i = 0; i < size; i++) {
        buffer[i] = i * 0.1f;
    }
    
    start = __builtin_ia32_rdtsc();
    /* Simulate vectorized access pattern */
    for (size_t i = 0; i < size; i += 8) {
        volatile float sum = 0;
        for (size_t j = 0; j < 8 && (i + j) < size; j++) {
            sum += buffer[i + j];
        }
        sink = (int)sum;
    }
    end = __builtin_ia32_rdtsc();
    
    if ((end - start) > 1500000) {
        cpu_features[3] ^= 1;
    }
    
    free(buffer);
}
#pragma GCC pop_options

/* Function with AVX512 target pragma */
#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    volatile uint64_t start, end;
    const size_t size = 1024 * 1024 * 16;
    double *buffer = malloc(size * sizeof(double));
    
    if (!buffer) return;
    
    for (size_t i = 0; i < size; i++) {
        buffer[i] = i * 0.01;
    }
    
    start = __builtin_ia32_rdtsc();
    /* Larger stride for AVX512 */
    for (size_t i = 0; i < size; i += 16) {
        volatile double sum = 0;
        for (size_t j = 0; j < 16 && (i + j) < size; j++) {
            sum += buffer[i + j];
        }
        sink = (int)sum;
    }
    end = __builtin_ia32_rdtsc();
    
    if ((end - start) > 2500000) {
        cpu_features[4] ^= 1;
    }
    
    free(buffer);
}
#pragma GCC pop_options

/* Main function that orchestrates all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting CPU cache detection test...\n");
    
    /* Execute all cache timing tests */
    time_cache_core2();
    time_cache_sandybridge();
    time_cache_skylake();
    avx2_cache_test();
    avx512_cache_test();
    
    /* Force use of all detected features to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        checksum ^= cpu_features[i % 50];
        checksum ^= cpu_models[i % 40];
    }
    
    /* Additional CPU feature queries in main */
    if (__builtin_cpu_supports("sse4.2")) checksum ^= 0x55;
    if (__builtin_cpu_supports("avx")) checksum ^= 0xAA;
    if (__builtin_cpu_supports("avx2")) checksum ^= 0x33;
    if (__builtin_cpu_supports("avx512f")) checksum ^= 0xCC;
    
    if (__builtin_cpu_is("intel")) checksum ^= 0xF0;
    if (__builtin_cpu_is("amd")) checksum ^= 0x0F;
    if (__builtin_cpu_is("core2")) checksum ^= 0x12;
    if (__builtin_cpu_is("sandybridge")) checksum ^= 0x34;
    if (__builtin_cpu_is("skylake")) checksum ^= 0x56;
    if (__builtin_cpu_is("znver3")) checksum ^= 0x78;
    
    /* Final computation that depends on all results */
    printf("Test checksum: %d\n", checksum);
    printf("CPU features detected: ");
    for (int i = 0; i < 10; i++) {
        printf("%d", cpu_features[i]);
    }
    printf("\n");
    
    return checksum & 1; /* Return 0 or 1 based on checksum */
}
