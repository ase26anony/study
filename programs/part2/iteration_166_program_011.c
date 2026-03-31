/* test_cache_detection.c - Comprehensive test for Intel CPU cache descriptor detection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target architectures */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void cache_sensitive_computation(int* data, int size) {
    /* Simple computation that benefits from cache awareness */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * (i % 16);
    }
    data[0] = sum;
}

/* Pattern B: Functions with explicit target attributes */
__attribute__((target("arch=core2")))
void core2_optimized() {
    /* Force driver to detect Core2 cache parameters */
    volatile int arr[2048]; /* 8KB array for L1 cache */
    for (int i = 0; i < 2048; i++) {
        arr[i] = i;
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized() {
    /* Force driver to detect Nehalem cache parameters */
    volatile int arr[4096]; /* 16KB array */
    for (int i = 0; i < 4096; i++) {
        arr[i] = i * 2;
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized() {
    /* Force driver to detect Sandy Bridge cache parameters */
    volatile int arr[8192]; /* 32KB array */
    for (int i = 0; i < 8192; i++) {
        arr[i] = i * 3;
    }
}

/* Pattern C: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

static void compute_avx(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 4 - 2;
    }
}

static void (*resolve_compute(void))(int*, int) {
    /* This resolver forces CPU detection */
    if (__builtin_cpu_supports("avx2")) {
        return compute_avx;
    }
    return compute_default;
}

__attribute__((ifunc("resolve_compute")))
void dynamic_compute(int* data, int size);

/* Pattern D: Cache line sized prefetching */
void cache_line_optimized_loop(int* data, int n) {
    /* Use prefetch hints with different cache line sizes */
    for (int i = 0; i < n; i += 16) { /* 64-byte stride for 64-byte cache lines */
        __builtin_prefetch(&data[i + 32], 0, 0); /* Prefetch ahead */
        data[i] = data[i] * data[i];
    }
    
    for (int i = 0; i < n; i += 8) { /* 32-byte stride for 32-byte cache lines */
        __builtin_prefetch(&data[i + 16], 0, 1); /* Medium locality */
        data[i] += i;
    }
}

/* Constructor to force early CPU detection */
__attribute__((constructor))
static void init_cpu_detection() {
    /* Force CPU initialization before main */
    __builtin_cpu_init();
    
    /* Execute CPUID leaf 2 directly (cache descriptors) */
    unsigned int eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Execute CPUID leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
    }
}

/* Compile-time assertion for x86 features */
_Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
               "Only 32-bit or 64-bit x86 supported");

#else
/* Dummy implementations for non-x86 targets */
void cache_sensitive_computation(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

void core2_optimized() {}
void nehalem_optimized() {}
void sandybridge_optimized() {}
void dynamic_compute(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3;
    }
}
void cache_line_optimized_loop(int* data, int n) {
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * data[i];
    }
}
#endif

/* Matrix multiplication optimized for cache */
void cache_aware_matrix_multiply(int n, int A[n][n], int B[n][n], int C[n][n]) {
    /* Blocked matrix multiplication for better cache utilization */
    const int BLOCK_SIZE = 16; /* Experiment with different block sizes */
    
    for (int i = 0; i < n; i += BLOCK_SIZE) {
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            for (int k = 0; k < n; k += BLOCK_SIZE) {
                /* Mini matrix multiplication on blocks */
                for (int ii = i; ii < i + BLOCK_SIZE && ii < n; ii++) {
                    for (int jj = j; jj < j + BLOCK_SIZE && jj < n; jj++) {
                        int sum = C[ii][jj];
                        for (int kk = k; kk < k + BLOCK_SIZE && kk < n; kk++) {
                            sum += A[ii][kk] * B[kk][jj];
                        }
                        C[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

int main() {
#if defined(__i386__) || defined(__x86_64__)
    /* Pattern B: Extensive use of CPU detection builtins */
    printf("CPU Detection Results:\n");
    
    __builtin_cpu_init();
    
    /* Check various Intel CPU models to trigger cache detection */
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
    
    /* Get cache line size via sysconf */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("  L1 Cache Line Size: %ld bytes\n", cache_line);
    
    /* Execute all target-specific functions */
    core2_optimized();
    nehalem_optimized();
    sandybridge_optimized();
#endif

    /* Create test data sized to trigger cache awareness */
    const int DATA_SIZE = 32768; /* 128KB for L2 cache testing */
    int* data = (int*)malloc(DATA_SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = i % 256;
    }
    
    /* Execute all cache-sensitive computations */
    cache_sensitive_computation(data, DATA_SIZE);
    
    dynamic_compute(data, DATA_SIZE);
    
    cache_line_optimized_loop(data, DATA_SIZE);
    
    /* Perform matrix multiplication (cache intensive) */
    const int MATRIX_SIZE = 64;
    int (*A)[MATRIX_SIZE] = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int (*B)[MATRIX_SIZE] = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int (*C)[MATRIX_SIZE] = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                A[i][j] = (i + j) % 8;
                B[i][j] = (i - j + MATRIX_SIZE) % 8;
                C[i][j] = 0;
            }
        }
        
        cache_aware_matrix_multiply(MATRIX_SIZE, A, B, C);
        
        /* Compute checksum for validation */
        int checksum = 0;
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                checksum = (checksum + C[i][j]) % 1000000;
            }
        }
        printf("Matrix checksum: %d\n", checksum);
    }
    
    /* Compute final result from data array */
    int result = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        result = (result + data[i]) % 1000000007;
    }
    printf("Final result: %d\n", result);
    
    /* Cleanup */
    free(data);
    if (A) free(A);
    if (B) free(B);
    if (C) free(C);
    
    return 0;
}
