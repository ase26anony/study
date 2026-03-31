/* test_cache_detection.c
 * Comprehensive test to trigger GCC driver's CPU cache detection logic
 * Targets specific Intel cache descriptor bytes in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with target attributes */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    /* Use cache-sized loops for 8KB, 16KB, 32KB L1 cache */
    for (int i = 0; i < size; i += 32) {  /* 32-byte cache line */
        data[i] = i * 2;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    /* Different access pattern for Nehalem */
    for (int i = 0; i < size; i += 64) {  /* 64-byte cache line */
        data[i] = i * 3;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    /* Sandy Bridge specific pattern */
    for (int i = 0; i < size; i += 64) {
        data[i] = i * 4;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    /* Ivy Bridge specific pattern */
    for (int i = 0; i < size; i += 64) {
        data[i] = i * 5;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
typedef void (*compute_func_t)(int*, int);

static compute_func_t resolve_compute() {
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_core2;
    } else {
        return compute_ivybridge;
    }
}

void compute_dynamic(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Target clones for multiple architectures */
__attribute__((target_clones("arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge,default")))
void compute_multiversion(int* data, int size) {
    /* Cache-sensitive computation */
    const int CACHE_LINE = 64;
    for (int i = 0; i < size; i += CACHE_LINE) {
        __builtin_prefetch(&data[i + CACHE_LINE], 0, 3);  /* High temporal locality */
        data[i] = data[i] * 2 + 1;
    }
}

/* Constructor to force early CPU detection */
__attribute__((constructor))
static void init_cpu_features() {
    /* Force CPU detection before main */
    __builtin_cpu_init();
    
    /* Pattern C: Inline assembly to execute CPUID leaf 2 and 4 */
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(0)  /* L1 cache */
    );
    
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(1)  /* L2 cache */
    );
}

/* Cache-sensitive matrix multiplication */
void cache_optimized_matmul(int n, double* A, double* B, double* C) {
    /* Tile size optimized for L1 cache */
    const int BLOCK_SIZE = 32;  /* Adjust based on cache line size */
    
    for (int i = 0; i < n; i += BLOCK_SIZE) {
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            for (int k = 0; k < n; k += BLOCK_SIZE) {
                /* Mini matrix multiplication */
                for (int ii = i; ii < i + BLOCK_SIZE && ii < n; ii++) {
                    for (int kk = k; kk < k + BLOCK_SIZE && kk < n; kk++) {
                        double a = A[ii * n + kk];
                        for (int jj = j; jj < j + BLOCK_SIZE && jj < n; jj++) {
                            C[ii * n + jj] += a * B[kk * n + jj];
                        }
                    }
                }
            }
        }
    }
}

#else
/* Dummy implementations for non-x86 */
void compute_dynamic(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i;
    }
}

void compute_multiversion(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i * 2;
    }
}

void cache_optimized_matmul(int n, double* A, double* B, double* C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
}
#endif

int main() {
    /* Pattern B: Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
    printf("CPU Feature Detection:\n");
    
    /* Check for various Intel CPUs to trigger cache detection */
    if (__builtin_cpu_is("intel")) {
        printf("  Vendor: Intel\n");
        
        /* Check specific microarchitectures */
        const char* archs[] = {
            "core2", "nehalem", "sandybridge", "ivybridge",
            "haswell", "skylake", "k8", "atom"
        };
        
        for (int i = 0; i < sizeof(archs)/sizeof(archs[0]); i++) {
            if (__builtin_cpu_is(archs[i])) {
                printf("  Microarchitecture: %s\n", archs[i]);
            }
        }
    }
    
    /* Check CPU features that influence cache detection */
    const char* features[] = {
        "sse", "sse2", "sse3", "ssse3", "sse4.1", "sse4.2",
        "avx", "avx2", "fma", "aes", "pclmul"
    };
    
    printf("  Features:");
    for (int i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf(" %s", features[i]);
        }
    }
    printf("\n");
    
    /* Get cache information at runtime */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("  L1 Cache Line Size: %ld bytes\n", cache_line);
    
    /* Create arrays sized to specific cache sizes */
    const int SIZE_8KB = 2048;    /* 8KB / sizeof(int) */
    const int SIZE_16KB = 4096;   /* 16KB / sizeof(int) */
    const int SIZE_32KB = 8192;   /* 32KB / sizeof(int) */
    const int SIZE_256KB = 65536; /* 256KB / sizeof(int) */
    
    int* data1 = (int*)malloc(SIZE_8KB * sizeof(int));
    int* data2 = (int*)malloc(SIZE_16KB * sizeof(int));
    int* data3 = (int*)malloc(SIZE_32KB * sizeof(int));
    int* data4 = (int*)malloc(SIZE_256KB * sizeof(int));
    
    if (!data1 || !data2 || !data3 || !data4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE_8KB; i++) data1[i] = i;
    for (int i = 0; i < SIZE_16KB; i++) data2[i] = i;
    for (int i = 0; i < SIZE_32KB; i++) data3[i] = i;
    for (int i = 0; i < SIZE_256KB; i++) data4[i] = i;
    
    /* Pattern D: Use __builtin_prefetch with cache-sized arrays */
    for (int i = 0; i < SIZE_8KB; i += 8) {  /* 8 ints = 32 bytes */
        __builtin_prefetch(&data1[i + 8], 0, 0);  /* Low locality */
    }
    
    for (int i = 0; i < SIZE_16KB; i += 16) { /* 16 ints = 64 bytes */
        __builtin_prefetch(&data2[i + 16], 0, 3); /* High locality */
    }
    
    /* Execute all compute functions to trigger different code paths */
    compute_dynamic(data1, SIZE_8KB);
    compute_multiversion(data2, SIZE_16KB);
    
    /* Call target-specific functions directly */
    compute_core2(data3, SIZE_32KB);
    compute_nehalem(data4, SIZE_256KB);
    
    /* Cache-sensitive matrix multiplication */
    const int MATRIX_SIZE = 128;
    double* A = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double* B = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double* C = (double*)calloc(MATRIX_SIZE * MATRIX_SIZE, sizeof(double));
    
    if (A && B && C) {
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            A[i] = (double)rand() / RAND_MAX;
            B[i] = (double)rand() / RAND_MAX;
        }
        
        cache_optimized_matmul(MATRIX_SIZE, A, B, C);
        
        /* Compute checksum for validation */
        double checksum = 0;
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            checksum += C[i];
        }
        printf("Matrix multiplication checksum: %f\n", checksum);
        
        free(A);
        free(B);
        free(C);
    }
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE_8KB; i++) checksum += data1[i];
    for (int i = 0; i < SIZE_16KB; i++) checksum += data2[i];
    for (int i = 0; i < SIZE_32KB; i++) checksum += data3[i];
    for (int i = 0; i < SIZE_256KB; i++) checksum += data4[i];
    
    printf("Final checksum: %llu\n", checksum);
    
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    
    return 0;
}
