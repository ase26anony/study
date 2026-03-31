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
    /* 8KB L1 cache optimized loop */
    const int block_size = 8192 / sizeof(int);
    for (int i = 0; i < size; i += block_size) {
        int limit = (i + block_size < size) ? i + block_size : size;
        for (int j = i; j < limit; j++) {
            data[j] = data[j] * 2 + 1;
        }
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized(int* data, int size) {
    /* 32KB L1 cache optimized loop */
    const int block_size = 32768 / sizeof(int);
    for (int i = 0; i < size; i += block_size) {
        int limit = (i + block_size < size) ? i + block_size : size;
        for (int j = i; j < limit; j++) {
            data[j] = data[j] * 3 - 2;
        }
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized(int* data, int size) {
    /* 256KB L2 cache optimized loop */
    const int block_size = 262144 / sizeof(int);
    for (int i = 0; i < size; i += block_size) {
        int limit = (i + block_size < size) ? i + block_size : size;
        for (int j = i; j < limit; j++) {
            data[j] = (data[j] << 1) | (data[j] >> 31);
        }
    }
}

/* Pattern C: ifunc resolver for runtime dispatch */
static void (*resolved_function)(int*, int);

static void (*resolve_cache_function(void))(int*, int) {
    /* Force CPU detection through builtins */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized;
    } else if (__builtin_cpu_supports("avx")) {
        return nehalem_optimized;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return core2_optimized;
    } else {
        return cache_sensitive_computation;
    }
}

__attribute__((ifunc("resolve_cache_function")))
void dynamic_cache_function(int* data, int size);

/* Pattern D: Inline assembly CPUID calls */
void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    printf("CPUID Leaf 2: eax=%08x ebx=%08x ecx=%08x edx=%08x\n", 
           eax, ebx, ecx, edx);
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 10; i++) {
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
        
        if ((eax & 0x1f) == 0) break; /* No more caches */
        
        printf("Cache %d: type=%u level=%u ways=%u sets=%u line=%u partitions=%u\n",
               i,
               eax & 0x1f,
               (eax >> 5) & 0x7,
               ((ebx >> 22) & 0x3ff) + 1,
               (ebx & 0x7ff) + 1,
               ((ebx >> 12) & 0x3ff) + 1,
               ((ebx >> 22) & 0x3ff) + 1);
    }
}

/* Constructor to run early CPU detection */
__attribute__((constructor))
static void early_cpu_init(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    /* Query cache information early */
    query_cpuid_cache_info();
}

/* Pattern E: Cache line sized prefetching */
void cache_line_optimized_access(int* data, int size) {
    /* Use different cache line sizes to trigger different cases */
    const int line32 = 32 / sizeof(int);
    const int line64 = 64 / sizeof(int);
    
    /* Mix of prefetch hints */
    for (int i = 0; i < size; i += line32) {
        __builtin_prefetch(&data[i + line32], 0, 3); /* High temporal locality */
        data[i] = data[i] * 2;
    }
    
    for (int i = 0; i < size; i += line64) {
        __builtin_prefetch(&data[i + line64], 1, 1); /* Low temporal locality */
        data[i] = data[i] + data[i + 1];
    }
}

#else
/* Non-x86 fallback implementations */
void cache_sensitive_computation(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

void dynamic_cache_function(int* data, int size) {
    cache_sensitive_computation(data, size);
}

void query_cpuid_cache_info(void) {
    printf("CPUID not available on this architecture\n");
}

void cache_line_optimized_access(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3;
    }
}
#endif

/* Runtime validation of cache parameters */
void validate_cache_parameters(void) {
#if defined(_SC_LEVEL1_DCACHE_LINESIZE)
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 cache line size: %ld bytes\n", cache_line);
#endif
    
#if defined(_SC_LEVEL1_DCACHE_SIZE)
    long cache_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    printf("System L1 cache size: %ld bytes\n", cache_size);
#endif
    
    /* Compile-time assertion for x86 features */
#if defined(__i386__) || defined(__x86_64__)
    static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
#endif
}

/* Main test program with cache-sensitive computation */
int main(void) {
    /* Pattern B: Extensive use of CPU detection builtins */
    __builtin_cpu_init();
    
    printf("CPU Detection Results:\n");
    printf("  SSE2: %d\n", __builtin_cpu_supports("sse2"));
    printf("  SSE4.2: %d\n", __builtin_cpu_supports("sse4.2"));
    printf("  AVX: %d\n", __builtin_cpu_supports("avx"));
    printf("  AVX2: %d\n", __builtin_cpu_supports("avx2"));
    
    /* Check for specific Intel microarchitectures */
    if (__builtin_cpu_is("core2")) {
        printf("Detected: Intel Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("Detected: Intel Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("Detected: Intel Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("Detected: Intel Ivy Bridge\n");
    }
    
    /* Validate cache parameters */
    validate_cache_parameters();
    
    /* Create test data sized to trigger different cache cases */
    const int test_sizes[] = {
        2048,   /* 8KB with 4-byte ints */
        4096,   /* 16KB */
        8192,   /* 32KB */
        32768,  /* 128KB */
        65536,  /* 256KB */
        131072, /* 512KB */
        262144, /* 1MB */
        524288  /* 2MB */
    };
    
    int total_checksum = 0;
    
    for (size_t s = 0; s < sizeof(test_sizes)/sizeof(test_sizes[0]); s++) {
        int size = test_sizes[s];
        int* data = (int*)malloc(size * sizeof(int));
        
        if (!data) continue;
        
        /* Initialize with pattern */
        for (int i = 0; i < size; i++) {
            data[i] = (i * 13) % 1024;
        }
        
        /* Use all cache optimization patterns */
        cache_sensitive_computation(data, size);
        cache_line_optimized_access(data, size);
        dynamic_cache_function(data, size);
        
#if defined(__i386__) || defined(__x86_64__)
        /* Call architecture-specific versions */
        core2_optimized(data, size);
        nehalem_optimized(data, size);
        sandybridge_optimized(data, size);
#endif
        
        /* Compute checksum */
        int checksum = 0;
        for (int i = 0; i < size && i < 1000; i++) {
            checksum ^= data[i];
        }
        total_checksum ^= checksum;
        
        printf("Size %d: checksum = %08x\n", size * (int)sizeof(int), checksum);
        
        free(data);
    }
    
    printf("Total checksum: %08x\n", total_checksum);
    
    /* Final CPUID query */
#if defined(__i386__) || defined(__x86_64__)
    query_cpuid_cache_info();
#endif
    
    return 0;
}
