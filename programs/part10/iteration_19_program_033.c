/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPUID leaf 2 cache detection
 * logic for various x86 microarchitectures. It uses conditional compilation
 * and function multi-versioning to target specific CPU models that should
 * trigger the uncovered case statements in driver-i386.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our memory accesses */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Memory clobber to force actual memory operations */
#define MEMORY_CLOBBER() __asm__ __volatile__("" ::: "memory")

/* Different test configurations for various cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
#define TARGET_CPU "pentium3"
#define EXPECTED_CASES {0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24}
#elif defined(TEST_PENTIUM4)
/* Targets cases: 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e */
#define TARGET_CPU "pentium4"
#define EXPECTED_CASES {0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e}
#elif defined(TEST_XEON_DP)
/* Targets case 0x49 (non-Xeon-MP path) */
#define TARGET_CPU "nocona"
#define EXPECTED_CASES {0x49}
#elif defined(TEST_ATHLON64)
/* Targets cases: 0x40 series, 0x78-0x87 */
#define TARGET_CPU "k8"
#define EXPECTED_CASES {0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7f, 0x80}
#elif defined(TEST_CORE2)
/* Targets cases: 0x66, 0x67, 0x68, 0x41-0x45 */
#define TARGET_CPU "core2"
#define EXPECTED_CASES {0x66, 0x67, 0x68, 0x41, 0x42, 0x43, 0x44, 0x45}
#elif defined(TEST_NEHALEM)
/* Targets cases: 0x60, 0x48, 0x4e */
#define TARGET_CPU "nehalem"
#define EXPECTED_CASES {0x60, 0x48, 0x4e}
#else
/* Generic fallback - will use actual CPU detection */
#define TARGET_CPU "native"
#define EXPECTED_CASES {}
#endif

/* Function attributes for targeting specific CPU architectures */
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define TARGET_SPECIFIC __attribute__((target(TARGET_CPU)))
#else
#define TARGET_SPECIFIC
#endif

/* Cache-thrashing benchmark function */
TARGET_SPECIFIC
static void perform_cache_benchmark(int iterations, int buffer_size_kb) {
    /* Allocate buffer larger than expected cache to ensure cache misses */
    size_t buffer_size = (size_t)buffer_size_kb * 1024 / sizeof(int);
    int *buffer = (int*)malloc(buffer_size * sizeof(int));
    volatile int sink = 0;
    
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer of %d KB\n", buffer_size_kb);
        return;
    }
    
    /* Initialize buffer with pseudo-random values */
    unsigned int seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = (int)(seed = seed * 1103515245 + 12345);
    }
    
    /* Perform cache-thrashing operations */
    for (int iter = 0; iter < iterations; iter++) {
        /* Access pattern designed to cause cache conflicts */
        for (size_t i = 0; i < buffer_size; i += 64) {  /* 64-byte stride */
            size_t idx = (i * 17 + iter) % buffer_size;  /* Pseudo-random but deterministic */
            buffer[idx] = buffer[idx] * 3 + 1;
        }
        
        /* Force memory operations */
        MEMORY_CLOBBER();
        
        /* Another pattern with different stride */
        for (size_t i = 0; i < buffer_size; i += 128) {
            size_t idx = (i * 13 + iter * 7) % buffer_size;
            sink += buffer[idx];
        }
        
        COMPILER_BARRIER();
    }
    
    /* Final computation to prevent dead code elimination */
    int sum = 0;
    for (size_t i = 0; i < buffer_size; i += 256) {
        sum ^= buffer[i];
    }
    
    sink = sum;
    
    free(buffer);
}

/* Multi-versioned functions for different CPU targets */
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
/* Pentium III target - should trigger early cache descriptor cases */
__attribute__((target("arch=pentium3")))
static void benchmark_pentium3(void) {
    /* Use buffer sizes that match Pentium III cache characteristics */
    perform_cache_benchmark(100, 512);  /* Larger than L2 */
}

/* Pentium 4 target - different cache hierarchy */
__attribute__((target("arch=pentium4")))
static void benchmark_pentium4(void) {
    perform_cache_benchmark(100, 2048);  /* Larger than typical P4 L2 */
}

/* Xeon DP (Nocona) target - for case 0x49 */
__attribute__((target("arch=nocona")))
static void benchmark_xeon_dp(void) {
    perform_cache_benchmark(100, 4096);  /* Large buffer for Xeon */
}

/* AMD K8 target */
__attribute__((target("arch=k8")))
static void benchmark_athlon64(void) {
    perform_cache_benchmark(100, 1024);
}

/* Core 2 target */
__attribute__((target("arch=core2")))
static void benchmark_core2(void) {
    perform_cache_benchmark(100, 4096);
}

/* Nehalem target */
__attribute__((target("arch=nehalem")))
static void benchmark_nehalem(void) {
    perform_cache_benchmark(100, 8192);
}
#endif

/* Main benchmark driver */
int main(int argc, char **argv) {
    int iterations = 50;
    int buffer_size_kb = 2048;  /* Default 2MB buffer */
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) buffer_size_kb = atoi(argv[2]);
    
    printf("Running cache benchmark targeting: %s\n", TARGET_CPU);
    printf("Iterations: %d, Buffer size: %d KB\n", iterations, buffer_size_kb);
    
    /* Run the main benchmark with target-specific optimizations */
    perform_cache_benchmark(iterations, buffer_size_kb);
    
    /* Also run multi-versioned benchmarks if supported */
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    printf("Running multi-versioned benchmarks...\n");
    
    /* These calls will cause GCC to generate code for each target,
     * potentially triggering cache detection for each architecture */
    benchmark_pentium3();
    benchmark_pentium4();
    benchmark_xeon_dp();
    benchmark_athlon64();
    benchmark_core2();
    benchmark_nehalem();
#endif
    
    /* Additional memory-intensive computation to increase optimization pressure */
    {
        const int N = 1024 * 1024;
        int *data = (int*)malloc(N * sizeof(int));
        volatile int result = 0;
        
        if (data) {
            /* Initialize */
            for (int i = 0; i < N; i++) {
                data[i] = i;
            }
            
            /* Matrix-style access pattern */
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < N - 1; j += 64) {
                    data[j] = data[j + 1] * 3 - data[j];
                }
                COMPILER_BARRIER();
            }
            
            /* Reduce */
            for (int i = 0; i < N; i += 128) {
                result ^= data[i];
            }
            
            free(data);
        }
        
        printf("Benchmark completed. Result: %d\n", result);
    }
    
    return 0;
}
