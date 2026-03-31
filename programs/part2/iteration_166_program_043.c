/*
 * test_cache_detection.c
 * 
 * This program triggers GCC's CPU cache detection logic by using:
 * 1. Function multiversioning with target attributes
 * 2. ifunc resolvers that query CPU features
 * 3. CPUID inline assembly
 * 4. Cache-sensitive computations
 * 5. Builtin CPU feature checks
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Function 1: Core2 target - may trigger cache descriptors like 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Function 2: Nehalem target - may trigger cache descriptors like 0x0a, 0x0c, 0x0d */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 8) {
        sum += data[i];
    }
}

/* Function 3: Sandy Bridge target - may trigger cache descriptors like 0x2c, 0x3a, 0x3b */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] << 3) | (data[i] >> 29);
    }
}

/* Function 4: Ivy Bridge target - may trigger cache descriptors like 0x3c, 0x3d, 0x3e */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i += 4) {
        data[i] = data[i] ^ 0xAAAAAAAA;
    }
}

/* Multi-versioned function using target_clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void compute_multiversion(int* data, int size) {
    /* Default implementation */
    for (int i = 0; i < size; i++) {
        data[i] = data[i] + 1;
    }
}

/* ifunc resolver that selects implementation based on CPU features */
static void (*compute_ifunc_resolve(void))(int*, int) {
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
    return compute_multiversion;
}

/* ifunc function declaration */
void compute_ifunc(int* data, int size) 
    __attribute__((ifunc("compute_ifunc_resolve")));

/* Execute CPUID leaf 2 (cache descriptors) */
static void cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB Descriptor information */
    asm volatile (
        "cpuid\n\t"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
    }
}

/* Constructor that runs before main - forces early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    __builtin_cpu_init();
    cpuid_cache_info();
    
    /* Check various CPU features to trigger detection */
    int has_sse2 = __builtin_cpu_supports("sse2");
    int has_sse3 = __builtin_cpu_supports("sse3");
    int has_ssse3 = __builtin_cpu_supports("ssse3");
    int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Use results to prevent optimization */
    volatile int dummy = has_sse2 + has_sse3 + has_ssse3 + 
                        has_sse4_1 + has_sse4_2 + has_avx + has_avx2;
    (void)dummy;
}

/* Cache-sensitive matrix multiplication */
static void cache_sensitive_computation(void) {
    const int N = 256;  /* Size chosen to exercise different cache levels */
    int* A = malloc(N * N * sizeof(int));
    int* B = malloc(N * N * sizeof(int));
    int* C = malloc(N * N * sizeof(int));
    
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < N * N; i++) {
        A[i] = i % 100;
        B[i] = (i + 1) % 100;
        C[i] = 0;
    }
    
    /* Matrix multiplication with tiling for cache optimization */
    const int BLOCK = 16;  /* Block size for cache tiling */
    for (int i = 0; i < N; i += BLOCK) {
        for (int j = 0; j < N; j += BLOCK) {
            for (int k = 0; k < N; k += BLOCK) {
                for (int ii = i; ii < i + BLOCK && ii < N; ii++) {
                    for (int jj = j; jj < j + BLOCK && jj < N; jj++) {
                        int sum = C[ii * N + jj];
                        for (int kk = k; kk < k + BLOCK && kk < N; kk++) {
                            sum += A[ii * N + kk] * B[kk * N + jj];
                        }
                        C[ii * N + jj] = sum;
                    }
                }
            }
        }
    }
    
    /* Use prefetch hints with different cache locality levels */
    for (int i = 0; i < N * N; i += 64) {  /* 64-byte cache line */
        __builtin_prefetch(&C[i + 64], 0, 0);  /* Prefetch for read, low locality */
    }
    
    /* Compute checksum */
    long long checksum = 0;
    for (int i = 0; i < N * N; i++) {
        checksum += C[i];
    }
    
    volatile long long result = checksum;  /* Prevent optimization */
    (void)result;
    
    free(A); free(B); free(C);
}

/* Array sized to match specific cache sizes mentioned in uncovered lines */
static void exercise_cache_sizes(void) {
    /* 8KB array (matches 0x0a: 8KB L1 cache) */
    char array_8k[8 * 1024];
    /* 16KB array (matches 0x0c, 0x0d: 16KB L1 cache) */
    char array_16k[16 * 1024];
    /* 32KB array (matches 0x2c: 32KB L1 cache) */
    char array_32k[32 * 1024];
    /* 256KB array (matches 0x21: 256KB L2 cache) */
    static char array_256k[256 * 1024];
    /* 1MB array (matches 0x24: 1024KB L2 cache) */
    static char array_1mb[1024 * 1024];
    
    /* Access arrays in patterns to exercise cache */
    for (int i = 0; i < sizeof(array_8k); i += 32) {  /* 32-byte line */
        array_8k[i] = i & 0xFF;
    }
    
    for (int i = 0; i < sizeof(array_16k); i += 64) {  /* 64-byte line */
        array_16k[i] = i & 0xFF;
    }
    
    for (int i = 0; i < sizeof(array_32k); i += 64) {
        array_32k[i] = i & 0xFF;
    }
    
    for (int i = 0; i < sizeof(array_256k); i += 128) {
        array_256k[i] = i & 0xFF;
    }
    
    for (int i = 0; i < sizeof(array_1mb); i += 256) {
        array_1mb[i] = i & 0xFF;
    }
}

#else
/* Non-x86 fallback implementations */
void compute_core2(int* data, int size) { (void)data; (void)size; }
void compute_nehalem(int* data, int size) { (void)data; (void)size; }
void compute_sandybridge(int* data, int size) { (void)data; (void)size; }
void compute_ivybridge(int* data, int size) { (void)data; (void)size; }
void compute_multiversion(int* data, int size) { (void)data; (void)size; }
void compute_ifunc(int* data, int size) { (void)data; (void)size; }
static void cpuid_cache_info(void) {}
static void init_cpu_detection(void) {}
static void cache_sensitive_computation(void) {}
static void exercise_cache_sizes(void) {}
#endif

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Force CPU detection early */
    __builtin_cpu_init();
    
    /* Check for specific Intel CPUs to trigger cache detection */
    int is_core2 = __builtin_cpu_is("core2");
    int is_nehalem = __builtin_cpu_is("nehalem");
    int is_sandybridge = __builtin_cpu_is("sandybridge");
    int is_ivybridge = __builtin_cpu_is("ivybridge");
    
    /* Use results to prevent optimization */
    volatile int cpu_type = is_core2 + is_nehalem + is_sandybridge + is_ivybridge;
    (void)cpu_type;
    
    /* Compile-time assertion for x86 */
    _Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
                   "Expected 32-bit or 64-bit architecture");
    
    /* Get cache line size at runtime */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (cache_line > 0) {
        printf("Detected cache line size: %ld bytes\n", cache_line);
    }
#endif
    
    /* Create test data */
    const int DATA_SIZE = 10000;
    int* data = malloc(DATA_SIZE * sizeof(int));
    if (!data) {
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = i;
    }
    
    /* Call all versions of compute functions */
    compute_core2(data, DATA_SIZE);
    compute_nehalem(data, DATA_SIZE);
    compute_sandybridge(data, DATA_SIZE);
    compute_ivybridge(data, DATA_SIZE);
    compute_multiversion(data, DATA_SIZE);
    compute_ifunc(data, DATA_SIZE);
    
    /* Perform cache-sensitive computations */
    cache_sensitive_computation();
    exercise_cache_sizes();
    
    /* Compute final checksum */
    long long checksum = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        checksum += data[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    free(data);
    return 0;
}
