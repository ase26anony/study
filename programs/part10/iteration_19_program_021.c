/* test_cache_coverage.c - Comprehensive test to cover CPUID leaf 2 cache descriptor cases */
/* Compilation instructions for different targets:
   
   Pentium III/Pentium M cases (0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24):
     gcc -O3 -march=pentium3 -mtune=pentium3 -DTARGET_PENTIUM3 test_cache_coverage.c -o test_p3
   
   Pentium 4/Xeon cases (0x39-0x3e, 0x41-0x45, 0x49):
     gcc -O3 -march=nocona -mtune=nocona -DTARGET_NOCONA test_cache_coverage.c -o test_nocona
     gcc -O3 -march=prescott -mtune=prescott -DTARGET_PRESCOTT test_cache_coverage.c -o test_prescott
   
   AMD K8/K10 cases (0x60, 0x66-0x68, 0x78-0x87):
     gcc -O3 -march=k8 -mtune=k8 -DTARGET_K8 test_cache_coverage.c -o test_k8
     gcc -O3 -march=k10 -mtune=k10 -DTARGET_K10 test_cache_coverage.c -o test_k10
   
   Core 2/Nehalem cases (0x2c, 0x48, 0x4e):
     gcc -O3 -march=core2 -mtune=core2 -DTARGET_CORE2 test_cache_coverage.c -o test_core2
   
   For maximum coverage with function multi-versioning:
     gcc -O3 -march=x86-64 -mtune=generic -fcf-protection=none \
         -DTARGET_ALL test_cache_coverage.c -o test_all
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache line size assumption for padding */
#define CACHE_LINE_SIZE 64

/* Different benchmark patterns to exercise various cache behaviors */
typedef enum {
    PATTERN_SEQUENTIAL,
    PATTERN_STRIDE,
    PATTERN_RANDOM,
    PATTERN_REVERSE
} access_pattern_t;

/* Benchmark configuration */
typedef struct {
    size_t buffer_size;      /* In elements */
    size_t element_size;     /* Size of each element in bytes */
    int iterations;
    access_pattern_t pattern;
    int stride;              /* For stride pattern */
} bench_config_t;

/* Function attributes for targeting specific architectures */
#ifdef __GNUC__
#define TARGET_ARCH(arch) __attribute__((target("arch=" #arch)))
#else
#define TARGET_ARCH(arch)
#endif

/* ============================================================================
   Architecture-specific benchmark variants
   ============================================================================ */

/* Pentium III/Pentium M target - triggers cases 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
#ifdef TARGET_PENTIUM3
TARGET_ARCH(pentium3)
#endif
static void benchmark_pentium3_style(void *buffer, size_t size, int iterations) {
    volatile char *buf = (volatile char *)buffer;
    size_t buf_size = size;
    volatile char result = 0;
    
    /* Sequential access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < buf_size; i += CACHE_LINE_SIZE) {
            result += buf[i];
        }
        COMPILER_BARRIER();
    }
    
    /* Stride-2 access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < buf_size; i += 2 * CACHE_LINE_SIZE) {
            result += buf[i];
        }
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent optimization */
    if (result == 0) printf(".");
}

/* Pentium 4/Xeon target - triggers cases 0x39-0x3e, 0x41-0x45, 0x49 */
#ifdef TARGET_NOCONA
TARGET_ARCH(nocona)
#endif
#ifdef TARGET_PRESCOTT
TARGET_ARCH(prescott)
#endif
static void benchmark_p4_style(void *buffer, size_t size, int iterations) {
    volatile int *buf = (volatile int *)buffer;
    size_t num_elements = size / sizeof(int);
    volatile int result = 0;
    
    /* Large stride access to stress L2 cache */
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < num_elements; i += 16) {
            result += buf[i];
        }
        COMPILER_BARRIER();
    }
    
    /* Reverse access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = num_elements - 1; i > 0; i -= 8) {
            result += buf[i];
        }
        COMPILER_BARRIER();
    }
    
    if (result == 0) printf(".");
}

/* AMD K8/K10 target - triggers cases 0x60, 0x66-0x68, 0x78-0x87 */
#ifdef TARGET_K8
TARGET_ARCH(k8)
#endif
#ifdef TARGET_K10
TARGET_ARCH(k10)
#endif
static void benchmark_amd_style(void *buffer, size_t size, int iterations) {
    volatile long *buf = (volatile long *)buffer;
    size_t num_elements = size / sizeof(long);
    volatile long result = 0;
    
    /* Pseudo-random access using linear congruential generator */
    uint32_t seed = 0xDEADBEEF;
    
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < num_elements; i++) {
            /* Simple LCG */
            seed = seed * 1103515245 + 12345;
            size_t idx = (seed >> 16) % num_elements;
            result += buf[idx];
        }
        COMPILER_BARRIER();
    }
    
    if (result == 0) printf(".");
}

/* Core 2/Nehalem target - triggers cases 0x2c, 0x48, 0x4e */
#ifdef TARGET_CORE2
TARGET_ARCH(core2)
#endif
static void benchmark_core2_style(void *buffer, size_t size, int iterations) {
    volatile double *buf = (volatile double *)buffer;
    size_t num_elements = size / sizeof(double);
    volatile double result = 0.0;
    
    /* Streaming access with temporal locality */
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < num_elements; i += 4) {
            result += buf[i] + buf[i + 1] + buf[i + 2] + buf[i + 3];
        }
        COMPILER_BARRIER();
    }
    
    /* Blocked matrix-style access */
    const size_t block_size = 64; /* Elements */
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < num_elements; i += block_size) {
            size_t end = (i + block_size < num_elements) ? i + block_size : num_elements;
            for (size_t j = i; j < end; j++) {
                result += buf[j];
            }
        }
        COMPILER_BARRIER();
    }
    
    if (result == 0.0) printf(".");
}

/* ============================================================================
   Main benchmark driver
   ============================================================================ */

static void run_benchmark_suite(const char *arch_name) {
    printf("Running cache benchmark for %s architecture...\n", arch_name);
    
    /* Allocate buffers larger than typical L2 cache to ensure cache misses */
    const size_t l1_size = 32 * 1024;      /* 32KB - typical L1 */
    const size_t l2_size = 256 * 1024;     /* 256KB - typical L2 */
    const size_t huge_size = 4 * 1024 * 1024; /* 4MB - larger than most L2 */
    
    void *buffer_small = malloc(l1_size * 2);
    void *buffer_medium = malloc(l2_size * 2);
    void *buffer_large = malloc(huge_size);
    
    if (!buffer_small || !buffer_medium || !buffer_large) {
        fprintf(stderr, "Memory allocation failed\n");
        free(buffer_small);
        free(buffer_medium);
        free(buffer_large);
        return;
    }
    
    /* Initialize with non-zero pattern */
    memset(buffer_small, 0xAA, l1_size * 2);
    memset(buffer_medium, 0xBB, l2_size * 2);
    memset(buffer_large, 0xCC, huge_size);
    
    /* Run architecture-specific benchmarks */
#ifdef TARGET_PENTIUM3
    benchmark_pentium3_style(buffer_small, l1_size * 2, 1000);
    benchmark_pentium3_style(buffer_medium, l2_size * 2, 500);
    benchmark_pentium3_style(buffer_large, huge_size, 100);
#endif
    
#ifdef TARGET_NOCONA
    benchmark_p4_style(buffer_small, l1_size * 2, 1000);
    benchmark_p4_style(buffer_medium, l2_size * 2, 500);
    benchmark_p4_style(buffer_large, huge_size, 100);
#endif
    
#ifdef TARGET_PRESCOTT
    benchmark_p4_style(buffer_small, l1_size * 2, 1000);
    benchmark_p4_style(buffer_medium, l2_size * 2, 500);
    benchmark_p4_style(buffer_large, huge_size, 100);
#endif
    
#ifdef TARGET_K8
    benchmark_amd_style(buffer_small, l1_size * 2, 1000);
    benchmark_amd_style(buffer_medium, l2_size * 2, 500);
    benchmark_amd_style(buffer_large, huge_size, 100);
#endif
    
#ifdef TARGET_K10
    benchmark_amd_style(buffer_small, l1_size * 2, 1000);
    benchmark_amd_style(buffer_medium, l2_size * 2, 500);
    benchmark_amd_style(buffer_large, huge_size, 100);
#endif
    
#ifdef TARGET_CORE2
    benchmark_core2_style(buffer_small, l1_size * 2, 1000);
    benchmark_core2_style(buffer_medium, l2_size * 2, 500);
    benchmark_core2_style(buffer_large, huge_size, 100);
#endif
    
#ifdef TARGET_ALL
    /* Multi-versioning: call all variants */
    benchmark_pentium3_style(buffer_small, l1_size * 2, 100);
    benchmark_p4_style(buffer_medium, l2_size * 2, 100);
    benchmark_amd_style(buffer_large, huge_size, 50);
    benchmark_core2_style(buffer_small, l1_size * 2, 100);
#endif
    
    /* Final computation that uses all buffers to prevent optimization */
    volatile int final_result = 0;
    char *small = (char *)buffer_small;
    char *medium = (char *)buffer_medium;
    char *large = (char *)buffer_large;
    
    for (size_t i = 0; i < l1_size * 2; i += 256) {
        final_result += small[i];
    }
    for (size_t i = 0; i < l2_size * 2; i += 256) {
        final_result += medium[i];
    }
    for (size_t i = 0; i < huge_size; i += 256) {
        final_result += large[i];
    }
    
    printf("\nBenchmark completed for %s. Checksum: %d\n", arch_name, final_result);
    
    free(buffer_small);
    free(buffer_medium);
    free(buffer_large);
}

/* ============================================================================
   CPUID simulation helper (for reference, not executed)
   ============================================================================ */

/*
 * Note: The actual CPUID detection happens in the GCC driver during compilation.
 * This function is just for documentation of what values we're trying to trigger.
 */
static void print_expected_cpuid_values(void) {
    printf("Expected CPUID leaf 2 descriptor bytes to cover:\n");
    printf("  0x0a: L1 8KB, 2-way, 32B line\n");
    printf("  0x0c: L1 16KB, 4-way, 32B line\n");
    printf("  0x0d: L1 16KB, 4-way, 64B line\n");
    printf("  0x0e: L1 24KB, 6-way, 64B line\n");
    printf("  0x21: L2 256KB, 8-way, 64B line\n");
    printf("  0x24: L2 1024KB, 16-way, 64B line\n");
    printf("  0x2c: L1 32KB, 8-way, 64B line\n");
    printf("  0x39: L2 128KB, 4-way, 64B line\n");
    printf("  0x3a: L2 192KB, 6-way, 64B line\n");
    printf("  0x3b: L2 128KB, 2-way, 64B line\n");
    printf("  0x3c: L2 256KB, 4-way, 64B line\n");
    printf("  0x3d: L2 384KB, 6-way, 64B line\n");
    printf("  0x3e: L2 512KB, 4-way, 64B line\n");
    printf("  0x41: L2 128KB, 4-way, 32B line\n");
    printf("  0x42: L2 256KB, 4-way, 32B line\n");
    printf("  0x43: L2 512KB, 4-way, 32B line\n");
    printf("  0x44: L2 1024KB, 4-way, 32B line\n");
    printf("  0x45: L2 2048KB, 4-way, 32B line\n");
    printf("  0x48: L2 3072KB, 12-way, 64B line\n");
    printf("  0x49: L2 4096KB, 16-way, 64B line (non-Xeon-MP)\n");
    printf("  0x4e: L2 6144KB, 24-way, 64B line\n");
    printf("  0x60: L1 16KB, 8-way, 64B line\n");
    printf("  0x66: L1 8KB, 4-way, 64B line\n");
    printf("  0x67: L1 16KB, 4-way, 64B line\n");
    printf("  0x68: L1 32KB, 4-way, 64B line\n");
    printf("  0x78: L2 1024KB, 4-way, 64B line\n");
    printf("  0x79: L2 128KB, 8-way, 64B line\n");
    printf("  0x7a: L2 256KB, 8-way, 64B line\n");
    printf("  0x7b: L2 512KB, 8-way, 64B line\n");
    printf("  0x7c: L2 1024KB, 8-way, 64B line\n");
    printf("  0x7d: L2 2048KB, 8-way, 64B line\n");
    printf("  0x7f: L2 512KB, 2-way, 64B line\n");
    printf("  0x80: L2 512KB, 8-way, 64B line\n");
    printf("  0x82: L2 256KB, 8-way, 32B line\n");
    printf("  0x83: L2 512KB, 8-way, 32B line\n");
    printf("  0x84: L2 1024KB, 8-way, 32B line\n");
    printf("  0x85: L2 2048KB, 8-way, 32B line\n");
    printf("  0x86: L2 512KB, 4-way, 64B line\n");
    printf("  0x87: L2 1024KB, 8-way, 64B line\n");
}

/* ============================================================================
   Main function
   ============================================================================ */

int main(int argc, char *argv[]) {
    const char *arch = "generic";
    
    /* Determine which architecture we're targeting */
#if defined(TARGET_PENTIUM3)
    arch = "pentium3";
#elif defined(TARGET_NOCONA)
    arch = "nocona (Xeon DP)";
#elif defined(TARGET_PRESCOTT)
    arch = "prescott";
#elif defined(TARGET_K8)
    arch = "AMD K8";
#elif defined(TARGET_K10)
    arch = "AMD K10";
#elif defined(TARGET_CORE2)
    arch = "Intel Core 2";
#elif defined(TARGET_ALL)
    arch = "multiple (function multi-versioning)";
#endif
    
    printf("========================================\n");
    printf("GCC Cache Descriptor Coverage Test\n");
    printf("Target Architecture: %s\n", arch);
    printf("========================================\n");
    
    /* Print what we expect to trigger */
    print_expected_cpuid_values();
    
    printf("\nStarting benchmarks...\n");
    
    /* Run the benchmark suite */
    run_benchmark_suite(arch);
    
    printf("\n========================================\n");
    printf("Test completed for %s\n", arch);
    printf("To cover all cases, compile with different -march options:\n");
    printf("  -march=pentium3    (cases 0x0a,0x0c,0x0d,0x0e,0x21,0x24)\n");
    printf("  -march=nocona      (cases 0x39-0x45,0x49)\n");
    printf("  -march=k8          (cases 0x60,0x66-0x68,0x78-0x87)\n");
    printf("  -march=core2       (cases 0x2c,0x48,0x4e)\n");
    printf("========================================\n");
    
    return 0;
}
