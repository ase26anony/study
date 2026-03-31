/* test_cache_detection.c
 * Comprehensive test to trigger GCC driver's CPUID cache descriptor parsing
 * Targets specific Intel cache descriptor bytes: 0x0a, 0x0c, 0x0d, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target attributes */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i += 2) {
        data[i] = data[i] * 5 - 3;
        if (i + 1 < size) {
            data[i + 1] = data[i + 1] * 2 + 1;
        }
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        data[i] = sum;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] << 3) | (data[i] >> 29);
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] + 1;
    }
}

static void* resolve_compute(void) {
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

__attribute__((ifunc("resolve_compute")))
void compute_dispatch(int* data, int size);

/* Pattern C: Direct CPUID queries */
static void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 10; i++) {
        asm volatile ("cpuid" 
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
        
        if ((eax & 0x1F) == 0)  /* No more cache levels */
            break;
    }
}

/* Pattern D: Cache-sensitive computations */
__attribute__((target_clones("default,arch=core2,arch=sandybridge,arch=haswell")))
void cache_sensitive_computation(int* data, int size) {
    /* Use different array sizes to match specific cache descriptors */
    const int sizes_kb[] = {8, 16, 24, 32, 128, 256, 512, 1024, 2048, 4096};
    const int line_sizes[] = {32, 64};
    
    for (int s = 0; s < sizeof(sizes_kb)/sizeof(sizes_kb[0]); s++) {
        int elements = (sizes_kb[s] * 1024) / sizeof(int);
        if (elements > size) elements = size;
        
        for (int l = 0; l < sizeof(line_sizes)/sizeof(line_sizes[0]); l++) {
            int line_elems = line_sizes[l] / sizeof(int);
            
            /* Prefetch hints with different locality levels */
            for (int i = 0; i < elements; i += line_elems) {
                __builtin_prefetch(&data[i], 0, 0);  /* Read, low locality */
                __builtin_prefetch(&data[i + line_elems/2], 0, 1); /* Read, moderate */
                __builtin_prefetch(&data[i + line_elems - 1], 0, 3); /* Read, high */
            }
            
            /* Access pattern optimized for cache line size */
            for (int i = 0; i < elements; i += line_elems) {
                int sum = 0;
                for (int j = 0; j < line_elems && (i + j) < elements; j++) {
                    sum += data[i + j];
                }
                if (i < elements) {
                    data[i] = sum;
                }
            }
        }
    }
}

/* Constructor to force early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    __builtin_cpu_init();
    query_cpuid_cache_info();
    
    /* Extensive use of __builtin_cpu_is to force driver detection */
    const char* cpu_types[] = {
        "intel", "core2", "nehalem", "sandybridge", "ivybridge",
        "haswell", "broadwell", "skylake", "cannonlake", "icelake"
    };
    
    for (int i = 0; i < sizeof(cpu_types)/sizeof(cpu_types[0]); i++) {
        if (__builtin_cpu_is(cpu_types[i])) {
            /* Force compiler to generate code for each CPU type check */
            volatile int dummy = i;
            (void)dummy;
        }
    }
}

/* Compile-time assertion for x86 features */
_Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
               "Only 32-bit or 64-bit x86 supported");

#endif /* __i386__ || __x86_64__ */

/* Main test program */
int main(int argc, char** argv) {
#if defined(__i386__) || defined(__x86_64__)
    /* Pattern B: Use builtins to query CPU features */
    __builtin_cpu_init();
    
    printf("CPU Feature Detection:\n");
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  SSE3: %s\n", __builtin_cpu_supports("sse3") ? "yes" : "no");
    printf("  SSSE3: %s\n", __builtin_cpu_supports("ssse3") ? "yes" : "no");
    printf("  SSE4.1: %s\n", __builtin_cpu_supports("sse4.1") ? "yes" : "no");
    printf("  SSE4.2: %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("  AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    
    /* Query system cache information */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("  L1 Cache Line: %ld bytes\n", cache_line);
    
    /* Create test data sized to trigger cache detection */
    const int data_size = 1024 * 1024;  /* 1MB */
    int* data = (int*)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < data_size; i++) {
        data[i] = i % 256;
    }
    
    /* Execute all computation patterns */
    compute_core2(data, 1024);
    compute_nehalem(data, 2048);
    compute_sandybridge(data, 4096);
    compute_ivybridge(data, 8192);
    
    compute_dispatch(data, data_size);
    
    cache_sensitive_computation(data, data_size);
    
    /* Verify computation */
    unsigned long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum += (unsigned int)data[i];
    }
    
    printf("Computation checksum: %llu\n", checksum);
    printf("Test completed successfully\n");
    
    free(data);
    return 0;
#else
    printf("Non-x86 architecture - skipping CPU cache detection tests\n");
    return 0;
#endif
}
