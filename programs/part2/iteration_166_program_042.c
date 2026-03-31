/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPUID cache detection
 * Specifically targets the large switch-case block in driver-i386.cc (lines 127-244)
 * 
 * Compilation recommendations:
 * 1. For standard x86: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 * 2. For 32-bit path: gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 * 3. For aggressive: gcc -O3 -march=native -mtune=native -fprofile-arcs -ftest-coverage -fno-inline -funroll-loops test_target.c -o test_target_aggressive
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)
#include <cpuid.h>
#include <x86intrin.h>
#endif

/* ============================================================================
 * PATTERN 1: Multiple target attributes for different Intel architectures
 * ============================================================================ */

/* Function with target attribute for Core 2 (may trigger descriptors: 0x66, 0x78, etc.) */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Function with target attribute for Nehalem (may trigger descriptors: 0x0a, 0x0c, 0x0d, etc.) */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int *data, int size) {
    for (int i = 0; i < size; i += 8) {
        /* Process 8 elements with potential SIMD optimization */
        int sum = 0;
        for (int j = 0; j < 8 && (i + j) < size; j++) {
            sum += data[i + j];
        }
        if (i < size) data[i] = sum;
    }
}

/* Function with target attribute for Sandy Bridge (may trigger descriptors: 0x2c, 0x3a, 0x3b, etc.) */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int *data, int size) {
    /* Use prefetching hints that depend on cache line size */
    for (int i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 32], 0, 3); /* High temporal locality */
        data[i] = data[i] * 2 - data[i + 1];
    }
}

/* Function with target attribute for Ivy Bridge (may trigger descriptors: 0x3e, 0x41, 0x42, etc.) */
__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_compute(int *data, int size) {
    /* Array sized to match specific cache sizes mentioned in switch cases */
    int temp[256]; /* ~1KB for potential L1 cache testing */
    for (int i = 0; i < size && i < 256; i++) {
        temp[i] = data[i] ^ 0x55;
    }
    for (int i = 0; i < size && i < 256; i++) {
        data[i] = temp[i];
    }
}

/* ============================================================================
 * PATTERN 2: ifunc resolver for runtime dispatch
 * ============================================================================ */

/* Default implementation */
static void default_compute(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] << 1) | (data[i] >> 31);
    }
}

/* Resolver function - forces CPU detection during dynamic linking */
static void (*resolve_compute(void))(int*, int) {
    /* These builtins cause GCC driver to initialize CPU cache structures */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return ivybridge_optimized_compute;
    } else if (__builtin_cpu_supports("avx")) {
        return sandybridge_optimized_compute;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_compute;
    } else if (__builtin_cpu_supports("sse3")) {
        return core2_optimized_compute;
    }
    
    return default_compute;
}

/* ifunc function that will trigger resolver at load time */
void ifunc_compute(int *data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* ============================================================================
 * PATTERN 3: Function with target_clones for multi-architecture compilation
 * ============================================================================ */

__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multiversion_compute(int *data, int size) {
    /* Loop with stride matching potential cache line sizes (32, 64 bytes) */
    for (int i = 0; i < size; i += 8) { /* 8 ints = 32 bytes */
        data[i] = data[i] * 3 + 1;
    }
    for (int i = 1; i < size; i += 16) { /* 16 ints = 64 bytes */
        data[i] = data[i] * 2 - 1;
    }
}

/* ============================================================================
 * PATTERN 4: Direct CPUID queries via inline assembly
 * ============================================================================ */

#if defined(__i386__) || defined(__x86_64__)
static void query_cpuid_cache_descriptors(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (triggers the switch-case directly) */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (2), "c" (0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 32; i++) { /* Query up to 32 cache levels */
        asm volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (4), "c" (i));
        
        /* Break when EAX[4:0] = 0 (no more caches) */
        if ((eax & 0x1F) == 0) break;
    }
    
    /* Also query leaf 1 for family/model/stepping which influences cache detection */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (1));
}
#endif

/* ============================================================================
 * PATTERN 5: Cache-sensitive computation with various array sizes
 * ============================================================================ */

/* Array sizes matching specific cache sizes from the switch cases */
#define SIZE_8KB    2048    /* 8KB / sizeof(int) for 0x0a */
#define SIZE_16KB   4096    /* 16KB for 0x0c, 0x0d */
#define SIZE_32KB   8192    /* 32KB for 0x2c */
#define SIZE_128KB  32768   /* 128KB for 0x39, 0x3b, 0x41 */
#define SIZE_256KB  65536   /* 256KB for 0x21, 0x3c, 0x42 */
#define SIZE_512KB  131072  /* 512KB for 0x3e, 0x43, 0x7f, 0x80, 0x83, 0x86 */
#define SIZE_1MB    262144  /* 1MB for 0x24, 0x44, 0x78, 0x7c, 0x84 */
#define SIZE_2MB    524288  /* 2MB for 0x45, 0x7d, 0x85 */

static void cache_sensitive_matrix_multiply(void) {
    /* Use multiple array sizes to potentially trigger different cache parameter initializations */
    int small[256];      /* 1KB - fits in L1 */
    int medium[8192];    /* 32KB - L1/L2 boundary */
    int large[65536];    /* 256KB - L2/L3 boundary */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) small[i] = i & 0xFF;
    for (int i = 0; i < 8192; i++) medium[i] = i & 0x7F;
    for (int i = 0; i < 65536; i++) large[i] = i & 0x3F;
    
    /* Perform computations that might benefit from cache-aware optimizations */
    for (int iter = 0; iter < 10; iter++) {
        for (int i = 0; i < 256; i++) {
            small[i] = small[i] * 3 + medium[i % 8192];
        }
        
        for (int i = 0; i < 8192; i += 8) { /* 32-byte stride */
            medium[i] = medium[i] + large[i % 65536];
        }
        
        for (int i = 0; i < 65536; i += 16) { /* 64-byte stride */
            large[i] = large[i] ^ small[i % 256];
        }
    }
}

/* ============================================================================
 * Constructor function - runs before main()
 * ============================================================================ */

__attribute__((constructor))
static void init_cpu_detection(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Force early CPU detection */
    __builtin_cpu_init();
    
    /* Query CPUID directly - may cause driver to sync cache model */
    query_cpuid_cache_descriptors();
    
    /* Check for specific Intel CPUs - causes driver to initialize cache structures */
    if (__builtin_cpu_is("intel")) {
        /* Check various Intel microarchitectures */
        int is_core2 = __builtin_cpu_is("core2");
        int is_nehalem = __builtin_cpu_is("nehalem");
        int is_sandybridge = __builtin_cpu_is("sandybridge");
        int is_ivybridge = __builtin_cpu_is("ivybridge");
        
        /* Use the results to prevent optimization away */
        volatile int dummy = is_core2 + is_nehalem + is_sandybridge + is_ivybridge;
        (void)dummy;
    }
#endif
}

/* ============================================================================
 * Main function
 * ============================================================================ */

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Compile-time assertion for x86 features */
    _Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
    
    /* Runtime CPU feature checks - trigger driver cache detection */
    __builtin_cpu_init();
    
    /* Extensive use of __builtin_cpu_is for various Intel CPUs */
    const char* cpu_types[] = {
        "core2", "nehalem", "sandybridge", "ivybridge",
        "haswell", "skylake", "k8", "atom"
    };
    
    int cpu_matches = 0;
    for (int i = 0; i < 8; i++) {
        if (__builtin_cpu_is(cpu_types[i])) {
            cpu_matches++;
        }
    }
    
    printf("CPU matches: %d\n", cpu_matches);
    
    /* Print cache line size via sysconf for verification */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
    
    /* Allocate and initialize test data */
    int data_size = 100000;
    int *data = (int*)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < data_size; i++) {
        data[i] = i & 0xFFFF;
    }
    
    /* Execute all variants to trigger different code paths in driver */
    core2_optimized_compute(data, 1000);
    nehalem_optimized_compute(data + 1000, 1000);
    sandybridge_optimized_compute(data + 2000, 1000);
    ivybridge_optimized_compute(data + 3000, 1000);
    
    /* Use ifunc function - triggers resolver */
    ifunc_compute(data + 4000, 1000);
    
    /* Use multiversion function */
    multiversion_compute(data + 5000, 1000);
    
    /* Perform cache-sensitive computation */
    cache_sensitive_matrix_multiply();
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum += data[i];
    }
    
    printf("Data checksum: %llu\n", checksum);
    
    free(data);
    
    /* Return success */
    return 0;
#else
    /* Non-x86 fallback */
    printf("Non-x86 architecture - skipping CPU-specific tests\n");
    return 0;
#endif
}
