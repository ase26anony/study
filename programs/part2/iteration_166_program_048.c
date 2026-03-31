/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPUID cache detection
 * Specifically targets the large switch-case block in driver-i386.cc (lines 127-244)
 * Compile with: gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 * For 32-bit: gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__)

/* ============================================
   PATTERN A: Multi-architecture compilation
   ============================================ */

/* Function with target attribute for specific Intel microarchitectures */
__attribute__((target("arch=core2")))
void cache_sensitive_core2(int* data, int size) {
    /* Loop designed to exercise 8KB/16KB L1 cache cases (0x0a, 0x0c, 0x0d, 0x0e) */
    const int block = 32; /* 32 * 4 bytes = 128 bytes, fits cache lines */
    for (int i = 0; i < size; i += block) {
        int limit = (i + block < size) ? i + block : size;
        for (int j = i; j < limit; j++) {
            data[j] = data[j] * 3 + 1;
        }
    }
}

__attribute__((target("arch=nehalem")))
void cache_sensitive_nehalem(int* data, int size) {
    /* Exercise larger L2 cache cases (0x21, 0x24, 0x48, 0x49) */
    const int block = 256; /* 256 * 4 bytes = 1KB */
    for (int i = 0; i < size; i += block) {
        int sum = 0;
        for (int j = i; j < i + block && j < size; j++) {
            sum += data[j];
        }
        if (i < size) data[i] = sum;
    }
}

__attribute__((target("arch=sandybridge")))
void cache_sensitive_sandybridge(int* data, int size) {
    /* Exercise L1 cache cases 0x66, 0x67, 0x68 */
    const int block = 64; /* 64 * 4 = 256 bytes */
    for (int i = 0; i < size; i += block) {
        for (int j = i; j < i + block && j < size; j++) {
            data[j] = (data[j] << 3) | (data[j] >> 29);
        }
    }
}

__attribute__((target("arch=ivybridge")))
void cache_sensitive_ivybridge(int* data, int size) {
    /* Exercise L2 cache cases 0x7a, 0x7b, 0x7c, 0x7d */
    const int block = 512; /* 512 * 4 = 2KB */
    for (int i = 0; i < size; i += block) {
        int max_val = data[i];
        for (int j = i + 1; j < i + block && j < size; j++) {
            if (data[j] > max_val) max_val = data[j];
        }
        if (i < size) data[i] = max_val;
    }
}

/* Default implementation */
__attribute__((target("default")))
void cache_sensitive_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] + i;
    }
}

/* ============================================
   PATTERN B: ifunc resolver for runtime dispatch
   ============================================ */

typedef void (*cache_func_t)(int*, int);

static cache_func_t resolve_cache_func(void) {
    /* Force driver to detect CPU features for ifunc resolution */
    if (__builtin_cpu_supports("avx2")) {
        return cache_sensitive_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return cache_sensitive_nehalem;
    } else if (__builtin_cpu_supports("ssse3")) {
        return cache_sensitive_core2;
    } else {
        return cache_sensitive_default;
    }
}

void cache_sensitive_dynamic(int* data, int size) 
    __attribute__((ifunc("resolve_cache_func")));

/* ============================================
   PATTERN C: Direct CPUID queries
   ============================================ */

static void cpuid_query_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (triggers switch cases) */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 32; i++) {
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
        if ((eax & 0x1f) == 0) break; /* No more caches */
    }
}

/* ============================================
   PATTERN D: Compiler optimization hints
   ============================================ */

/* Array sizes matching specific cache sizes from switch cases */
#define ARRAY_8KB   2048   /* 2048 * 4 bytes = 8KB - case 0x0a */
#define ARRAY_16KB  4096   /* 4096 * 4 = 16KB - case 0x0c, 0x0d */
#define ARRAY_32KB  8192   /* 8192 * 4 = 32KB - case 0x2c */
#define ARRAY_256KB 65536  /* 65536 * 4 = 256KB - case 0x21 */

__attribute__((constructor))
static void init_cache_test(void) {
    /* Force early CPU detection in driver */
    __builtin_cpu_init();
    cpuid_query_cache_info();
}

/* ============================================
   Main test program
   ============================================ */

int main(void) {
    /* PATTERN B: Extensive use of CPU detection builtins */
    printf("CPU Detection Results:\n");
    printf("  is Intel: %d\n", __builtin_cpu_is("intel"));
    printf("  is core2: %d\n", __builtin_cpu_is("core2"));
    printf("  is nehalem: %d\n", __builtin_cpu_is("nehalem"));
    printf("  is sandybridge: %d\n", __builtin_cpu_is("sandybridge"));
    printf("  is ivybridge: %d\n", __builtin_cpu_is("ivybridge"));
    
    printf("CPU Feature Support:\n");
    printf("  SSE2: %d\n", __builtin_cpu_supports("sse2"));
    printf("  SSE3: %d\n", __builtin_cpu_supports("sse3"));
    printf("  SSSE3: %d\n", __builtin_cpu_supports("ssse3"));
    printf("  SSE4.1: %d\n", __builtin_cpu_supports("sse4.1"));
    printf("  SSE4.2: %d\n", __builtin_cpu_supports("sse4.2"));
    printf("  AVX: %d\n", __builtin_cpu_supports("avx"));
    printf("  AVX2: %d\n", __builtin_cpu_supports("avx2"));
    
    /* Runtime cache line size query */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 Cache line size: %ld bytes\n", cache_line);
    
    /* Allocate arrays matching cache sizes from switch cases */
    int* data_8kb = malloc(ARRAY_8KB * sizeof(int));
    int* data_16kb = malloc(ARRAY_16KB * sizeof(int));
    int* data_32kb = malloc(ARRAY_32KB * sizeof(int));
    int* data_256kb = malloc(ARRAY_256KB * sizeof(int));
    
    if (!data_8kb || !data_16kb || !data_32kb || !data_256kb) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_8KB; i++) data_8kb[i] = i & 0xFF;
    for (int i = 0; i < ARRAY_16KB; i++) data_16kb[i] = i & 0xFF;
    for (int i = 0; i < ARRAY_32KB; i++) data_32kb[i] = i & 0xFF;
    for (int i = 0; i < ARRAY_256KB; i++) data_256kb[i] = i & 0xFF;
    
    /* PATTERN D: Use __builtin_prefetch with cache-aware access patterns */
    for (int i = 0; i < ARRAY_256KB; i += cache_line / sizeof(int)) {
        __builtin_prefetch(&data_256kb[i + cache_line / sizeof(int)], 0, 3);
        data_256kb[i] = data_256kb[i] * 2;
    }
    
    /* Execute all target-specific functions to trigger driver detection */
    cache_sensitive_core2(data_8kb, ARRAY_8KB);
    cache_sensitive_nehalem(data_16kb, ARRAY_16KB);
    cache_sensitive_sandybridge(data_32kb, ARRAY_32KB);
    cache_sensitive_ivybridge(data_256kb, ARRAY_256KB);
    cache_sensitive_default(data_8kb, ARRAY_8KB);
    
    /* Use ifunc-resolved function */
    cache_sensitive_dynamic(data_16kb, ARRAY_16KB);
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_8KB; i++) checksum += data_8kb[i];
    for (int i = 0; i < ARRAY_16KB; i++) checksum += data_16kb[i];
    for (int i = 0; i < ARRAY_32KB; i++) checksum += data_32kb[i];
    for (int i = 0; i < ARRAY_256KB; i++) checksum += data_256kb[i];
    
    printf("Final checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(data_8kb);
    free(data_16kb);
    free(data_32kb);
    free(data_256kb);
    
    return 0;
}

#else /* Non-x86 fallback */

int main(void) {
    printf("This test is designed for x86 architecture only.\n");
    return 0;
}

#endif /* __i386__ || __x86_64__ */
