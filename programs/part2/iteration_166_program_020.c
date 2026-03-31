/*
 * test_cache_descriptors.c
 * 
 * This program is designed to trigger the GCC driver's CPU cache detection
 * logic, specifically targeting the switch-case block in driver-i386.cc
 * that handles Intel CPUID cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 * 
 * Compilation flags to maximize coverage:
 *   gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage \
 *       -m32 -fno-inline -funroll-loops test_cache_descriptors.c -o test_cache
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

/* ==================== CPUID INLINE ASSEMBLY ==================== */
#if defined(__i386__) || defined(__x86_64__)

/* Execute CPUID leaf 2 (cache descriptors) */
static inline void cpuid_leaf2(uint32_t* eax, uint32_t* ebx, 
                               uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2), "c"(0)
    );
}

/* Execute CPUID leaf 4 (deterministic cache parameters) */
static inline void cpuid_leaf4(uint32_t* eax, uint32_t* ebx,
                               uint32_t* ecx, uint32_t* edx, uint32_t level) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(level)
    );
}

/* Execute CPUID leaf 1 (feature bits) */
static inline void cpuid_leaf1(uint32_t* eax, uint32_t* ebx,
                               uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(1)
    );
}

#else
/* Dummy implementations for non-x86 */
static inline void cpuid_leaf2(uint32_t* eax, uint32_t* ebx,
                               uint32_t* ecx, uint32_t* edx) {
    *eax = *ebx = *ecx = *edx = 0;
}
static inline void cpuid_leaf4(uint32_t* eax, uint32_t* ebx,
                               uint32_t* ecx, uint32_t* edx, uint32_t level) {
    *eax = *ebx = *ecx = *edx = 0;
}
static inline void cpuid_leaf1(uint32_t* eax, uint32_t* ebx,
                               uint32_t* ecx, uint32_t* edx) {
    *eax = *ebx = *ecx = *edx = 0;
}
#endif

/* ==================== TARGET-SPECIFIC FUNCTIONS ==================== */
/* Each function targets a different Intel microarchitecture to trigger
 * different cache descriptor patterns in the GCC driver */

/* Core 2 - known to use various cache descriptors */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Nehalem - different cache hierarchy */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 2;
    }
}

/* Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 1;
    }
}

/* Ivy Bridge */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 3;
    }
}

/* ==================== IFUNC RESOLVER ==================== */
/* This forces CPU detection at runtime */
typedef void (*compute_func_t)(int*, int);

static compute_func_t resolve_compute(void) {
    /* These builtins cause GCC to initialize CPU detection */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;  /* Sandy Bridge or later */
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("ssse3")) {
        return compute_core2;
    } else {
        return compute_core2;
    }
}

/* The ifunc forces runtime CPU detection */
void compute_optimized(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* ==================== MULTIVERSIONED FUNCTION ==================== */
/* GCC will generate multiple versions for different architectures */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
int multiversion_sum(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

/* ==================== CACHE-SENSITIVE COMPUTATIONS ==================== */
/* Arrays sized to match specific cache line sizes and cache capacities */

/* 8KB array - matches L1 cache sizes from descriptors 0x0a, 0x66 */
#define SIZE_8KB (8192 / sizeof(int))
static int array_8kb[SIZE_8KB];

/* 16KB array - matches L1 cache sizes from descriptors 0x0c, 0x0d, 0x67 */
#define SIZE_16KB (16384 / sizeof(int))
static int array_16kb[SIZE_16KB];

/* 32KB array - matches L1 cache sizes from descriptors 0x2c, 0x68 */
#define SIZE_32KB (32768 / sizeof(int))
static int array_32kb[SIZE_32KB];

/* 64KB array - matches various L2 cache sizes */
#define SIZE_64KB (65536 / sizeof(int))
static int array_64kb[SIZE_64KB];

/* 128KB array - matches L2 cache sizes from descriptors 0x39, 0x3b, 0x41, 0x79 */
#define SIZE_128KB (131072 / sizeof(int))
static int array_128kb[SIZE_128KB];

/* ==================== CONSTRUCTOR ==================== */
/* Runs before main, forces early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
#if defined(__i386__) || defined(__x86_64__)
    uint32_t eax, ebx, ecx, edx;
    
    /* Force CPUID leaf 2 query (cache descriptors) */
    cpuid_leaf2(&eax, &ebx, &ecx, &edx);
    
    /* Force CPUID leaf 4 queries for cache hierarchy */
    for (int level = 0; level < 4; level++) {
        cpuid_leaf4(&eax, &ebx, &ecx, &edx, level);
    }
    
    /* Initialize builtin CPU detection */
    __builtin_cpu_init();
#endif
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE_8KB; i++) array_8kb[i] = i & 0xFF;
    for (int i = 0; i < SIZE_16KB; i++) array_16kb[i] = i & 0xFF;
    for (int i = 0; i < SIZE_32KB; i++) array_32kb[i] = i & 0xFF;
    for (int i = 0; i < SIZE_64KB; i++) array_64kb[i] = i & 0xFF;
    for (int i = 0; i < SIZE_128KB; i++) array_128kb[i] = i & 0xFF;
}

/* ==================== CACHE LINE OPTIMIZED LOOPS ==================== */
/* These loops use prefetching and access patterns that depend on cache line size */

static void cache_line_optimized_compute(int* data, int size, int line_size) {
    /* Use prefetch hints - these may cause GCC to query cache parameters */
    for (int i = 0; i < size; i += line_size / sizeof(int)) {
        __builtin_prefetch(&data[i + line_size / sizeof(int)], 0, 3);
        data[i] = data[i] * 2 + 1;
    }
}

/* ==================== MAIN ==================== */
int main(void) {
    int result = 0;
    
#if defined(__i386__) || defined(__x86_64__)
    /* Compile-time assertion for x86 features */
    static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
                  "Expected 32 or 64-bit x86");
    
    /* Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
    /* Check for specific Intel CPUs - forces cache detection */
    if (__builtin_cpu_is("core2")) {
        printf("CPU: Core 2\n");
    } else if (__builtin_cpu_is("nehalem")) {
        printf("CPU: Nehalem\n");
    } else if (__builtin_cpu_is("sandybridge")) {
        printf("CPU: Sandy Bridge\n");
    } else if (__builtin_cpu_is("ivybridge")) {
        printf("CPU: Ivy Bridge\n");
    } else if (__builtin_cpu_is("haswell")) {
        printf("CPU: Haswell\n");
    } else if (__builtin_cpu_is("skylake")) {
        printf("CPU: Skylake\n");
    }
    
    /* Check CPU features - each call may trigger cache detection */
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("sse3")) {
        printf("SSE3 supported\n");
    }
    if (__builtin_cpu_supports("ssse3")) {
        printf("SSSE3 supported\n");
    }
    if (__builtin_cpu_supports("sse4.1")) {
        printf("SSE4.1 supported\n");
    }
    if (__builtin_cpu_supports("sse4.2")) {
        printf("SSE4.2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
    }
    
    /* Get cache line size at runtime */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
    
    /* Execute CPUID directly */
    uint32_t eax, ebx, ecx, edx;
    cpuid_leaf1(&eax, &ebx, &ecx, &edx);
    printf("CPUID Leaf 1: eax=0x%08x\n", eax);
    
    /* Execute CPUID leaf 2 (cache descriptors) */
    cpuid_leaf2(&eax, &ebx, &ecx, &edx);
    printf("CPUID Leaf 2: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n",
           eax, ebx, ecx, edx);
#endif
    
    /* Call all target-specific functions */
    compute_core2(array_8kb, SIZE_8KB);
    compute_nehalem(array_16kb, SIZE_16KB);
    compute_sandybridge(array_32kb, SIZE_32KB);
    compute_ivybridge(array_64kb, SIZE_64KB);
    
    /* Call ifunc-resolved function */
    compute_optimized(array_128kb, SIZE_128KB);
    
    /* Call multiversioned function */
    result += multiversion_sum(array_8kb, SIZE_8KB);
    result += multiversion_sum(array_16kb, SIZE_16KB);
    result += multiversion_sum(array_32kb, SIZE_32KB);
    
    /* Perform cache-line optimized computations */
    cache_line_optimized_compute(array_8kb, SIZE_8KB, 32);   /* 32-byte lines */
    cache_line_optimized_compute(array_16kb, SIZE_16KB, 64); /* 64-byte lines */
    
    /* Matrix-style computation that benefits from cache optimization */
    for (int i = 0; i < SIZE_8KB; i++) {
        for (int j = 0; j < 16; j++) {
            array_8kb[i] = array_8kb[i] * 3 + array_8kb[(i + j) % SIZE_8KB];
        }
    }
    
    /* Final checksum */
    for (int i = 0; i < SIZE_8KB; i++) {
        result += array_8kb[i];
    }
    for (int i = 0; i < SIZE_16KB; i++) {
        result += array_16kb[i];
    }
    for (int i = 0; i < SIZE_32KB; i++) {
        result += array_32kb[i];
    }
    
    printf("Result checksum: %d\n", result & 0xFFFF);
    return (result & 0xFFFF) == 0 ? 0 : 1;
}
