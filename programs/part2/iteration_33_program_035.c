/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink;

/* Early CPU initialization - forces driver to run CPUID and cache detection */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* This forces the driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different code paths */
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
    cpu_features[12] = __builtin_cpu_supports("fma");
    cpu_features[13] = __builtin_cpu_supports("aes");
    cpu_features[14] = __builtin_cpu_supports("pclmul");
    cpu_features[15] = __builtin_cpu_supports("rdrand");
    cpu_features[16] = __builtin_cpu_supports("rdseed");
    cpu_features[17] = __builtin_cpu_supports("sha");
    cpu_features[18] = __builtin_cpu_supports("mmx");
    cpu_features[19] = __builtin_cpu_supports("3dnow");
    cpu_features[20] = __builtin_cpu_supports("popcnt");
    cpu_features[21] = __builtin_cpu_supports("bmi");
    cpu_features[22] = __builtin_cpu_supports("bmi2");
    cpu_features[23] = __builtin_cpu_supports("adx");
    cpu_features[24] = __builtin_cpu_supports("f16c");
    cpu_features[25] = __builtin_cpu_supports("fma4");
    cpu_features[26] = __builtin_cpu_supports("xop");
    cpu_features[27] = __builtin_cpu_supports("lzcnt");
    cpu_features[28] = __builtin_cpu_supports("tbm");
    cpu_features[29] = __builtin_cpu_supports("rtm");
    cpu_features[30] = __builtin_cpu_supports("mpx");
    cpu_features[31] = __builtin_cpu_supports("sgx");
    
    /* Check various CPU models to trigger different cache descriptor tables */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");
}

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = malloc(256 * 1024); /* 256KB - typical L2 for Core2 */
    if (buffer) {
        for (int i = 0; i < 256 * 1024; i += 64) { /* 64-byte stride */
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024); /* 1MB - typical L3 for Sandy Bridge */
    if (buffer) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *buffer = malloc(2048 * 1024); /* 2MB - typical L3 for Haswell */
    if (buffer) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(8192 * 1024); /* 8MB - typical L3 for Skylake */
    if (buffer) {
        for (int i = 0; i < 8192 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    volatile char *buffer = malloc(8192 * 1024); /* 8MB - typical L3 for Zen */
    if (buffer) {
        for (int i = 0; i < 8192 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

/* Cache-sensitive timing test */
static uint64_t time_cache_access(size_t size, int stride) {
    char *buffer = malloc(size);
    if (!buffer) return 0;
    
    /* Initialize buffer */
    memset(buffer, 0, size);
    
    uint64_t start, end;
    
    /* Use inline assembly for RDTSC to avoid library dependencies */
    __asm__ __volatile__ (
        "mfence\n\t"
        "rdtsc\n\t"
        "shl $32, %%rdx\n\t"
        "or %%rdx, %%rax\n\t"
        : "=a"(start)
        : 
        : "rdx", "memory"
    );
    
    /* Access pattern sensitive to cache size */
    volatile char sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        sum += buffer[i];
    }
    sink = sum;
    
    __asm__ __volatile__ (
        "mfence\n\t"
        "rdtsc\n\t"
        "shl $32, %%rdx\n\t"
        "or %%rdx, %%rax\n\t"
        : "=a"(end)
        : 
        : "rdx", "memory"
    );
    
    free(buffer);
    return end - start;
}

/* Vectorized function with pragma to trigger AVX code generation */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    /* Use AVX2 instructions that might trigger cache-specific optimizations */
    volatile int *buffer = malloc(1024 * sizeof(int));
    if (buffer) {
        for (int i = 0; i < 1024; i++) {
            buffer[i] = i;
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    /* AVX-512 uses different cache lines (64 bytes) */
    volatile char *buffer = malloc(4096 * 1024);
    if (buffer) {
        for (int i = 0; i < 4096 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

int main(void) {
    uint64_t checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    
    /* Call vectorized functions */
    avx2_cache_test();
    avx512_cache_test();
    
    /* Perform cache timing tests with different sizes and strides */
    uint64_t time1 = time_cache_access(8 * 1024, 32);    /* 8KB, 32-byte stride */
    uint64_t time2 = time_cache_access(16 * 1024, 32);   /* 16KB, 32-byte stride */
    uint64_t time3 = time_cache_access(32 * 1024, 64);   /* 32KB, 64-byte stride */
    uint64_t time4 = time_cache_access(256 * 1024, 64);  /* 256KB, 64-byte stride */
    uint64_t time5 = time_cache_access(1024 * 1024, 64); /* 1MB, 64-byte stride */
    uint64_t time6 = time_cache_access(8192 * 1024, 64); /* 8MB, 64-byte stride */
    
    /* Create checksum from timing results and CPU features */
    checksum = time1 ^ time2 ^ time3 ^ time4 ^ time5 ^ time6;
    
    /* Incorporate CPU feature flags into checksum */
    for (int i = 0; i < 32; i++) {
        if (cpu_features[i]) {
            checksum ^= (1ULL << (i % 64));
        }
    }
    
    /* Incorporate CPU model flags */
    for (int i = 0; i < 8; i++) {
        if (cpu_models[i]) {
            checksum ^= (1ULL << (32 + i));
        }
    }
    
    /* Use checksum to prevent dead code elimination */
    sink = (int)checksum;
    
    /* Print minimal output to satisfy test framework */
    printf("CPU detection test completed. Checksum: %llu\n", 
           (unsigned long long)checksum);
    
    return 0;
}
