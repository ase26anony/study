/* test_cache_detection.c
 * Comprehensive test to trigger Intel CPU cache descriptor parsing in GCC driver
 * Targets uncovered lines 127-244 in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern 1: Multiple target attributes for different Intel architectures */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    /* Use cache-friendly access pattern */
    for (int i = 0; i < size; i += 16) {
        data[i] = i * 2;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    /* Different stride to potentially trigger different cache behavior */
    for (int i = 0; i < size; i += 32) {
        data[i] = i * 3;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    /* AVX-friendly stride */
    for (int i = 0; i < size; i += 64) {
        data[i] = i * 4;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    /* Different access pattern */
    for (int i = 0; i < size; i += 128) {
        data[i] = i * 5;
    }
}

/* Pattern 2: ifunc resolver for runtime dispatch */
typedef void (*compute_func_t)(int*, int);

static compute_func_t resolve_compute() {
    /* These builtins force CPU detection */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("ssse3")) {
        return compute_core2;
    } else {
        return compute_core2;
    }
}

void compute_dispatch(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern 3: Function with multiple target clones */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=haswell")))
void multiversion_compute(int* data, int size) {
    /* Matrix-like access pattern that benefits from cache optimization */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 64; j++) {
            data[(i * 64 + j) % size] += j;
        }
    }
}

/* Pattern 4: Constructor that runs CPUID queries early */
__attribute__((constructor))
static void early_cpu_detection() {
    /* Force CPUID execution through inline assembly */
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (triggers the switch cases) */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(0)
    );
    
    /* Additional leaves for comprehensive detection */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
}

/* Cache-sensitive computation */
static void cache_optimized_computation() {
    /* Array sizes matching various cache sizes from the switch cases */
    const int size_8kb = 2048;      /* 8KB / sizeof(int) */
    const int size_16kb = 4096;     /* 16KB / sizeof(int) */
    const int size_32kb = 8192;     /* 32KB / sizeof(int) */
    const int size_64kb = 16384;    /* 64KB / sizeof(int) */
    const int size_128kb = 32768;   /* 128KB / sizeof(int) */
    const int size_256kb = 65536;   /* 256KB / sizeof(int) */
    const int size_512kb = 131072;  /* 512KB / sizeof(int) */
    const int size_1mb = 262144;    /* 1MB / sizeof(int) */
    const int size_2mb = 524288;    /* 2MB / sizeof(int) */
    const int size_4mb = 1048576;   /* 4MB / sizeof(int) */
    const int size_6mb = 1572864;   /* 6MB / sizeof(int) */
    
    /* Allocate arrays matching cache sizes */
    int* data_8kb = malloc(size_8kb * sizeof(int));
    int* data_16kb = malloc(size_16kb * sizeof(int));
    int* data_32kb = malloc(size_32kb * sizeof(int));
    int* data_64kb = malloc(size_64kb * sizeof(int));
    
    if (!data_8kb || !data_16kb || !data_32kb || !data_64kb) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize data */
    for (int i = 0; i < size_64kb; i++) {
        if (i < size_8kb) data_8kb[i] = i;
        if (i < size_16kb) data_16kb[i] = i;
        if (i < size_32kb) data_32kb[i] = i;
        data_64kb[i] = i;
    }
    
    /* Pattern D: Use __builtin_prefetch with cache line hints */
    for (int i = 0; i < size_64kb; i += 16) {
        __builtin_prefetch(&data_64kb[i + 32], 0, 3); /* High temporal locality */
    }
    
    /* Execute all compute functions to trigger different code paths */
    compute_core2(data_8kb, size_8kb);
    compute_nehalem(data_16kb, size_16kb);
    compute_sandybridge(data_32kb, size_32kb);
    compute_ivybridge(data_64kb, size_64kb);
    
    /* Use ifunc dispatch */
    compute_dispatch(data_8kb, size_8kb);
    
    /* Use multiversion function */
    multiversion_compute(data_16kb, size_16kb);
    
    /* Cache-blocked matrix multiplication simulation */
    const int block_size = 32; /* Common cache line size */
    for (int i = 0; i < size_32kb; i += block_size) {
        for (int j = 0; j < size_32kb; j += block_size) {
            for (int ii = i; ii < i + block_size && ii < size_32kb; ii++) {
                for (int jj = j; jj < j + block_size && jj < size_32kb; jj++) {
                    data_32kb[ii] += data_32kb[jj] * 3;
                }
            }
        }
    }
    
    /* Compute checksum for validation */
    unsigned long long checksum = 0;
    for (int i = 0; i < size_64kb; i++) {
        checksum += data_64kb[i];
    }
    
    printf("Computed checksum: %llu\n", checksum);
    
    free(data_8kb);
    free(data_16kb);
    free(data_32kb);
    free(data_64kb);
}

#else
/* Non-x86 fallback */
static void cache_optimized_computation() {
    printf("Non-x86 architecture - cache detection not applicable\n");
}
#endif

int main() {
    /* Pattern B: Extensive use of CPU detection builtins */
#if defined(__i386__) || defined(__x86_64__)
    __builtin_cpu_init();
    
    /* Check for various Intel CPUs to trigger cache detection */
    if (__builtin_cpu_is("intel")) {
        printf("Intel CPU detected\n");
    }
    
    if (__builtin_cpu_is("core2")) {
        printf("Core 2 detected\n");
    }
    
    if (__builtin_cpu_is("nehalem")) {
        printf("Nehalem detected\n");
    }
    
    if (__builtin_cpu_is("sandybridge")) {
        printf("Sandy Bridge detected\n");
    }
    
    if (__builtin_cpu_is("ivybridge")) {
        printf("Ivy Bridge detected\n");
    }
    
    /* Check CPU features that correlate with cache descriptors */
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    
    if (__builtin_cpu_supports("sse3")) {
        printf("SSE3 supported\n");
    }
    
    if (__builtin_cpu_supports("ssse3")) {
        printf("SSSE3 supported\n");
    }
    
    if (__builtin_cpu_supports("sse4.1")) {
        printf("SSE4.1 supported\n");
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        printf("SSE4.2 supported\n");
    }
    
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
    }
    
    /* Get cache line size via sysconf */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
#endif
    
    /* Execute cache-sensitive computation */
    cache_optimized_computation();
    
    /* Additional inline assembly for CPUID leaf 2 (cache descriptors) */
#if defined(__i386__) || defined(__x86_64__)
    {
        unsigned int eax, ebx, ecx, edx;
        
        /* Execute CPUID leaf 2 multiple times to ensure detection */
        for (int i = 0; i < 3; i++) {
            asm volatile (
                "cpuid\n\t"
                : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                : "a"(2)
                : "memory"
            );
            
            /* Print first byte of cache descriptor (may be 0x0a, 0x0c, etc.) */
            unsigned char cache_desc = (eax >> 8) & 0xFF;
            printf("CPUID leaf 2 iteration %d: first descriptor byte: 0x%02x\n", 
                   i, cache_desc);
        }
    }
#endif
    
    return 0;
}
