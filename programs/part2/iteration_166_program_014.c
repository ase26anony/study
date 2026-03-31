/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPUID cache detection
 * Compile with: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 * For 32-bit: gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#if defined(__i386__) || defined(__x86_64__)

/* ============================================
   PATTERN A: Multi-architecture compilation
   ============================================ */

/* Function with target attribute for specific Intel microarchitectures */
__attribute__((target("arch=core2")))
void core2_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Matrix multiplication tuned for Core2 cache */
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double aik = A[i*n + k];
            for (int j = 0; j < n; j++) {
                C[i*n + j] += aik * B[k*n + j];
            }
        }
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Different loop order for Nehalem cache */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i*n + k] * B[k*n + j];
            }
            C[i*n + j] = sum;
        }
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Sandy Bridge optimized version */
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
            double bkj = B[k*n + j];
            for (int i = 0; i < n; i++) {
                C[i*n + j] += A[i*n + k] * bkj;
            }
        }
    }
}

__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Ivy Bridge version with prefetching hints */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            __builtin_prefetch(&C[i*n + j + 32], 1, 3); /* Prepare for write */
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                __builtin_prefetch(&A[i*n + k + 16], 0, 3); /* High locality read */
                __builtin_prefetch(&B[k*n + j + 16], 0, 3);
                sum += A[i*n + k] * B[k*n + j];
            }
            C[i*n + j] = sum;
        }
    }
}

/* Multi-versioned function using target_clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multiversion_matrix_multiply(int n, double *A, double *B, double *C) {
    /* Default implementation */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < j; j++) {
            C[i*n + j] = 0.0;
            for (int k = 0; k < n; k++) {
                C[i*n + j] += A[i*n + k] * B[k*n + j];
            }
        }
    }
}

/* ============================================
   PATTERN B: ifunc resolver for runtime dispatch
   ============================================ */

typedef void (*matrix_multiply_func_t)(int, double*, double*, double*);

static matrix_multiply_func_t resolve_matrix_multiply() {
    /* This resolver forces CPU detection */
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("avx")) {
        return sandybridge_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return nehalem_optimized_matrix_multiply;
    } else if (__builtin_cpu_supports("ssse3")) {
        return core2_optimized_matrix_multiply;
    } else {
        return core2_optimized_matrix_multiply;
    }
}

void ifunc_matrix_multiply(int n, double *A, double *B, double *C)
    __attribute__((ifunc("resolve_matrix_multiply")));

/* ============================================
   PATTERN C: Direct CPUID queries
   ============================================ */

static void cpuid_leaf_2_query() {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Force the values to be used */
    volatile unsigned int use_them = eax + ebx + ecx + edx;
    (void)use_them;
}

static void cpuid_leaf_4_query(int cache_level) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(cache_level)
    );
    
    /* Extract cache information */
    unsigned int cache_type = eax & 0x1F;
    unsigned int cache_level_num = (eax >> 5) & 0x7;
    unsigned int is_self_initializing = (eax >> 8) & 0x1;
    unsigned int is_fully_associative = (eax >> 9) & 0x1;
    
    volatile unsigned int use_info = cache_type + cache_level_num + 
                                     is_self_initializing + is_fully_associative;
    (void)use_info;
}

/* ============================================
   PATTERN D: Cache-sensitive computations
   ============================================ */

/* Arrays sized to match specific cache line sizes */
#define CACHE_LINE_32_SIZE 32
#define CACHE_LINE_64_SIZE 64

/* 8KB array - matches L1 cache size cases 0x0a, 0x66 */
static char array_8kb[8 * 1024];

/* 16KB array - matches L1 cache size cases 0x0c, 0x0d, 0x67 */
static char array_16kb[16 * 1024];

/* 32KB array - matches L1 cache size cases 0x2c, 0x68 */
static char array_32kb[32 * 1024];

/* 256KB array - matches L2 cache size cases 0x21, 0x3c, 0x7a, 0x82 */
static char array_256kb[256 * 1024];

/* 1MB array - matches L2 cache size cases 0x24, 0x78, 0x7c, 0x84 */
static char array_1mb[1024 * 1024];

static void cache_sensitive_access() {
    /* Access arrays with different strides to exercise cache detection */
    
    /* Access with 32-byte stride (32-byte cache line) */
    for (size_t i = 0; i < sizeof(array_8kb); i += CACHE_LINE_32_SIZE) {
        array_8kb[i] = (char)(i & 0xFF);
        __builtin_prefetch(&array_8kb[i + CACHE_LINE_32_SIZE], 0, 3);
    }
    
    /* Access with 64-byte stride (64-byte cache line) */
    for (size_t i = 0; i < sizeof(array_16kb); i += CACHE_LINE_64_SIZE) {
        array_16kb[i] = (char)(i & 0xFF);
        __builtin_prefetch(&array_16kb[i + CACHE_LINE_64_SIZE], 0, 3);
    }
    
    /* Random access pattern to defeat prefetching */
    for (int i = 0; i < 1000; i++) {
        size_t idx = (i * 97) % sizeof(array_256kb);  /* Prime stride */
        array_256kb[idx] = (char)(array_256kb[idx] + 1);
    }
}

/* ============================================
   Constructor to run CPUID queries early
   ============================================ */

__attribute__((constructor))
static void early_cpu_detection() {
    /* Initialize CPU detection before main() */
    __builtin_cpu_init();
    
    /* Query CPUID leaves 2 and 4 */
    cpuid_leaf_2_query();
    for (int i = 0; i < 4; i++) {
        cpuid_leaf_4_query(i);
    }
    
    /* Check for specific Intel CPUs */
    if (__builtin_cpu_is("core2")) {
        /* Will trigger cache descriptor parsing for Core2 */
        volatile int is_core2 = 1;
        (void)is_core2;
    }
    if (__builtin_cpu_is("nehalem")) {
        volatile int is_nehalem = 1;
        (void)is_nehalem;
    }
    if (__builtin_cpu_is("sandybridge")) {
        volatile int is_sandybridge = 1;
        (void)is_sandybridge;
    }
    if (__builtin_cpu_is("ivybridge")) {
        volatile int is_ivybridge = 1;
        (void)is_ivybridge;
    }
}

/* ============================================
   Main test program
   ============================================ */

int main() {
    printf("Starting CPU cache detection test...\n");
    
    /* Force CPU feature checks */
    assert(__builtin_cpu_supports("sse2") && "SSE2 required for x86_64");
    
    /* Check cache line size at runtime */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("Detected L1 cache line size: %ld bytes\n", cache_line);
    
    /* Perform cache-sensitive computations */
    cache_sensitive_access();
    
    /* Create test matrices */
    const int MATRIX_SIZE = 64;  /* Small enough to fit in L1/L2 cache */
    double *A = (double*)aligned_alloc(64, MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *B = (double*)aligned_alloc(64, MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *C = (double*)aligned_alloc(64, MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    
    if (!A || !B || !C) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        A[i] = (double)(i % 100);
        B[i] = (double)((i + 1) % 100);
        C[i] = 0.0;
    }
    
    /* Call all matrix multiply variants to trigger different target attributes */
    core2_optimized_matrix_multiply(MATRIX_SIZE, A, B, C);
    
    /* Reset C */
    memset(C, 0, MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    nehalem_optimized_matrix_multiply(MATRIX_SIZE, A, B, C);
    
    /* Reset C */
    memset(C, 0, MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    sandybridge_optimized_matrix_multiply(MATRIX_SIZE, A, B, C);
    
    /* Reset C */
    memset(C, 0, MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    ivybridge_optimized_matrix_multiply(MATRIX_SIZE, A, B, C);
    
    /* Reset C */
    memset(C, 0, MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    multiversion_matrix_multiply(MATRIX_SIZE, A, B, C);
    
    /* Reset C */
    memset(C, 0, MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    ifunc_matrix_multiply(MATRIX_SIZE, A, B, C);
    
    /* Compute checksum for validation */
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        checksum += C[i];
    }
    printf("Matrix multiplication checksum: %f\n", checksum);
    
    /* Clean up */
    free(A);
    free(B);
    free(C);
    
    /* Additional CPUID queries */
    printf("Performing additional CPUID queries...\n");
    cpuid_leaf_2_query();
    for (int i = 0; i < 4; i++) {
        cpuid_leaf_4_query(i);
    }
    
    printf("Test completed successfully.\n");
    return 0;
}

#else
/* Non-x86 fallback */
int main() {
    printf("This test is for x86 systems only.\n");
    return 0;
}
#endif /* __i386__ || __x86_64__ */
