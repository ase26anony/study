/* test_cache_detection.c
 * Comprehensive test to trigger GCC driver's CPU cache detection logic
 * Targets specific Intel cache descriptor bytes in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target architectures */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
static void cache_sensitive_computation(int* data, int size) {
    /* Simple computation that benefits from cache awareness */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * (i % 256);
    }
    data[0] = sum;
}

/* Pattern B: Functions with specific target attributes */
__attribute__((target("arch=core2")))
static void core2_optimized(int* data, int size) {
    /* 8KB L1 cache (0x0a), 256KB L2 (0x21) or other core2 cache configs */
    const int l1_size = 8192 / sizeof(int);  /* ~8KB in integers */
    for (int i = 0; i < size; i += l1_size) {
        int block_end = (i + l1_size < size) ? i + l1_size : size;
        int block_sum = 0;
        for (int j = i; j < block_end; j++) {
            block_sum += data[j];
        }
        data[i] = block_sum;
    }
}

__attribute__((target("arch=nehalem")))
static void nehalem_optimized(int* data, int size) {
    /* 32KB L1 (0x2c), 256KB L2 (0x21) */
    const int l1_size = 32768 / sizeof(int);  /* 32KB */
    for (int i = 0; i < size; i += l1_size) {
        int block_end = (i + l1_size < size) ? i + l1_size : size;
        for (int j = i; j < block_end; j++) {
            data[j] = data[j] * 3 + 1;
        }
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_optimized(int* data, int size) {
    /* 32KB L1, 256KB L2 */
    const int cache_line = 64;  /* bytes */
    const int ints_per_line = cache_line / sizeof(int);
    
    for (int i = 0; i < size; i += ints_per_line) {
        /* Prefetch next cache line */
        if (i + ints_per_line < size) {
            __builtin_prefetch(&data[i + ints_per_line], 0, 3);
        }
        /* Process current cache line */
        int line_sum = 0;
        for (int j = 0; j < ints_per_line && (i + j) < size; j++) {
            line_sum += data[i + j];
        }
        data[i] = line_sum;
    }
}

/* Pattern C: ifunc resolver for runtime dispatch */
typedef void (*compute_func_t)(int*, int);

static void generic_compute(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] * 1103515245 + 12345) & 0x7fffffff;
    }
}

static compute_func_t resolve_compute() {
    /* Force CPU detection for ifunc resolution */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized;
    } else if (__builtin_cpu_supports("sse3")) {
        return core2_optimized;
    }
    return generic_compute;
}

void compute_with_cache(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern D: Inline assembly to directly query CPUID */
static void query_cpuid_cache_info() {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB Descriptors */
    asm volatile (
        "cpuid\n\t"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_detection() {
    query_cpuid_cache_info();
    __builtin_cpu_init();
}

/* Runtime validation of cache parameters */
static void validate_cache_sizes() {
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("Detected cache line size: %ld bytes\n", cache_line);
    
    /* Compile-time assertion for x86 features */
#if defined(__i386__) || defined(__x86_64__)
    if (!__builtin_cpu_supports("sse2")) {
        fprintf(stderr, "Warning: SSE2 not supported\n");
    }
#endif
}

#else
/* Non-x86 fallback implementations */
static void cache_sensitive_computation(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

static void compute_with_cache(int* data, int size) {
    cache_sensitive_computation(data, size);
}

static void validate_cache_sizes() {
    printf("Non-x86 architecture - using generic cache settings\n");
}
#endif

/* Pattern E: Cache-size-specific computations */
static void test_various_cache_sizes() {
    /* Test arrays sized to trigger different cache descriptor cases */
    const int sizes_kb[] = {8, 16, 24, 32, 128, 256, 512, 1024, 2048, 4096};
    
    for (unsigned int i = 0; i < sizeof(sizes_kb)/sizeof(sizes_kb[0]); i++) {
        int size_in_ints = (sizes_kb[i] * 1024) / sizeof(int);
        int* data = (int*)malloc(size_in_ints * sizeof(int));
        
        if (!data) continue;
        
        /* Initialize with pattern */
        for (int j = 0; j < size_in_ints; j++) {
            data[j] = j % 100;
        }
        
        /* Perform cache-sensitive computation */
        compute_with_cache(data, size_in_ints);
        cache_sensitive_computation(data, size_in_ints);
        
        /* Verify computation */
        int checksum = 0;
        for (int j = 0; j < size_in_ints; j++) {
            checksum = (checksum + data[j]) & 0xFFFF;
        }
        printf("Size %dKB: checksum = 0x%04x\n", sizes_kb[i], checksum);
        
        free(data);
    }
}

/* Main function with extensive CPU feature queries */
int main() {
#if defined(__i386__) || defined(__x86_64__)
    /* Force CPU detection via builtins */
    __builtin_cpu_init();
    
    /* Check for various Intel CPUs to trigger different cache paths */
    if (__builtin_cpu_is("core2")) {
        printf("CPU: Core 2 - expecting 0x0a, 0x0c, 0x0d, 0x21 descriptors\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU: Nehalem - expecting 0x2c, 0x21 descriptors\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU: Sandy Bridge - expecting various L2 descriptors\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU: Ivy Bridge - expecting 0x3a, 0x3b, 0x3c descriptors\n");
    }
    
    /* Check specific features that correlate with cache architectures */
    const char* features[] = {"sse2", "sse3", "ssse3", "sse4.1", "sse4.2", "avx", "avx2"};
    printf("CPU features: ");
    for (unsigned int i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf("%s ", features[i]);
        }
    }
    printf("\n");
#endif
    
    validate_cache_sizes();
    test_various_cache_sizes();
    
    /* Final computation with mixed strategies */
    const int final_size = 1024 * 1024;  /* 1MB */
    int* final_data = (int*)malloc(final_size * sizeof(int));
    
    if (final_data) {
        for (int i = 0; i < final_size; i++) {
            final_data[i] = i & 0xFF;
        }
        
        /* Use all optimization strategies */
        cache_sensitive_computation(final_data, final_size);
        
#if defined(__i386__) || defined(__x86_64__)
        core2_optimized(final_data, final_size);
        nehalem_optimized(final_data, final_size);
        sandybridge_optimized(final_data, final_size);
#endif
        
        compute_with_cache(final_data, final_size);
        
        /* Compute final result */
        unsigned long long total = 0;
        for (int i = 0; i < final_size; i++) {
            total += final_data[i];
        }
        printf("Final result: %llu\n", total);
        
        free(final_data);
    }
    
    return 0;
}
