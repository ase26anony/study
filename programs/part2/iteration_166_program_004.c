/* test_cache_detection.c - Trigger GCC driver CPU cache detection logic */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Multiple target attributes for different Intel architectures */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    /* Use cache-sized loops for 8KB/16KB L1 cache */
    for (int i = 0; i < size; i += 32) {  /* 32 ints = 128 bytes */
        data[i] = i * 2;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    /* Different access pattern for Nehalem cache */
    for (int i = 0; i < size; i += 16) {  /* 16 ints = 64 bytes */
        data[i] = i * 3;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    /* Sandy Bridge specific pattern */
    for (int i = 0; i < size; i += 8) {  /* 8 ints = 32 bytes */
        data[i] = i * 4;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    /* Ivy Bridge pattern */
    for (int i = 0; i < size; i += 64) {  /* 64 ints = 256 bytes */
        data[i] = i * 5;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i;
    }
}

static void* resolve_compute(void) {
    /* Force CPU detection through builtins */
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
        return compute_default;
    }
}

/* ifunc function */
void compute_optimized(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Direct CPUID queries */
static void query_cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        ecx = i;
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(ecx));
    }
}

/* Pattern D: Cache-sensitive computations with prefetching */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=haswell")))
void cache_sensitive_computation(int* matrix, int size) {
    const int CACHE_LINE_SIZE = 64;  /* Common cache line size */
    const int L1_SIZE_KB = 32;       /* Common L1 cache size */
    const int L1_ELEMS = (L1_SIZE_KB * 1024) / sizeof(int);
    
    /* Tile computation to fit in L1 cache */
    for (int i = 0; i < size; i += L1_ELEMS) {
        int limit = i + L1_ELEMS;
        if (limit > size) limit = size;
        
        for (int j = i; j < limit; j += CACHE_LINE_SIZE / sizeof(int)) {
            /* Prefetch next cache line */
            __builtin_prefetch(&matrix[j + CACHE_LINE_SIZE / sizeof(int)], 0, 3);
            
            /* Simple computation */
            matrix[j] = matrix[j] * 2 + 1;
        }
    }
}

/* Constructor to force early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query CPUID directly */
    query_cpuid_cache_info();
    
    /* Check various CPU features to trigger detection */
    if (__builtin_cpu_is("intel")) {
        /* Check specific cache-related features */
        if (__builtin_cpu_supports("sse2")) {
            /* Compile-time assertion */
            _Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
                          "Pointer size check");
        }
    }
}

/* Main verification function */
static int verify_computation(int* data, int size) {
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += data[i];
    }
    return checksum;
}

#else
/* Non-x86 fallback */
void compute_optimized(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i;
    }
}

void cache_sensitive_computation(int* matrix, int size) {
    for (int i = 0; i < size; i++) {
        matrix[i] = matrix[i] * 2 + 1;
    }
}
#endif

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Pattern B: Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
    /* Check for various Intel CPUs to trigger cache detection */
    if (__builtin_cpu_is("core2")) {
        printf("CPU: Core 2 detected\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU: Nehalem detected\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU: Sandy Bridge detected\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU: Ivy Bridge detected\n");
    }
    
    /* Check cache-related features */
    printf("SSE2: %d\n", __builtin_cpu_supports("sse2"));
    printf("SSE3: %d\n", __builtin_cpu_supports("sse3"));
    printf("SSSE3: %d\n", __builtin_cpu_supports("ssse3"));
    printf("SSE4.1: %d\n", __builtin_cpu_supports("sse4.1"));
    printf("SSE4.2: %d\n", __builtin_cpu_supports("sse4.2"));
    printf("AVX: %d\n", __builtin_cpu_supports("avx"));
    
    /* Get system cache info */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("Cache line size: %ld bytes\n", cache_line);
#endif

    /* Create test data with various cache-sized arrays */
    const int TEST_SIZES[] = {
        2048,   /* 8KB with 4-byte ints */
        4096,   /* 16KB */
        8192,   /* 32KB */
        16384,  /* 64KB */
        32768   /* 128KB */
    };
    
    int total_checksum = 0;
    
    for (int s = 0; s < sizeof(TEST_SIZES)/sizeof(TEST_SIZES[0]); s++) {
        int size = TEST_SIZES[s];
        int* data = malloc(size * sizeof(int));
        
        if (!data) continue;
        
        /* Initialize */
        for (int i = 0; i < size; i++) {
            data[i] = i % 256;
        }
        
        /* Use ifunc-resolved function */
        compute_optimized(data, size);
        
        /* Use cache-sensitive computation */
        cache_sensitive_computation(data, size);
        
        /* Verify */
        int checksum = verify_computation(data, size);
        total_checksum += checksum;
        
        printf("Size %d: checksum = %d\n", size, checksum);
        
        free(data);
    }
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Simple matrix multiplication for cache optimization */
    const int N = 256;  /* Fits in L2 cache for many configurations */
    int* A = malloc(N * N * sizeof(int));
    int* B = malloc(N * N * sizeof(int));
    int* C = malloc(N * N * sizeof(int));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < N * N; i++) {
            A[i] = i % 100;
            B[i] = (i + 1) % 100;
            C[i] = 0;
        }
        
        /* Cache-optimized matrix multiplication */
        const int BLOCK_SIZE = 32;  /* Cache blocking */
        for (int i = 0; i < N; i += BLOCK_SIZE) {
            for (int j = 0; j < N; j += BLOCK_SIZE) {
                for (int k = 0; k < N; k += BLOCK_SIZE) {
                    for (int ii = i; ii < i + BLOCK_SIZE && ii < N; ii++) {
                        for (int jj = j; jj < j + BLOCK_SIZE && jj < N; jj++) {
                            int sum = C[ii * N + jj];
                            for (int kk = k; kk < k + BLOCK_SIZE && kk < N; kk++) {
                                sum += A[ii * N + kk] * B[kk * N + jj];
                            }
                            C[ii * N + jj] = sum;
                        }
                    }
                }
            }
        }
        
        /* Compute final checksum */
        int matrix_checksum = 0;
        for (int i = 0; i < N * N; i++) {
            matrix_checksum += C[i];
        }
        printf("Matrix checksum: %d\n", matrix_checksum);
    }
    
    free(A);
    free(B);
    free(C);
    
    return 0;
}
