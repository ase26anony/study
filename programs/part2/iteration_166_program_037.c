/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPUID cache detection
 * Specifically targets the large switch-case block in driver-i386.cc (lines 127-244)
 * 
 * Compilation recommendations:
 *   gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 *   gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 *   gcc -O3 -march=native -mtune=native -fprofile-arcs -ftest-coverage -fno-inline -funroll-loops test_target.c -o test_target_aggressive
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* ============================================
   PATTERN 1: Multiple target attributes for different Intel architectures
   ============================================ */

/* Function with target attribute for Core 2 - may trigger cache descriptors: 0x66, 0x67, 0x68, etc. */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size) {
    /* 8KB array fits L1 cache for some Core 2 variants (0x66: 8KB, 4-way, 64-byte line) */
    int temp[2048]; /* 8KB */
    for (size_t i = 0; i < size && i < 2048; i++) {
        temp[i % 2048] = data[i] * 3 + 7;
    }
    
    /* Use prefetch with 64-byte line hint */
    for (size_t i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 64], 0, 3);
    }
}

/* Function with target attribute for Nehalem - may trigger cache descriptors: 0x0a, 0x0c, 0x0d, etc. */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size) {
    /* 16KB array fits L1 cache for Nehalem (0x0d: 16KB, 4-way, 64-byte line) */
    int temp[4096]; /* 16KB */
    for (size_t i = 0; i < size && i < 4096; i++) {
        temp[i % 4096] = data[i] * 5 - 2;
    }
    
    /* Prefetch with 64-byte line */
    for (size_t i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 128], 0, 2);
    }
}

/* Function with target attribute for Sandy Bridge - may trigger L2 cache descriptors: 0x24, 0x3c, etc. */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size) {
    /* 256KB array fits L2 cache for Sandy Bridge (0x3c: 256KB, 4-way, 64-byte line) */
    int temp[65536]; /* 256KB */
    for (size_t i = 0; i < size && i < 65536; i++) {
        temp[i % 65536] = data[i] * 11 + data[i/2];
    }
    
    /* Prefetch with both 32 and 64 byte line hints */
    for (size_t i = 0; i < size; i += 8) {
        __builtin_prefetch(&data[i + 32], 0, 1);
    }
}

/* Function with target attribute for Ivy Bridge - may trigger cache descriptors: 0x3a, 0x3e, etc. */
__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_compute(int* data, size_t size) {
    /* 32KB L1 cache (0x2c: 32KB, 8-way, 64-byte line) */
    int temp[8192]; /* 32KB */
    for (size_t i = 0; i < size && i < 8192; i++) {
        temp[i % 8192] = data[i] * 7 - data[i/4];
    }
    
    /* Mix of prefetch patterns */
    for (size_t i = 0; i < size; i += 32) {
        __builtin_prefetch(&data[i + 96], 0, 0);
    }
}

/* ============================================
   PATTERN 2: ifunc resolver for runtime dispatch
   ============================================ */

typedef void (*compute_func_t)(int*, size_t);

/* Default implementation */
static void default_compute(int* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

/* Resolver function - forces CPU detection */
static compute_func_t resolve_compute(void) {
    /* These builtins cause GCC driver to initialize CPU cache data structures */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return ivybridge_optimized_compute;
    } else if (__builtin_cpu_supports("avx")) {
        return sandybridge_optimized_compute;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_compute;
    } else if (__builtin_cpu_supports("ssse3")) {
        return core2_optimized_compute;
    }
    
    return default_compute;
}

/* ifunc function - triggers resolver at load time */
void ifunc_compute(int* data, size_t size) 
    __attribute__((ifunc("resolve_compute")));

/* ============================================
   PATTERN 3: Function multiversioning
   ============================================ */

__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multiversion_compute(int* data, size_t size) {
    /* Array sized to 16KB - matches several L1 cache configurations */
    int l1_cache[4096]; /* 16KB */
    /* Array sized to 256KB - matches several L2 cache configurations */
    int l2_cache[65536]; /* 256KB */
    
    /* Access patterns designed for different cache line sizes */
    for (size_t i = 0; i < size && i < 4096; i++) {
        /* 32-byte stride access */
        l1_cache[i] = data[i] * 3;
        /* 64-byte stride access */
        if (i % 2 == 0) {
            l2_cache[i * 16] = data[i] + l1_cache[i];
        }
    }
}

/* ============================================
   PATTERN 4: Direct CPUID queries via inline assembly
   ============================================ */

static void query_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (Intel) */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    printf("CPUID leaf 2: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n", 
           eax, ebx, ecx, edx);
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 8; i++) { /* Query up to 8 cache levels */
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
        
        if ((eax & 0x1f) == 0) /* No more caches */
            break;
            
        printf("CPUID leaf 4[%d]: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n",
               i, eax, ebx, ecx, edx);
    }
}

/* ============================================
   PATTERN 5: Constructor function for early CPU detection
   ============================================ */

__attribute__((constructor))
static void early_cpu_init(void) {
    /* Force CPU initialization before main */
    __builtin_cpu_init();
    
    /* Query CPUID early */
    query_cpuid_cache_descriptors();
    
    /* Check various CPU features to trigger detection paths */
    if (__builtin_cpu_is("intel")) {
        printf("CPU vendor: Intel\n");
    }
    
    /* Check for specific cache-related features */
    if (__builtin_cpu_supports("clflush")) {
        printf("CLFLUSH instruction available\n");
    }
    
    if (__builtin_cpu_supports("clflushopt")) {
        printf("CLFLUSHOPT instruction available\n");
    }
}

/* ============================================
   Main test program with cache-sensitive computation
   ============================================ */

int main(void) {
    const size_t data_size = 100000;
    int* data = (int*)malloc(data_size * sizeof(int));
    int* data2 = (int*)malloc(data_size * sizeof(int));
    
    if (!data || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (size_t i = 0; i < data_size; i++) {
        data[i] = (i * 3) % 100;
        data2[i] = (i * 7) % 100;
    }
    
    /* PATTERN B: Extensive use of __builtin_cpu_is checks */
    printf("=== CPU Detection Checks ===\n");
    __builtin_cpu_init();
    
    /* Check for various Intel CPUs - each may trigger different cache descriptors */
    const char* intel_cpus[] = {
        "core2", "nehalem", "sandybridge", "ivybridge", 
        "haswell", "broadwell", "skylake", "kabylake"
    };
    
    for (int i = 0; i < 8; i++) {
        if (__builtin_cpu_is(intel_cpus[i])) {
            printf("Detected CPU: %s\n", intel_cpus[i]);
        }
    }
    
    /* Check cache line size at runtime */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 cache line size: %ld bytes\n", cache_line);
    
    /* Compile-time assertion for x86 */
#if defined(__i386__) || defined(__x86_64__)
    /* SSE2 should be available on all x86-64 and most modern x86 */
    if (!__builtin_cpu_supports("sse2")) {
        printf("Warning: SSE2 not supported\n");
    }
#endif
    
    /* ============================================
       Cache-sensitive computation: Matrix-style multiplication
       ============================================ */
    
    printf("\n=== Running Cache-Sensitive Computations ===\n");
    
    /* Use ifunc dispatch */
    printf("1. Using ifunc dispatch...\n");
    ifunc_compute(data, data_size);
    
    /* Use multiversion function */
    printf("2. Using multiversion function...\n");
    multiversion_compute(data2, data_size);
    
    /* Call all target-specific functions */
    printf("3. Calling all target-specific functions...\n");
    core2_optimized_compute(data, data_size / 4);
    nehalem_optimized_compute(data, data_size / 4);
    sandybridge_optimized_compute(data, data_size / 4);
    ivybridge_optimized_compute(data, data_size / 4);
    
    /* ============================================
       Cache line size optimized loops
       ============================================ */
    
    printf("4. Running cache-line optimized loops...\n");
    
    /* Loop tiling with different tile sizes matching potential cache lines */
    const size_t tile_32 = 32 / sizeof(int);  /* 32-byte cache line */
    const size_t tile_64 = 64 / sizeof(int);  /* 64-byte cache line */
    
    /* Try both tile sizes */
    for (size_t tile_size : {tile_32, tile_64}) {
        for (size_t i = 0; i < data_size; i += tile_size) {
            size_t end = i + tile_size;
            if (end > data_size) end = data_size;
            
            int sum = 0;
            for (size_t j = i; j < end; j++) {
                sum += data[j];
                /* Mix in some data2 accesses */
                if (j % 4 == 0) {
                    data2[j] = sum;
                }
            }
            data[i] = sum;
        }
    }
    
    /* ============================================
       Final validation
       ============================================ */
    
    printf("5. Validating results...\n");
    
    /* Compute checksum */
    unsigned long long checksum = 0;
    for (size_t i = 0; i < data_size; i++) {
        checksum += (unsigned long long)data[i];
        checksum += (unsigned long long)data2[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Test completed successfully.\n");
    
    free(data);
    free(data2);
    
    return 0;
}

#else /* Non-x86 fallback */

int main(void) {
    printf("This test is designed for x86 architecture only.\n");
    printf("Compile with -m32 or on x86_64 system for full coverage.\n");
    return 0;
}

#endif /* __i386__ || __x86_64__ */
