/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our memory accesses */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
/* Targets cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68 */
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
/* Targets cases: 0x78-0x87 */
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
/* Targets cases: 0x48, 0x4e */
__attribute__((target("arch=core2")))
#endif
static void cache_thrashing_benchmark(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j;
    
    /* Allocate buffer larger than L2 cache to ensure cache misses */
    int elements = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    unsigned int seed = 42;
    for (i = 0; i < elements; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    /* Cache thrashing pattern - access memory in strides that
       trigger different associativity patterns */
    for (j = 0; j < iterations; j++) {
        /* Different access patterns to exercise various cache configurations */
        
        /* Pattern 1: Sequential access (good for prefetching) */
        for (i = 0; i < elements; i += 64) {
            result += buffer[i];
            MEMORY_BARRIER();
        }
        
        /* Pattern 2: Strided access (tests associativity) */
        for (i = 0; i < elements; i += 128) {
            result ^= buffer[i];
            MEMORY_BARRIER();
        }
        
        /* Pattern 3: Reverse access */
        for (i = elements - 1; i >= 0; i -= 256) {
            result |= buffer[i];
            MEMORY_BARRIER();
        }
        
        /* Pattern 4: Random-like but deterministic pattern */
        unsigned int idx = j;
        for (i = 0; i < 1000; i++) {
            idx = (idx * 1103515245 + 12345) % elements;
            result += buffer[idx];
            MEMORY_BARRIER();
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Benchmark result: %d\n", result);
    
    free(buffer);
}

/* Multi-versioned function using target_clones attribute */
#if defined(USE_MULTI_VERSIONING) && __GNUC__ >= 6
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void multi_version_cache_test(void) {
    /* This function will be compiled for multiple targets */
    cache_thrashing_benchmark(10, 4096); /* 4MB buffer */
}

/* Individual test functions for specific cache descriptor cases */
#ifdef TEST_CASE_0x0A
/* 8KB L1, 2-way, 32B line - Early Pentium III */
__attribute__((target("arch=pentium3")))
static void test_case_0x0a(void) {
    printf("Testing for cache descriptor 0x0a (8KB L1)\n");
    cache_thrashing_benchmark(20, 512); /* Small buffer for L1 testing */
}
#endif

#ifdef TEST_CASE_0x49
/* 4MB L2, 16-way, 64B line - Pentium 4/Xeon (non-MP) */
__attribute__((target("arch=nocona")))
static void test_case_0x49(void) {
    printf("Testing for cache descriptor 0x49 (4MB L2, non-Xeon-MP)\n");
    cache_thrashing_benchmark(15, 8192); /* 8MB buffer to exceed L2 */
}
#endif

#ifdef TEST_CASE_0x78_0x87
/* Various L2 configurations - AMD K8 family */
__attribute__((target("arch=k8")))
static void test_case_0x78_0x87(void) {
    printf("Testing for cache descriptors 0x78-0x87 (AMD K8)\n");
    cache_thrashing_benchmark(12, 4096);
}
#endif

#ifdef TEST_CASE_0x48_0x4E
/* Large L2 caches - Core 2 family */
__attribute__((target("arch=core2")))
static void test_case_0x48_0x4e(void) {
    printf("Testing for cache descriptors 0x48, 0x4e (Core 2)\n");
    cache_thrashing_benchmark(8, 16384); /* 16MB buffer */
}
#endif

int main(int argc, char *argv[]) {
    int test_iterations = 10;
    int buffer_size_kb = 4096; /* 4MB default */
    
    /* Parse command line arguments */
    if (argc > 1) test_iterations = atoi(argv[1]);
    if (argc > 2) buffer_size_kb = atoi(argv[2]);
    
    printf("Cache Detection Coverage Test\n");
    printf("=============================\n");
    
    /* Run architecture-specific tests if compiled with those defines */
    
#ifdef TEST_CASE_0x0A
    test_case_0x0a();
#endif
    
#ifdef TEST_CASE_0x49
    test_case_0x49();
#endif
    
#ifdef TEST_CASE_0x78_0x87
    test_case_0x78_0x87();
#endif
    
#ifdef TEST_CASE_0x48_0x4E
    test_case_0x48_0x4e();
#endif
    
    /* Main benchmark - this will use the target attribute if defined */
    printf("\nRunning main cache thrashing benchmark...\n");
    printf("Iterations: %d, Buffer size: %d KB\n", test_iterations, buffer_size_kb);
    
#if defined(USE_MULTI_VERSIONING) && __GNUC__ >= 6
    printf("Using multi-versioned function...\n");
    multi_version_cache_test();
#else
    /* Default benchmark with current target */
    cache_thrashing_benchmark(test_iterations, buffer_size_kb);
#endif
    
    /* Additional test: Matrix multiplication to exercise cache hierarchy */
    printf("\nRunning matrix multiplication test...\n");
    {
        const int N = 512; /* Size that doesn't fit in L1/L2 */
        int *A = (int*)malloc(N * N * sizeof(int));
        int *B = (int*)malloc(N * N * sizeof(int));
        int *C = (int*)malloc(N * N * sizeof(int));
        
        if (A && B && C) {
            /* Initialize matrices */
            for (int i = 0; i < N * N; i++) {
                A[i] = i % 100;
                B[i] = (i + 1) % 100;
            }
            
            /* Matrix multiplication - exercises cache blocking behavior */
            for (int i = 0; i < N; i++) {
                for (int k = 0; k < N; k++) {
                    for (int j = 0; j < N; j++) {
                        C[i * N + j] += A[i * N + k] * B[k * N + j];
                    }
                }
                MEMORY_BARRIER();
            }
            
            /* Use result */
            volatile int sum = 0;
            for (int i = 0; i < N * N; i += 64) {
                sum += C[i];
            }
            printf("Matrix test sum: %d\n", sum);
        }
        
        free(A); free(B); free(C);
    }
    
    printf("\nTest completed.\n");
    return 0;
}
