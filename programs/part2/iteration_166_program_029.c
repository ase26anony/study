/*
 * test_target.c - Test program to trigger GCC driver CPU cache detection
 * Specifically targets uncovered lines 127-244 in driver-i386.cc
 * Compile with: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 * For 32-bit: gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* ============================================
   PATTERN A: Function Multiversioning with Target Attributes
   ============================================ */

/* Function 1: Core2 microarchitecture - may trigger cache descriptors 0x0a, 0x0c, 0x0d, etc. */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Function 2: Nehalem microarchitecture */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 11;
    }
}

/* Function 3: Sandy Bridge microarchitecture */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 13;
    }
}

/* Function 4: Ivy Bridge microarchitecture */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 17;
    }
}

/* ============================================
   PATTERN B: ifunc resolver for runtime dispatch
   ============================================ */

typedef void (*compute_func_t)(int*, int);

/* Default implementation */
void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

/* Resolver function - forces CPU detection */
static compute_func_t resolve_compute(void) {
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return compute_core2;
    }
    
    return compute_default;
}

/* ifunc function - driver must detect CPU features for resolver */
void compute_optimized(int* data, int size)
    __attribute__((ifunc("resolve_compute")));

/* ============================================
   PATTERN C: Direct CPUID queries
   ============================================ */

static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t* eax, uint32_t* ebx,
                  uint32_t* ecx, uint32_t* edx) {
    asm volatile("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "a"(leaf), "c"(subleaf));
}

/* Query CPUID leaf 2 (cache descriptors) */
static void query_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB Descriptor information */
    cpuid(2, 0, &eax, &ebx, &ecx, &edx);
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    for (int i = 0; i < 10; i++) {
        cpuid(4, i, &eax, &ebx, &ecx, &edx);
        if ((eax & 0x1F) == 0) break; /* No more cache levels */
    }
}

/* ============================================
   PATTERN D: Cache-sensitive computations
   ============================================ */

/* Array sizes matching specific cache sizes from uncovered lines */
#define SIZE_8KB    2048    /* 8KB / sizeof(int) */
#define SIZE_16KB   4096    /* 16KB / sizeof(int) */
#define SIZE_32KB   8192    /* 32KB / sizeof(int) */
#define SIZE_64KB   16384   /* 64KB / sizeof(int) */
#define SIZE_128KB  32768   /* 128KB / sizeof(int) */
#define SIZE_256KB  65536   /* 256KB / sizeof(int) */
#define SIZE_512KB  131072  /* 512KB / sizeof(int) */
#define SIZE_1MB    262144  /* 1MB / sizeof(int) */
#define SIZE_2MB    524288  /* 2MB / sizeof(int) */
#define SIZE_4MB    1048576 /* 4MB / sizeof(int) */

/* Matrix multiplication with cache-aware tiling */
static void matrix_multiply_cache_aware(int n, int* A, int* B, int* C) {
    const int BLOCK = 32; /* Try to match cache line size */
    
    for (int i = 0; i < n; i += BLOCK) {
        for (int j = 0; j < n; j += BLOCK) {
            for (int k = 0; k < n; k += BLOCK) {
                for (int ii = i; ii < i + BLOCK && ii < n; ii++) {
                    for (int jj = j; jj < j + BLOCK && jj < n; jj++) {
                        int sum = C[ii * n + jj];
                        for (int kk = k; kk < k + BLOCK && kk < n; kk++) {
                            sum += A[ii * n + kk] * B[kk * n + jj];
                        }
                        C[ii * n + jj] = sum;
                    }
                }
            }
        }
    }
}

/* Prefetch hints with different locality levels */
static void prefetch_experiment(int* data, int size) {
    for (int i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 32], 0, 0); /* L1 prefetch */
        __builtin_prefetch(&data[i + 64], 0, 1); /* L2 prefetch */
        __builtin_prefetch(&data[i + 128], 0, 2); /* L3 prefetch */
        data[i] = data[i] * 3 + data[i + 8];
    }
}

/* ============================================
   Validation and Debugging
   ============================================ */

/* Constructor runs before main - forces early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    printf("Initializing CPU detection...\n");
    __builtin_cpu_init();
    
    /* Force driver to check CPU features */
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("sse4.1")) {
        printf("SSE4.1 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    
    /* Query cache descriptors directly */
    query_cpuid_cache_descriptors();
}

/* Runtime cache information */
static void print_cache_info(void) {
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    long l1_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", l1_line);
#endif
    
#ifdef _SC_LEVEL2_CACHE_SIZE
    long l2_size = sysconf(_SC_LEVEL2_CACHE_SIZE);
    printf("L2 cache size: %ld bytes\n", l2_size);
#endif
}

/* ============================================
   Main test function
   ============================================ */

int main(void) {
    printf("=== GCC Driver Cache Detection Test ===\n");
    
    /* PATTERN B: Use __builtin_cpu_is to force driver detection */
    __builtin_cpu_init();
    
    if (__builtin_cpu_is("intel")) {
        printf("Intel CPU detected\n");
    }
    if (__builtin_cpu_is("core2")) {
        printf("Core2 microarchitecture\n");
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
    
    /* Print cache information */
    print_cache_info();
    
    /* Allocate arrays of various cache-relevant sizes */
    int* data_small = malloc(SIZE_8KB * sizeof(int));
    int* data_medium = malloc(SIZE_64KB * sizeof(int));
    int* data_large = malloc(SIZE_256KB * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < SIZE_8KB; i++) data_small[i] = i % 256;
    for (int i = 0; i < SIZE_64KB; i++) data_medium[i] = i % 512;
    for (int i = 0; i < SIZE_256KB; i++) data_large[i] = i % 1024;
    
    /* Execute all pattern variations */
    
    /* 1. Call ifunc-resolved function */
    printf("\n1. Executing ifunc-resolved computation...\n");
    compute_optimized(data_small, SIZE_8KB);
    
    /* 2. Call target-specific functions directly */
    printf("2. Executing target-specific functions...\n");
    compute_core2(data_medium, SIZE_64KB);
    compute_nehalem(data_medium, SIZE_64KB / 2);
    compute_sandybridge(data_medium, SIZE_64KB / 4);
    compute_ivybridge(data_medium, SIZE_64KB / 8);
    
    /* 3. Cache-sensitive matrix multiplication */
    printf("3. Performing cache-aware matrix multiply...\n");
    const int MATRIX_SIZE = 128;
    int* A = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* B = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* C = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    if (A && B && C) {
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            A[i] = i % 100;
            B[i] = (i + 1) % 100;
            C[i] = 0;
        }
        
        matrix_multiply_cache_aware(MATRIX_SIZE, A, B, C);
        
        /* Compute checksum */
        long long checksum = 0;
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            checksum += C[i];
        }
        printf("   Matrix checksum: %lld\n", checksum);
        
        free(A);
        free(B);
        free(C);
    }
    
    /* 4. Prefetch experiment */
    printf("4. Running prefetch experiment...\n");
    prefetch_experiment(data_large, SIZE_256KB);
    
    /* 5. Compute final checksum for validation */
    printf("5. Computing final checksum...\n");
    long long total_checksum = 0;
    for (int i = 0; i < SIZE_8KB; i++) total_checksum += data_small[i];
    for (int i = 0; i < SIZE_64KB; i++) total_checksum += data_medium[i];
    for (int i = 0; i < SIZE_256KB; i++) total_checksum += data_large[i];
    
    printf("\nTotal checksum: %lld\n", total_checksum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(data_small);
    free(data_medium);
    free(data_large);
    
    return 0;
}

#else /* Non-x86 fallback */

int main(void) {
    printf("This test is for x86 architectures only.\n");
    printf("Compile with: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c\n");
    return 0;
}

#endif /* __i386__ || __x86_64__ */
