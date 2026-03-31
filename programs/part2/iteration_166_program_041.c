/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPUID cache detection
 * Specifically targets the switch-case block for Intel cache descriptor bytes
 * Compile with: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 * For 32-bit: gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__)

/* ========== PATTERN A: Multi-Architecture Function Multiversioning ========== */

/* Function 1: Core2 target - may trigger cache descriptors 0x0a, 0x0c, 0x0d, etc. */
__attribute__((target("arch=core2")))
int compute_core2(int *data, int size) {
    int sum = 0;
    /* Loop with stride matching potential cache line sizes */
    for (int i = 0; i < size; i += 8) {  /* 8*4=32 bytes - matches 32-byte cache line */
        sum += data[i];
    }
    return sum;
}

/* Function 2: Nehalem target - may trigger different cache descriptors */
__attribute__((target("arch=nehalem")))
int compute_nehalem(int *data, int size) {
    int sum = 0;
    /* 64-byte cache line stride */
    for (int i = 0; i < size; i += 16) {  /* 16*4=64 bytes */
        sum += data[i];
    }
    return sum;
}

/* Function 3: Sandy Bridge target */
__attribute__((target("arch=sandybridge")))
int compute_sandybridge(int *data, int size) {
    int sum = 0;
    /* Mix of strides to potentially trigger various cache models */
    for (int i = 0; i < size; i += 32) {  /* 32*4=128 bytes */
        sum += data[i];
    }
    return sum;
}

/* Function 4: Ivy Bridge target */
__attribute__((target("arch=ivybridge")))
int compute_ivybridge(int *data, int size) {
    int sum = 0;
    /* Another stride pattern */
    for (int i = 0; i < size; i += 64) {  /* 64*4=256 bytes */
        sum += data[i];
    }
    return sum;
}

/* Default implementation */
int compute_default(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

/* Multi-versioned function using target_clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
int compute_multiversion(int *data, int size) {
    return compute_default(data, size);
}

/* ========== PATTERN B: ifunc for Runtime Dispatch ========== */

/* Resolver function - forces CPU detection */
static void *resolver(void) {
    /* These builtins cause GCC to initialize CPU cache data structures */
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

/* ifunc function - resolved at load time */
int compute_ifunc(int *data, int size) 
    __attribute__((ifunc("resolver")));

/* ========== PATTERN C: Inline Assembly with CPUID ========== */

static void execute_cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, 
                          uint32_t *ecx, uint32_t *edx) {
    asm volatile (
        "cpuid\n\t"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

/* Force CPUID leaf 2 (cache descriptors) and leaf 4 (deterministic cache parameters) */
static void probe_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (may trigger the switch-case) */
    execute_cpuid(2, &eax, &ebx, &ecx, &edx);
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 10; i++) {  /* Check multiple cache levels */
        execute_cpuid(4, &eax, &ebx, &ecx, &edx);
        if ((eax & 0x1F) == 0)  /* No more caches */
            break;
    }
}

/* ========== PATTERN D: Compiler Optimization Hints ========== */

/* Array sizes matching specific cache sizes from the switch-case */
#define ARRAY_8KB   2048   /* 2048 * 4 bytes = 8192 bytes = 8KB */
#define ARRAY_16KB  4096   /* 4096 * 4 bytes = 16384 bytes = 16KB */
#define ARRAY_32KB  8192   /* 8192 * 4 bytes = 32768 bytes = 32KB */
#define ARRAY_256KB 65536  /* 65536 * 4 bytes = 262144 bytes = 256KB */

static void cache_sensitive_computation(void) {
    /* Allocate arrays matching cache sizes from the switch-case */
    int *array_8kb = malloc(ARRAY_8KB * sizeof(int));
    int *array_16kb = malloc(ARRAY_16KB * sizeof(int));
    int *array_32kb = malloc(ARRAY_32KB * sizeof(int));
    int *array_256kb = malloc(ARRAY_256KB * sizeof(int));
    
    if (!array_8kb || !array_16kb || !array_32kb || !array_256kb) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_8KB; i++) array_8kb[i] = i % 256;
    for (int i = 0; i < ARRAY_16KB; i++) array_16kb[i] = i % 256;
    for (int i = 0; i < ARRAY_32KB; i++) array_32kb[i] = i % 256;
    for (int i = 0; i < ARRAY_256KB; i++) array_256kb[i] = i % 256;
    
    /* Use __builtin_prefetch with different locality hints */
    int sum = 0;
    
    /* Process 8KB array with prefetch for L1 cache */
    for (int i = 0; i < ARRAY_8KB; i += 8) {  /* 32-byte stride */
        __builtin_prefetch(&array_8kb[i + 32], 0, 0);  /* Prefetch for read, low locality */
        sum += array_8kb[i];
    }
    
    /* Process 16KB array with prefetch for L1/L2 */
    for (int i = 0; i < ARRAY_16KB; i += 16) {  /* 64-byte stride */
        __builtin_prefetch(&array_16kb[i + 64], 0, 1);  /* Medium locality */
        sum += array_16kb[i];
    }
    
    /* Process 32KB array with prefetch for L2 */
    for (int i = 0; i < ARRAY_32KB; i += 32) {  /* 128-byte stride */
        __builtin_prefetch(&array_32kb[i + 128], 0, 2);  /* High locality */
        sum += array_32kb[i];
    }
    
    /* Process 256KB array - likely L2/L3 cache sized */
    for (int i = 0; i < ARRAY_256KB; i += 64) {  /* 256-byte stride */
        __builtin_prefetch(&array_256kb[i + 256], 0, 3);  /* Highest locality */
        sum += array_256kb[i];
    }
    
    /* Use the sum to prevent optimization */
    asm volatile ("" : : "r"(sum) : "memory");
    
    free(array_8kb);
    free(array_16kb);
    free(array_32kb);
    free(array_256kb);
}

/* ========== Validation and Debugging Aids ========== */

/* Constructor runs before main - forces early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    printf("Constructor: Initializing CPU detection...\n");
    
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Execute CPUID to probe cache */
    probe_cache_descriptors();
    
    /* Check CPU features at compile time */
    #ifdef __SSE2__
    printf("Constructor: SSE2 is available at compile time\n");
    #endif
    
    /* Runtime CPU checks */
    if (__builtin_cpu_supports("sse2")) {
        printf("Constructor: SSE2 is supported at runtime\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("Constructor: AVX is supported at runtime\n");
    }
    
    /* Check specific Intel CPUs */
    if (__builtin_cpu_is("core2")) {
        printf("Constructor: CPU identified as Core2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("Constructor: CPU identified as Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("Constructor: CPU identified as Sandy Bridge\n");
    }
}

/* Runtime cache information */
static void print_cache_info(void) {
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("Cache line size: %ld bytes\n", cache_line);
    
    #ifdef _SC_LEVEL1_DCACHE_SIZE
    long l1_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    printf("L1 cache size: %ld bytes\n", l1_size);
    #endif
    
    #ifdef _SC_LEVEL2_CACHE_SIZE
    long l2_size = sysconf(_SC_LEVEL2_CACHE_SIZE);
    printf("L2 cache size: %ld bytes\n", l2_size);
    #endif
}

#else
/* Non-x86 fallback implementations */
int compute_default(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int compute_multiversion(int *data, int size) {
    return compute_default(data, size);
}

int compute_ifunc(int *data, int size) {
    return compute_default(data, size);
}

static void probe_cache_descriptors(void) {
    printf("CPU detection not available on non-x86\n");
}

static void cache_sensitive_computation(void) {
    printf("Cache-sensitive computation not available on non-x86\n");
}

static void print_cache_info(void) {
    printf("Cache info not available on non-x86\n");
}
#endif

/* ========== Main Program ========== */

int main(void) {
    printf("=== Starting CPU Cache Detection Test ===\n");
    
    /* Force CPU initialization again in main */
    #if defined(__i386__) || defined(__x86_64__)
    __builtin_cpu_init();
    #endif
    
    /* Print cache information */
    print_cache_info();
    
    /* Execute CPUID probes */
    probe_cache_descriptors();
    
    /* Perform cache-sensitive computations */
    cache_sensitive_computation();
    
    /* Test array for function calls */
    #define TEST_SIZE 10000
    int *test_data = malloc(TEST_SIZE * sizeof(int));
    if (!test_data) {
        fprintf(stderr, "Failed to allocate test data\n");
        return 1;
    }
    
    /* Initialize test data */
    for (int i = 0; i < TEST_SIZE; i++) {
        test_data[i] = (i * 3) % 100;
    }
    
    /* Call all versions of compute functions */
    int result = 0;
    
    result += compute_default(test_data, TEST_SIZE);
    printf("Default compute result: %d\n", result);
    
    result += compute_multiversion(test_data, TEST_SIZE);
    printf("Multi-version compute result: %d\n", result);
    
    result += compute_ifunc(test_data, TEST_SIZE);
    printf("ifunc compute result: %d\n", result);
    
    #if defined(__i386__) || defined(__x86_64__)
    /* Call architecture-specific functions */
    result += compute_core2(test_data, TEST_SIZE);
    result += compute_nehalem(test_data, TEST_SIZE);
    result += compute_sandybridge(test_data, TEST_SIZE);
    result += compute_ivybridge(test_data, TEST_SIZE);
    #endif
    
    /* Use result to prevent optimization */
    printf("Final checksum: %d\n", result);
    
    free(test_data);
    
    printf("=== Test Complete ===\n");
    return 0;
}
