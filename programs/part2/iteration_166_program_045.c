/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPUID cache detection
 * Specifically targets the large switch-case block in driver-i386.cc (lines 127-244)
 * 
 * Compilation recommendations:
 * 1. For standard x86 cache detection:
 *    gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_executable
 * 2. For 32-bit i386 driver path:
 *    gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 * 3. For aggressive CPU feature usage:
 *    gcc -O3 -march=native -mtune=native -fprofile-arcs -ftest-coverage -fno-inline -funroll-loops test_target.c -o test_target_aggressive
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)
/* Pattern A: Function multiversioning with target attributes */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 11;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 13;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 17;
    }
}

/* Pattern A: Multi-architecture compilation with target_clones */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=haswell")))
void multiversion_compute(float* a, float* b, float* c, int n) {
    /* Simple matrix-vector multiplication tuned for different cache sizes */
    for (int i = 0; i < n; i++) {
        c[i] = 0.0f;
        for (int j = 0; j < n; j++) {
            c[i] += a[i * n + j] * b[j];
        }
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_impl)(int*, int) = NULL;

static void init_compute_impl(void) __attribute__((constructor));
static void init_compute_impl(void) {
    /* Force CPU detection early */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        compute_impl = compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        compute_impl = compute_nehalem;
    } else if (__builtin_cpu_supports("sse2")) {
        compute_impl = compute_core2;
    } else {
        compute_impl = compute_core2; /* fallback */
    }
}

/* Pattern C: Direct CPUID queries */
static void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(0)
    );
}

static void query_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (Intel) */
    cpuid(2, &eax, &ebx, &ecx, &edx);
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 32; i++) {
        cpuid(4, &eax, &ebx, &ecx, &edx);
        if ((eax & 0x1F) == 0) break; /* No more caches */
    }
}

/* Pattern D: Cache-sensitive computations */
__attribute__((optimize("O3")))
void cache_sensitive_computation(void) {
    /* Arrays sized to match specific cache line configurations from the switch */
    
    /* 8KB array (matches 0x0a: 8KB L1) */
    int array_8k[2048]; /* 2048 * 4 bytes = 8192 bytes */
    
    /* 16KB array (matches 0x0c, 0x0d: 16KB L1) */
    int array_16k[4096]; /* 4096 * 4 = 16384 bytes */
    
    /* 32KB array (matches 0x2c: 32KB L1) */
    int array_32k[8192]; /* 8192 * 4 = 32768 bytes */
    
    /* 64KB array (matches various L2 sizes) */
    int array_64k[16384]; /* 16384 * 4 = 65536 bytes */
    
    /* Initialize arrays */
    for (int i = 0; i < 2048; i++) array_8k[i] = i;
    for (int i = 0; i < 4096; i++) array_16k[i] = i * 2;
    for (int i = 0; i < 8192; i++) array_32k[i] = i * 3;
    for (int i = 0; i < 16384; i++) array_64k[i] = i * 5;
    
    /* Pattern D: Use __builtin_prefetch with different cache line hints */
    for (int i = 0; i < 2048; i += 8) { /* 8 ints = 32 bytes */
        __builtin_prefetch(&array_8k[i + 8], 0, 3); /* High temporal locality */
    }
    
    for (int i = 0; i < 4096; i += 16) { /* 16 ints = 64 bytes */
        __builtin_prefetch(&array_16k[i + 16], 0, 1); /* Low temporal locality */
    }
    
    /* Compute checksums to ensure computation happens */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    /* Access patterns that stress different cache levels */
    for (int i = 0; i < 2048; i++) {
        sum1 += array_8k[i];
        if (i % 32 == 0) { /* Every 128 bytes */
            sum2 += array_16k[i % 4096];
        }
        if (i % 64 == 0) { /* Every 256 bytes */
            sum3 += array_32k[i % 8192];
        }
        if (i % 128 == 0) { /* Every 512 bytes */
            sum4 += array_64k[i % 16384];
        }
    }
    
    /* Use results to prevent optimization */
    asm volatile ("" : : "r"(sum1), "r"(sum2), "r"(sum3), "r"(sum4));
}

/* Validation and debugging */
static void print_cache_info(void) {
    printf("Cache information:\n");
    
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    printf("L1 DCache line size: %ld\n", sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
#endif
#ifdef _SC_LEVEL1_DCACHE_SIZE
    printf("L1 DCache size: %ld\n", sysconf(_SC_LEVEL1_DCACHE_SIZE));
#endif
#ifdef _SC_LEVEL2_CACHE_SIZE
    printf("L2 Cache size: %ld\n", sysconf(_SC_LEVEL2_CACHE_SIZE));
#endif
    
    /* Pattern B: Extensive use of __builtin_cpu_is checks */
    const char* cpus[] = {
        "intel", "core2", "nehalem", "sandybridge", 
        "ivybridge", "haswell", "skylake", "atom"
    };
    
    printf("CPU detection:\n");
    for (size_t i = 0; i < sizeof(cpus)/sizeof(cpus[0]); i++) {
        if (__builtin_cpu_is(cpus[i])) {
            printf("  Detected: %s\n", cpus[i]);
        }
    }
    
    /* Check specific features that correlate with cache descriptors */
    const char* features[] = {
        "sse", "sse2", "sse3", "ssse3", "sse4.1", "sse4.2",
        "avx", "avx2", "fma", "aes", "pclmulqdq"
    };
    
    printf("CPU features:\n");
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf("  %s: supported\n", features[i]);
        }
    }
}

/* Main computation - matrix multiplication with cache tiling */
void matrix_multiply_tiled(const float* a, const float* b, float* c, 
                          int n, int tile_size) {
    for (int i = 0; i < n; i += tile_size) {
        for (int j = 0; j < n; j += tile_size) {
            for (int k = 0; k < n; k += tile_size) {
                /* Mini matrix multiplication within tile */
                for (int ii = i; ii < i + tile_size && ii < n; ii++) {
                    for (int kk = k; kk < k + tile_size && kk < n; kk++) {
                        float a_ik = a[ii * n + kk];
                        for (int jj = j; jj < j + tile_size && jj < n; jj++) {
                            c[ii * n + jj] += a_ik * b[kk * n + jj];
                        }
                    }
                }
            }
        }
    }
}

#else
/* Non-x86 fallback implementations */
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = data[i] * 3 + 7;
}

void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = data[i] * 5 - 11;
}

void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = data[i] * 7 + 13;
}

void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = data[i] * 11 - 17;
}

void query_cache_descriptors(void) {
    /* No-op for non-x86 */
}

void cache_sensitive_computation(void) {
    /* Simple computation for non-x86 */
    int array[1024];
    for (int i = 0; i < 1024; i++) array[i] = i;
    int sum = 0;
    for (int i = 0; i < 1024; i++) sum += array[i];
    asm volatile ("" : : "r"(sum));
}

void print_cache_info(void) {
    printf("Non-x86 architecture - cache detection not applicable\n");
}

void matrix_multiply_tiled(const float* a, const float* b, float* c, 
                          int n, int tile_size) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            c[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}
#endif

/* Main function with comprehensive CPU feature usage */
int main(void) {
    printf("Starting cache detection test...\n");
    
#if defined(__i386__) || defined(__x86_64__)
    /* Compile-time assertion for x86 features */
    _Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
#endif
    
    /* Pattern B: Extensive __builtin_cpu_is usage */
    __builtin_cpu_init();
    
    if (__builtin_cpu_is("intel")) {
        printf("Intel CPU detected\n");
        
        /* Check various Intel microarchitectures */
        if (__builtin_cpu_is("core2")) {
            printf("Core 2 microarchitecture\n");
        }
        if (__builtin_cpu_is("nehalem")) {
            printf("Nehalem microarchitecture\n");
        }
        if (__builtin_cpu_is("sandybridge")) {
            printf("Sandy Bridge microarchitecture\n");
        }
        if (__builtin_cpu_is("ivybridge")) {
            printf("Ivy Bridge microarchitecture\n");
        }
    }
    
    /* Pattern C: Direct CPUID queries */
    query_cache_descriptors();
    
    /* Pattern D: Cache-sensitive computations */
    cache_sensitive_computation();
    
    /* Use all target-specific functions */
    int test_data[1024];
    for (int i = 0; i < 1024; i++) test_data[i] = i;
    
    compute_core2(test_data, 1024);
    compute_nehalem(test_data, 1024);
    compute_sandybridge(test_data, 1024);
    compute_ivybridge(test_data, 1024);
    
    /* Multi-version function call */
    float a[64], b[64], c[64];
    for (int i = 0; i < 64; i++) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
        c[i] = 0.0f;
    }
    multiversion_compute(a, b, c, 8);
    
    /* Matrix multiplication with different tile sizes */
    const int MATRIX_SIZE = 128;
    float* mat_a = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    float* mat_b = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    float* mat_c = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    
    if (mat_a && mat_b && mat_c) {
        /* Initialize matrices */
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            mat_a[i] = (i % 100) * 0.01f;
            mat_b[i] = ((i + 1) % 100) * 0.01f;
            mat_c[i] = 0.0f;
        }
        
        /* Try different tile sizes corresponding to cache line sizes */
        matrix_multiply_tiled(mat_a, mat_b, mat_c, MATRIX_SIZE, 8);   /* 32 bytes */
        matrix_multiply_tiled(mat_a, mat_b, mat_c, MATRIX_SIZE, 16);  /* 64 bytes */
        matrix_multiply_tiled(mat_a, mat_b, mat_c, MATRIX_SIZE, 32);  /* 128 bytes */
        
        /* Compute checksum */
        float checksum = 0.0f;
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            checksum += mat_c[i];
        }
        printf("Matrix multiplication checksum: %f\n", checksum);
    }
    
    free(mat_a);
    free(mat_b);
    free(mat_c);
    
    /* Print cache information */
    print_cache_info();
    
    /* Final checksum of test_data */
    int final_sum = 0;
    for (int i = 0; i < 1024; i++) {
        final_sum += test_data[i];
    }
    printf("Final checksum: %d\n", final_sum);
    
    return 0;
}
