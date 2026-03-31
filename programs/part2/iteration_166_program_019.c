/* test_cache_detection.c - Comprehensive test for Intel CPU cache descriptor detection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target architectures */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void cache_sensitive_computation(int* data, int size) {
    /* Simple computation that benefits from cache awareness */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * (i % 256);
    }
    data[0] = sum;
}

/* Pattern B: Individual functions with specific target attributes */
__attribute__((target("arch=core2")))
void core2_optimized(int* data, int size) {
    /* Use prefetch hints sized for Core2 cache lines (64 bytes) */
    for (int i = 0; i < size; i += 16) {  /* 16 * 4 bytes = 64 bytes */
        __builtin_prefetch(&data[i + 16], 0, 3);
        data[i] *= 2;
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized(int* data, int size) {
    /* Different access pattern for Nehalem */
    for (int i = 0; i < size; i += 8) {  /* 8 * 4 bytes = 32 bytes */
        __builtin_prefetch(&data[i + 32], 1, 3);
        data[i] += i;
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized(int* data, int size) {
    /* Sandy Bridge optimized pattern */
    for (int i = 0; i < size; i += 32) {  /* 32 * 4 bytes = 128 bytes */
        __builtin_prefetch(&data[i + 64], 0, 2);
        data[i] -= i;
    }
}

/* Pattern C: ifunc resolver for runtime dispatch */
static void (*resolved_computation)(int*, int);

static void (*resolve_computation(void))(int*, int) {
    /* Force CPU detection by checking various features */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized;
    } else if (__builtin_cpu_supports("avx")) {
        return nehalem_optimized;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return core2_optimized;
    }
    return cache_sensitive_computation;
}

void dynamic_computation(int* data, int size) 
    __attribute__((ifunc("resolve_computation")));

/* Pattern D: Inline assembly to directly query CPUID */
static void query_cpuid_cache_info(void) {
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
static void init_cpu_cache_info(void) {
    query_cpuid_cache_info();
    
    /* Force driver to detect cache parameters by using builtins */
    __builtin_cpu_init();
    
    /* Check various CPU types to trigger cache detection */
    if (__builtin_cpu_is("intel")) {
        /* Check specific Intel microarchitectures */
        const char* intel_archs[] = {
            "core2", "nehalem", "sandybridge", "ivybridge",
            "haswell", "broadwell", "skylake", "kabylake"
        };
        
        for (int i = 0; i < 8; i++) {
            if (__builtin_cpu_is(intel_archs[i])) {
                break;
            }
        }
    }
}

/* Array sizes designed to match specific cache sizes from uncovered lines */
#define SIZE_8KB    2048    /* 8KB / 4 bytes per int */
#define SIZE_16KB   4096
#define SIZE_32KB   8192
#define SIZE_64KB   16384
#define SIZE_128KB  32768
#define SIZE_256KB  65536
#define SIZE_512KB  131072
#define SIZE_1MB    262144
#define SIZE_2MB    524288
#define SIZE_4MB    1048576
#define SIZE_6MB    1572864

#else
/* Non-x86 fallback implementations */
void cache_sensitive_computation(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

void dynamic_computation(int* data, int size) {
    cache_sensitive_computation(data, size);
}

static void init_cpu_cache_info(void) {}
#endif

/* Compile-time assertion for x86 features */
#ifdef __x86_64__
_Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86_64");
#endif

/* Main test function */
int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Create test arrays sized to match various cache configurations */
    int* array_8kb = malloc(SIZE_8KB * sizeof(int));
    int* array_16kb = malloc(SIZE_16KB * sizeof(int));
    int* array_32kb = malloc(SIZE_32KB * sizeof(int));
    int* array_64kb = malloc(SIZE_64KB * sizeof(int));
    
    if (!array_8kb || !array_16kb || !array_32kb || !array_64kb) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE_64KB; i++) {
        if (i < SIZE_8KB) array_8kb[i] = i;
        if (i < SIZE_16KB) array_16kb[i] = i * 2;
        if (i < SIZE_32KB) array_32kb[i] = i * 3;
        array_64kb[i] = i * 4;
    }
    
    /* Execute cache-sensitive computations with different array sizes */
    cache_sensitive_computation(array_8kb, SIZE_8KB);
    cache_sensitive_computation(array_16kb, SIZE_16KB);
    cache_sensitive_computation(array_32kb, SIZE_32KB);
    cache_sensitive_computation(array_64kb, SIZE_64KB);
    
    /* Use target-specific functions */
    core2_optimized(array_8kb, SIZE_8KB);
    nehalem_optimized(array_16kb, SIZE_16KB);
    sandybridge_optimized(array_32kb, SIZE_32KB);
    
    /* Use ifunc-resolved function */
    dynamic_computation(array_64kb, SIZE_64KB);
    
    /* Compute checksum for validation */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE_64KB; i++) {
        if (i < SIZE_8KB) checksum += array_8kb[i];
        if (i < SIZE_16KB) checksum += array_16kb[i];
        if (i < SIZE_32KB) checksum += array_32kb[i];
        checksum += array_64kb[i];
    }
    
    /* Print cache information if available */
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    printf("L1 Cache line size: %ld bytes\n", 
           sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
#endif
    
    printf("Computation checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(array_8kb);
    free(array_16kb);
    free(array_32kb);
    free(array_64kb);
    
    return 0;
#else
    printf("Non-x86 architecture detected, using fallback implementation\n");
    
    int* test_array = malloc(1024 * sizeof(int));
    for (int i = 0; i < 1024; i++) {
        test_array[i] = i;
    }
    
    cache_sensitive_computation(test_array, 1024);
    
    int sum = 0;
    for (int i = 0; i < 1024; i++) {
        sum += test_array[i];
    }
    
    printf("Fallback checksum: %d\n", sum);
    free(test_array);
    
    return 0;
#endif
}
