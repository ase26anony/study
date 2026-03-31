/*
 * test_cache_detection.c
 * 
 * This program is designed to trigger GCC driver's CPU cache detection logic
 * for Intel CPUID cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 * It uses multiple techniques to ensure the driver parses these descriptors.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern 1: Multiple functions with different target attributes */
/* Each target may correspond to different cache descriptor bytes */

__attribute__((target("arch=core2")))
void compute_core2(int *data, int size) {
    /* Core2 architecture may trigger descriptors like 0x66, 0x67, 0x68 */
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 1;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int *data, int size) {
    /* Nehalem architecture may trigger descriptors like 0x0a, 0x0c, 0x0d */
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 2;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int *data, int size) {
    /* Sandy Bridge may trigger descriptors like 0x2c, 0x3a, 0x3b */
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 3;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int *data, int size) {
    /* Ivy Bridge may trigger descriptors like 0x3c, 0x3d, 0x3e */
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 5;
    }
}

/* Pattern 2: Function multiversioning with target_clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void compute_multiversion(int *data, int size) {
    /* This will create multiple versions, each requiring cache detection */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        data[i] = sum;
    }
}

/* Pattern 3: ifunc resolver for runtime dispatch */
static void (*compute_ifunc_ptr)(int *, int);

static void compute_default(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

static void *resolve_compute(void) {
    /* This resolver forces CPU detection */
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_core2;
    } else {
        return compute_default;
    }
}

/* ifunc function declaration */
void compute_ifunc(int *data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern 4: Direct CPUID inline assembly */
static void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB Descriptors */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (2), "c" (0));
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (4), "c" (0));
    
    /* Additional leaf 4 queries for different cache levels */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (4), "c" (i));
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_cache(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query CPUID cache information */
    query_cpuid_cache_info();
    
    /* Print cache line size for verification */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("Constructor: L1 cache line size: %ld bytes\n", cache_line);
}

/* Cache-sensitive computation */
static void cache_sensitive_computation(void) {
    /* Array sizes matching specific cache sizes from the switch cases */
    const int size_8kb = 2048;      /* 8KB / sizeof(int) for 32-bit ints */
    const int size_16kb = 4096;     /* 16KB */
    const int size_32kb = 8192;     /* 32KB */
    const int size_64kb = 16384;    /* 64KB */
    const int size_128kb = 32768;   /* 128KB */
    const int size_256kb = 65536;   /* 256KB */
    const int size_512kb = 131072;  /* 512KB */
    const int size_1mb = 262144;    /* 1MB */
    const int size_2mb = 524288;    /* 2MB */
    const int size_4mb = 1048576;   /* 4MB */
    const int size_6mb = 1572864;   /* 6MB */
    
    /* Allocate arrays with different cache-sized patterns */
    int *data1 = malloc(size_8kb * sizeof(int));
    int *data2 = malloc(size_16kb * sizeof(int));
    int *data3 = malloc(size_32kb * sizeof(int));
    int *data4 = malloc(size_64kb * sizeof(int));
    
    if (!data1 || !data2 || !data3 || !data4) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize data */
    for (int i = 0; i < size_8kb; i++) data1[i] = i % 256;
    for (int i = 0; i < size_16kb; i++) data2[i] = i % 512;
    for (int i = 0; i < size_32kb; i++) data3[i] = i % 1024;
    for (int i = 0; i < size_64kb; i++) data4[i] = i % 2048;
    
    /* Use __builtin_prefetch with different locality hints */
    for (int i = 0; i < size_8kb; i += 16) {
        __builtin_prefetch(&data1[i + 32], 0, 0);  /* Read, low locality */
        __builtin_prefetch(&data1[i + 64], 0, 3);  /* Read, high locality */
    }
    
    /* Call all different compute functions */
    compute_core2(data1, size_8kb);
    compute_nehalem(data2, size_16kb);
    compute_sandybridge(data3, size_32kb);
    compute_ivybridge(data4, size_64kb);
    
    /* Call multiversion function */
    compute_multiversion(data1, size_8kb);
    
    /* Call ifunc function */
    compute_ifunc(data2, size_16kb);
    
    /* Clean up */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
}

#else /* Non-x86 fallback */

/* Dummy implementations for non-x86 platforms */
static void cache_sensitive_computation(void) {
    printf("Non-x86 platform: cache detection not applicable\n");
}

#endif /* x86 guard */

/* Main function with extensive CPU checks */
int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Force CPU detection through builtins */
    __builtin_cpu_init();
    
    /* Check for various Intel CPUs that may have different cache descriptors */
    if (__builtin_cpu_is("core2")) {
        printf("CPU detected: Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU detected: Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU detected: Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU detected: Ivy Bridge\n");
    }
    if (__builtin_cpu_is("haswell")) {
        printf("CPU detected: Haswell\n");
    }
    if (__builtin_cpu_is("skylake")) {
        printf("CPU detected: Skylake\n");
    }
    
    /* Check CPU features that correlate with cache descriptors */
    printf("CPU features:\n");
    if (__builtin_cpu_supports("sse2")) printf("  SSE2\n");
    if (__builtin_cpu_supports("sse3")) printf("  SSE3\n");
    if (__builtin_cpu_supports("ssse3")) printf("  SSSE3\n");
    if (__builtin_cpu_supports("sse4.1")) printf("  SSE4.1\n");
    if (__builtin_cpu_supports("sse4.2")) printf("  SSE4.2\n");
    if (__builtin_cpu_supports("avx")) printf("  AVX\n");
    if (__builtin_cpu_supports("avx2")) printf("  AVX2\n");
    
    /* Compile-time assertion for x86 */
    _Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
                   "Expected 32-bit or 64-bit x86");
    
    /* Perform cache-sensitive computation */
    cache_sensitive_computation();
    
    /* Query system cache information at runtime */
    printf("Cache information:\n");
    printf("  L1 line size: %ld\n", sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
    printf("  L1 size: %ld\n", sysconf(_SC_LEVEL1_DCACHE_SIZE));
    printf("  L2 size: %ld\n", sysconf(_SC_LEVEL2_CACHE_SIZE));
    
    /* Simple checksum computation to verify program works */
    int array[1024];
    int sum = 0;
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
        sum += array[i];
    }
    
    printf("Computed checksum: %d (expected: %d)\n", sum, 1023*1024/2);
    
    return 0;
#else
    printf("This test is for x86 platforms only\n");
    return 0;
#endif
}
