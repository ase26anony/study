/* test_cache_detection.c
 * This program is designed to trigger GCC driver's CPU cache detection logic
 * for specific Intel cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.)
 * The goal is to exercise the switch-case block in driver-i386.cc lines 127-244
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* CPUID inline assembly helper */
static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, 
                         uint32_t *ecx, uint32_t *edx) {
    asm volatile("cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(0));
}

/* Force CPUID leaf 2 (cache descriptors) execution */
__attribute__((constructor)) 
static void init_cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Execute CPUID leaf 2 - Cache Descriptors */
    cpuid(2, &eax, &ebx, &ecx, &edx);
    
    /* Execute CPUID leaf 4 - Deterministic Cache Parameters */
    for (int i = 0; i < 10; i++) {
        cpuid(4, &eax, &ebx, &ecx, &edx);
        if ((eax & 0x1F) == 0) break; /* No more cache levels */
    }
}

/* Pattern A: Function multiversioning with different target attributes */
__attribute__((target("arch=core2")))
static int compute_core2(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * 3;
    }
    return sum;
}

__attribute__((target("arch=nehalem")))
static int compute_nehalem(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * 5;
    }
    return sum;
}

__attribute__((target("arch=sandybridge")))
static int compute_sandybridge(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * 7;
    }
    return sum;
}

__attribute__((target("arch=ivybridge")))
static int compute_ivybridge(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * 11;
    }
    return sum;
}

/* Pattern B: ifunc resolver for runtime dispatch */
static int (*compute_optimal)(int *, int);

static void *resolve_compute(void) {
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
    return compute_core2;
}

__attribute__((ifunc("resolve_compute")))
int compute_multiversion(int *data, int size);

/* Pattern C: Target clones for multiple architectures */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
static int compute_clones(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * 13;
    }
    return sum;
}

/* Pattern D: Cache-sensitive computation with prefetching */
static void cache_sensitive_computation(void) {
    /* Arrays sized to match various cache sizes from the uncovered lines */
    const int size_8kb = 2048;      /* 8KB / 4 bytes per int */
    const int size_16kb = 4096;     /* 16KB / 4 */
    const int size_32kb = 8192;     /* 32KB / 4 */
    const int size_64kb = 16384;    /* 64KB / 4 */
    const int size_128kb = 32768;   /* 128KB / 4 */
    const int size_256kb = 65536;   /* 256KB / 4 */
    const int size_512kb = 131072;  /* 512KB / 4 */
    const int size_1mb = 262144;    /* 1MB / 4 */
    const int size_2mb = 524288;    /* 2MB / 4 */
    const int size_4mb = 1048576;   /* 4MB / 4 */
    const int size_6mb = 1572864;   /* 6MB / 4 */
    
    int *data8k = malloc(size_8kb * sizeof(int));
    int *data16k = malloc(size_16kb * sizeof(int));
    int *data32k = malloc(size_32kb * sizeof(int));
    int *data64k = malloc(size_64kb * sizeof(int));
    int *data128k = malloc(size_128kb * sizeof(int));
    int *data256k = malloc(size_256kb * sizeof(int));
    int *data512k = malloc(size_512kb * sizeof(int));
    int *data1m = malloc(size_1mb * sizeof(int));
    
    if (!data8k || !data16k || !data32k || !data64k || 
        !data128k || !data256k || !data512k || !data1m) {
        return;
    }
    
    /* Initialize data */
    for (int i = 0; i < size_8kb; i++) data8k[i] = i & 0xFF;
    for (int i = 0; i < size_16kb; i++) data16k[i] = i & 0xFF;
    for (int i = 0; i < size_32kb; i++) data32k[i] = i & 0xFF;
    for (int i = 0; i < size_64kb; i++) data64k[i] = i & 0xFF;
    for (int i = 0; i < size_128kb; i++) data128k[i] = i & 0xFF;
    for (int i = 0; i < size_256kb; i++) data256k[i] = i & 0xFF;
    for (int i = 0; i < size_512kb; i++) data512k[i] = i & 0xFF;
    for (int i = 0; i < size_1mb; i++) data1m[i] = i & 0xFF;
    
    /* Perform cache-sensitive computations with prefetching */
    int sum = 0;
    
    /* 8KB L1 cache test (cases: 0x0a, 0x66) */
    for (int i = 0; i < size_8kb; i += 8) {
        __builtin_prefetch(&data8k[i + 32], 0, 3); /* High temporal locality */
        sum += data8k[i];
    }
    
    /* 16KB L1 cache test (cases: 0x0c, 0x0d, 0x67) */
    for (int i = 0; i < size_16kb; i += 16) {
        __builtin_prefetch(&data16k[i + 64], 0, 2); /* Medium locality */
        sum += data16k[i];
    }
    
    /* 32KB L1 cache test (cases: 0x2c, 0x68) */
    for (int i = 0; i < size_32kb; i += 32) {
        __builtin_prefetch(&data32k[i + 128], 0, 1); /* Low locality */
        sum += data32k[i];
    }
    
    /* 64KB-1MB L2 cache tests (various cases from uncovered lines) */
    for (int i = 0; i < size_64kb; i += 64) {
        sum += data64k[i];
    }
    
    for (int i = 0; i < size_128kb; i += 128) {
        sum += data128k[i];
    }
    
    for (int i = 0; i < size_256kb; i += 256) {
        sum += data256k[i];
    }
    
    for (int i = 0; i < size_512kb; i += 512) {
        sum += data512k[i];
    }
    
    for (int i = 0; i < size_1mb; i += 1024) {
        sum += data1m[i];
    }
    
    /* Matrix multiplication - cache blocking optimization */
    const int N = 256; /* Fits in L2 cache for many configurations */
    int A[N][N], B[N][N], C[N][N];
    
    /* Initialize matrices */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = i + j;
            B[i][j] = i - j;
            C[i][j] = 0;
        }
    }
    
    /* Cache-blocked matrix multiplication */
    const int BLOCK = 32; /* Typical cache block size */
    for (int i0 = 0; i0 < N; i0 += BLOCK) {
        for (int j0 = 0; j0 < N; j0 += BLOCK) {
            for (int k0 = 0; k0 < N; k0 += BLOCK) {
                for (int i = i0; i < i0 + BLOCK && i < N; i++) {
                    for (int j = j0; j < j0 + BLOCK && j < N; j++) {
                        for (int k = k0; k < k0 + BLOCK && k < N; k++) {
                            C[i][j] += A[i][k] * B[k][j];
                        }
                    }
                }
            }
        }
    }
    
    /* Use the result to avoid dead code elimination */
    volatile int result = C[N-1][N-1];
    (void)result;
    
    free(data8k);
    free(data16k);
    free(data32k);
    free(data64k);
    free(data128k);
    free(data256k);
    free(data512k);
    free(data1m);
}

#else
/* Non-x86 fallback */
static void init_cpuid_cache_info(void) {}
static int compute_multiversion(int *data, int size) { return 0; }
static int compute_clones(int *data, int size) { return 0; }
static void cache_sensitive_computation(void) {}
#endif

/* Compile-time assertion for x86 features */
#ifdef __x86_64__
_Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86_64");
#endif

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Pattern B: Extensive use of CPU detection builtins */
    printf("CPU Detection Results:\n");
    
    if (__builtin_cpu_is("intel")) {
        printf("  Vendor: Intel\n");
    }
    if (__builtin_cpu_is("core2")) {
        printf("  Microarchitecture: Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("  Microarchitecture: Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("  Microarchitecture: Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("  Microarchitecture: Ivy Bridge\n");
    }
    
    printf("CPU Features:\n");
    if (__builtin_cpu_supports("sse")) printf("  SSE\n");
    if (__builtin_cpu_supports("sse2")) printf("  SSE2\n");
    if (__builtin_cpu_supports("sse3")) printf("  SSE3\n");
    if (__builtin_cpu_supports("ssse3")) printf("  SSSE3\n");
    if (__builtin_cpu_supports("sse4.1")) printf("  SSE4.1\n");
    if (__builtin_cpu_supports("sse4.2")) printf("  SSE4.2\n");
    if (__builtin_cpu_supports("avx")) printf("  AVX\n");
    if (__builtin_cpu_supports("avx2")) printf("  AVX2\n");
    
    /* Runtime cache information */
    printf("\nCache Information (via sysconf):\n");
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    printf("  L1 DCache line size: %ld\n", sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
#endif
#ifdef _SC_LEVEL1_DCACHE_SIZE
    printf("  L1 DCache size: %ld\n", sysconf(_SC_LEVEL1_DCACHE_SIZE));
#endif
#ifdef _SC_LEVEL2_CACHE_SIZE
    printf("  L2 Cache size: %ld\n", sysconf(_SC_LEVEL2_CACHE_SIZE));
#endif
    
    /* Test data for multiversion functions */
    const int test_size = 10000;
    int *test_data = malloc(test_size * sizeof(int));
    for (int i = 0; i < test_size; i++) {
        test_data[i] = i % 100;
    }
    
    /* Execute all multiversion functions */
    int result1 = compute_multiversion(test_data, test_size);
    int result2 = compute_clones(test_data, test_size);
    
    /* Execute cache-sensitive computation */
    cache_sensitive_computation();
    
    /* Use results to avoid dead code elimination */
    printf("\nComputation Results:\n");
    printf("  Multiversion result: %d\n", result1);
    printf("  Clones result: %d\n", result2);
    
    free(test_data);
    
    /* Final checksum to verify program executed correctly */
    unsigned long checksum = result1 + result2;
    printf("\nFinal checksum: %lu\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
#else
    printf("Non-x86 architecture - cache detection test skipped\n");
    return 0;
#endif
}
