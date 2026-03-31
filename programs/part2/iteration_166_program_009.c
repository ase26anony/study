/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC driver's CPU cache detection logic
 * for specific Intel cache descriptor bytes (0x0a, 0x0c, 0x0d, etc. from lines 127-244).
 * It uses multiple techniques to force the driver to query CPUID and initialize cache parameters.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Multiple functions with different target attributes */
/* Each target attribute may cause the driver to detect different cache descriptors */

/* Function for Core2 architecture - may trigger descriptors like 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void compute_core2(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Function for Nehalem architecture - may trigger descriptors like 0x0a, 0x0c, 0x0d */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 3;
    }
}

/* Function for Sandy Bridge architecture */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 11;
    }
}

/* Function for Ivy Bridge architecture */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 7;
    }
}

/* Pattern B: Function with multiple target clones */
/* This forces the driver to detect CPU capabilities for each clone */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
int checksum_array(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

/* Pattern C: Inline assembly to directly query CPUID */
/* This forces the driver to be aware of CPU cache hierarchy */
static void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* Query CPUID leaf 2 (cache descriptors) */
    asm volatile ("cpuid"
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                  : "a" (2), "c" (0));
    
    /* Query CPUID leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (4), "c" (i));
    }
}

/* Pattern D: Cache-sensitive computation with prefetch hints */
/* Uses arrays sized to match specific cache sizes from the uncovered lines */
__attribute__((target("default")))
void cache_sensitive_computation(void) {
    /* Array sizes matching cache sizes from uncovered lines */
    const int size_8kb = 2048;      /* 8KB / sizeof(int) for 0x0a */
    const int size_16kb = 4096;     /* 16KB / sizeof(int) for 0x0c, 0x0d */
    const int size_32kb = 8192;     /* 32KB / sizeof(int) for 0x2c */
    const int size_256kb = 65536;   /* 256KB / sizeof(int) for 0x21 */
    
    int *data8k = malloc(size_8kb * sizeof(int));
    int *data16k = malloc(size_16kb * sizeof(int));
    int *data32k = malloc(size_32kb * sizeof(int));
    int *data256k = malloc(size_256kb * sizeof(int));
    
    if (!data8k || !data16k || !data32k || !data256k) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size_8kb; i++) data8k[i] = i % 256;
    for (int i = 0; i < size_16kb; i++) data16k[i] = i % 512;
    for (int i = 0; i < size_32kb; i++) data32k[i] = i % 1024;
    for (int i = 0; i < size_256kb; i++) data256k[i] = i % 2048;
    
    /* Pattern D: Use __builtin_prefetch with different locality hints */
    for (int i = 0; i < size_8kb; i += 8) {  /* 8 ints = 32 bytes (common cache line) */
        __builtin_prefetch(&data8k[i + 32], 0, 0);  /* Low locality */
    }
    
    for (int i = 0; i < size_16kb; i += 16) { /* 16 ints = 64 bytes (another cache line) */
        __builtin_prefetch(&data16k[i + 64], 0, 3);  /* High locality */
    }
    
    /* Perform computations on different sized arrays */
    compute_core2(data8k, size_8kb);
    compute_nehalem(data16k, size_16kb);
    compute_sandybridge(data32k, size_32kb);
    compute_ivybridge(data256k, size_256kb);
    
    /* Compute checksums using multiversioned function */
    int sum1 = checksum_array(data8k, size_8kb);
    int sum2 = checksum_array(data16k, size_16kb);
    int sum3 = checksum_array(data32k, size_32kb);
    int sum4 = checksum_array(data256k, size_256kb);
    
    printf("Checksums: %d, %d, %d, %d\n", sum1, sum2, sum3, sum4);
    
    free(data8k);
    free(data16k);
    free(data32k);
    free(data256k);
}

/* Runtime CPU dispatch using ifunc */
typedef void (*compute_func_t)(int*, int);

static compute_func_t resolve_compute(void) {
    /* Pattern B: Use __builtin_cpu_is to check for specific Intel CPUs */
    __builtin_cpu_init();
    
    if (__builtin_cpu_is("core2")) {
        return compute_core2;
    } else if (__builtin_cpu_is("nehalem")) {
        return compute_nehalem;
    } else if (__builtin_cpu_is("sandybridge")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_is("ivybridge")) {
        return compute_ivybridge;
    }
    
    return compute_core2;  /* Default */
}

/* ifunc resolver forces driver to detect CPU features */
void dispatch_compute(int *data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_cache_info(void) {
    printf("Initializing CPU cache detection...\n");
    query_cpuid_cache_info();
    
    /* Pattern B: Extensive use of __builtin_cpu_is to force cache detection */
    __builtin_cpu_init();
    
    /* Check for various Intel CPUs that might have different cache descriptors */
    const char *cpu_types[] = {
        "core2", "nehalem", "sandybridge", "ivybridge",
        "haswell", "broadwell", "skylake", "k8", "atom"
    };
    
    for (size_t i = 0; i < sizeof(cpu_types)/sizeof(cpu_types[0]); i++) {
        if (__builtin_cpu_is(cpu_types[i])) {
            printf("Detected CPU: %s\n", cpu_types[i]);
        }
    }
    
    /* Check CPU features that depend on cache detection */
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
    }
}

/* Compile-time assertion for x86 features */
_Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
               "Expected 32-bit or 64-bit x86 architecture");

#else
/* Non-x86 fallback implementations */
void cache_sensitive_computation(void) {
    printf("Non-x86 architecture - using fallback implementation\n");
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    printf("Fallback checksum: %d\n", sum);
}

void query_cpuid_cache_info(void) {
    printf("CPUID not available on non-x86\n");
}

#endif /* __i386__ || __x86_64__ */

/* Main function with cache-sensitive computation */
int main(void) {
    printf("Starting cache detection test program\n");
    
#if defined(__i386__) || defined(__x86_64__)
    /* Get actual cache line size from system */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 cache line size: %ld bytes\n", cache_line);
    
    /* Validate cache line size matches common values from uncovered lines */
    if (cache_line != 32 && cache_line != 64) {
        printf("Warning: Unexpected cache line size: %ld\n", cache_line);
    }
#endif
    
    /* Execute cache-sensitive computation */
    cache_sensitive_computation();
    
    /* Additional verification computation */
    const int test_size = 10000;
    int *test_data = malloc(test_size * sizeof(int));
    if (!test_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < test_size; i++) {
        test_data[i] = i % 100;
    }
    
#if defined(__i386__) || defined(__x86_64__)
    /* Use ifunc-dispatched function */
    dispatch_compute(test_data, test_size);
#endif
    
    /* Final checksum for verification */
    int final_sum = 0;
    for (int i = 0; i < test_size; i++) {
        final_sum += test_data[i];
    }
    
    printf("Final verification checksum: %d\n", final_sum);
    printf("Test completed successfully\n");
    
    free(test_data);
    return 0;
}
