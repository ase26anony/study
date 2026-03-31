/* test_cache_detection.c - Comprehensive test for Intel CPU cache descriptor detection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target attributes */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i += 8) {
        /* Use prefetch to encourage cache model usage */
        __builtin_prefetch(&data[i + 32], 0, 3);
        data[i] = data[i] * 5 + 11;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    /* Array sized to match specific cache sizes from uncovered lines */
    int temp[2048]; /* 8KB for 32-bit ints - matches some L1 cache sizes */
    for (int i = 0; i < size && i < 2048; i++) {
        temp[i] = data[i];
        data[i] = temp[i] * 7 + 13;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    /* Use different access pattern */
    for (int i = size - 1; i >= 0; i--) {
        data[i] = data[i] * 11 + 17;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_impl)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

static void* resolve_compute() {
    /* Force CPU detection through builtins */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse3")) {
        return compute_core2;
    }
    return compute_default;
}

/* ifunc function */
void compute_optimized(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Direct CPUID queries */
static void query_cpuid_cache_info() {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(2), "c"(0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 10; i++) { /* Query multiple cache levels */
        asm volatile ("cpuid" 
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                      : "a"(4), "c"(i));
        
        /* Break when no more cache levels */
        if ((eax & 0x1F) == 0) break;
    }
}

/* Constructor to run early CPU detection */
__attribute__((constructor))
static void init_cpu_cache() {
    query_cpuid_cache_info();
    
    /* Check various CPU features to trigger detection */
    __builtin_cpu_init();
    
    /* Test for specific Intel CPUs that use the cache descriptors */
    if (__builtin_cpu_is("core2")) {
        printf("CPU detected as Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU detected as Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU detected as Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU detected as Ivy Bridge\n");
    }
    
    /* Check cache line size at runtime */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
}

/* Pattern D: Cache-sensitive computation */
static void cache_sensitive_computation() {
    /* Create arrays sized to specific cache sizes from uncovered lines */
    
    /* 8KB array - matches 0x0a descriptor */
    int array_8k[2048];  /* 2048 * 4 bytes = 8192 bytes */
    
    /* 16KB array - matches 0x0c, 0x0d descriptors */
    int array_16k[4096]; /* 4096 * 4 = 16384 bytes */
    
    /* 32KB array - matches 0x2c descriptor */
    int array_32k[8192]; /* 8192 * 4 = 32768 bytes */
    
    /* Initialize arrays */
    for (int i = 0; i < 2048; i++) array_8k[i] = i;
    for (int i = 0; i < 4096; i++) array_16k[i] = i * 2;
    for (int i = 0; i < 8192; i++) array_32k[i] = i * 3;
    
    /* Perform computations with different access patterns */
    
    /* Sequential access - good for prefetch */
    int sum1 = 0;
    for (int i = 0; i < 2048; i += 64/sizeof(int)) { /* 64-byte cache line */
        sum1 += array_8k[i];
        __builtin_prefetch(&array_8k[i + 64/sizeof(int)], 0, 0);
    }
    
    /* Strided access - tests associativity */
    int sum2 = 0;
    for (int i = 0; i < 4096; i += 8) {
        sum2 += array_16k[i];
    }
    
    /* Reverse access */
    int sum3 = 0;
    for (int i = 8191; i >= 0; i -= 16) {
        sum3 += array_32k[i];
    }
    
    /* Use all compute functions */
    compute_core2(array_8k, 2048);
    compute_nehalem(array_16k, 4096);
    compute_sandybridge(array_32k, 8192);
    compute_ivybridge(array_8k, 2048);
    
    /* Use ifunc version */
    compute_optimized(array_16k, 4096);
    
    printf("Cache-sensitive computation results: %d, %d, %d\n", sum1, sum2, sum3);
}

#else
/* Non-x86 fallback */
static void cache_sensitive_computation() {
    printf("Non-x86 architecture - using fallback computation\n");
    int array[1024];
    int sum = 0;
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
        sum += array[i];
    }
    printf("Fallback sum: %d\n", sum);
}
#endif

/* Compile-time assertion for x86 features */
#if defined(__i386__) || defined(__x86_64__)
_Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
#endif

/* Main function with extensive CPU feature checks */
int main() {
#if defined(__i386__) || defined(__x86_64__)
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check many CPU features to ensure driver runs detection */
    printf("CPU Feature checks:\n");
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("  SSSE3: %s\n", __builtin_cpu_supports("ssse3") ? "yes" : "no");
    printf("  SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("  SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("  AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    
    /* Additional CPUID query */
    unsigned int eax, ebx, ecx, edx;
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(1), "c"(0));
    printf("CPUID(1): eax=0x%08x\n", eax);
#endif
    
    /* Perform cache-sensitive computation */
    cache_sensitive_computation();
    
    /* Final verification computation */
    const int N = 1024;
    int* matrix = (int*)malloc(N * N * sizeof(int));
    if (!matrix) return 1;
    
    /* Initialize matrix */
    for (int i = 0; i < N * N; i++) {
        matrix[i] = i % 100;
    }
    
    /* Simple matrix operation - encourages cache optimization */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            checksum += matrix[i * N + j];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    free(matrix);
    
    return 0;
}
