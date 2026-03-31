/* test_cache_detection.c - Trigger GCC driver's CPUID cache descriptor parsing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Function 1: Core2 target - should trigger various cache descriptors */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i += 8) {
        __builtin_prefetch(&data[i + 32], 0, 3);  /* High temporal locality */
        data[i] = data[i] * 3 + 1;
    }
}

/* Function 2: Nehalem target - different cache descriptors */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    /* Use 64-byte stride to match cache lines */
    for (int i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 64], 0, 2);  /* Medium locality */
        data[i] = data[i] * 5 - 2;
    }
}

/* Function 3: Sandy Bridge target */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    /* 32-byte stride for potential 32-byte cache lines */
    for (int i = 0; i < size; i += 8) {
        __builtin_prefetch(&data[i + 128], 1, 1);  /* Write prefetch, low locality */
        data[i] = data[i] * 7 + 3;
    }
}

/* Function 4: Ivy Bridge target */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    /* Mixed access pattern */
    for (int i = 0; i < size; i += 4) {
        if (i % 64 == 0) {  /* Every cache line */
            __builtin_prefetch(&data[i], 0, 0);  /* No locality hint */
        }
        data[i] = data[i] * 11 - 5;
    }
}

/* Multi-versioned function using target_clones */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void compute_multiversion(int* data, int size) {
    /* Default implementation */
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

/* ifunc resolver for runtime dispatch */
static void (*resolve_compute(void)) (int*, int) {
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_ivybridge;
    } else {
        return compute_core2;
    }
}

/* ifunc function - forces CPU detection at load time */
void compute_ifunc(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Constructor to force early CPU detection */
__attribute__((constructor))
static void init_cpu_detection(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Execute CPUID leaf 2 directly - may influence driver's cache model */
    unsigned int eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Also query leaf 4 for deterministic cache parameters */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(0)  /* Query L1 cache */
    );
}

/* Compile-time assertion for x86 features */
_Static_assert(
    __builtin_cpu_supports("sse2") || 1,  /* Always true for x86-64, may be false for i386 */
    "SSE2 support expected for x86 targets"
);

#else
/* Dummy implementations for non-x86 */
void compute_core2(int* data, int size) { (void)data; (void)size; }
void compute_nehalem(int* data, int size) { (void)data; (void)size; }
void compute_sandybridge(int* data, int size) { (void)data; (void)size; }
void compute_ivybridge(int* data, int size) { (void)data; (void)size; }
void compute_multiversion(int* data, int size) { (void)data; (void)size; }
void compute_ifunc(int* data, int size) { (void)data; (void)size; }
#endif

/* Cache-sensitive computation */
static long perform_cache_sensitive_computation(void) {
    const int CACHE_LINE_SIZE = 64;  /* Common cache line size */
    const int L1_SIZE_KB = 32;       /* Common L1 size */
    const int L1_ELEMENTS = (L1_SIZE_KB * 1024) / sizeof(int);
    
    /* Create arrays sized to specific cache sizes to trigger cache modeling */
    int* l1_array = malloc(L1_ELEMENTS * sizeof(int));
    int* l2_array = malloc(256 * 1024);  /* 256KB - common L2 size */
    int* l3_array = malloc(8 * 1024 * 1024);  /* 8MB - common L3 size */
    
    if (!l1_array || !l2_array || !l3_array) {
        free(l1_array);
        free(l2_array);
        free(l3_array);
        return 0;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < L1_ELEMENTS; i++) {
        l1_array[i] = i % 256;
    }
    
    /* Perform computations that should trigger cache detection */
#if defined(__i386__) || defined(__x86_64__)
    /* Extensive CPU feature checking - forces driver to detect CPU */
    if (__builtin_cpu_is("intel")) {
        printf("Detected Intel CPU\n");
    }
    if (__builtin_cpu_is("core2")) {
        printf("Detected Core2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("Detected Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("Detected Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("Detected Ivy Bridge\n");
    }
    
    /* Check specific features that correlate with cache descriptors */
    printf("SSE: %d\n", __builtin_cpu_supports("sse"));
    printf("SSE2: %d\n", __builtin_cpu_supports("sse2"));
    printf("SSE3: %d\n", __builtin_cpu_supports("sse3"));
    printf("SSSE3: %d\n", __builtin_cpu_supports("ssse3"));
    printf("SSE4.1: %d\n", __builtin_cpu_supports("sse4.1"));
    printf("SSE4.2: %d\n", __builtin_cpu_supports("sse4.2"));
    printf("AVX: %d\n", __builtin_cpu_supports("avx"));
    printf("AVX2: %d\n", __builtin_cpu_supports("avx2"));
#endif
    
    /* Call all target-specific functions */
    compute_core2(l1_array, L1_ELEMENTS);
    compute_nehalem(l1_array, L1_ELEMENTS / 2);
    compute_sandybridge(l2_array, (256 * 1024) / sizeof(int));
    compute_ivybridge(l3_array, (8 * 1024 * 1024) / sizeof(int));
    
    /* Call multi-versioned function */
    compute_multiversion(l1_array, L1_ELEMENTS);
    
    /* Call ifunc function */
    compute_ifunc(l2_array, (256 * 1024) / sizeof(int));
    
    /* Cache-blocked matrix-style computation */
    long sum = 0;
    const int BLOCK_SIZE = CACHE_LINE_SIZE / sizeof(int);
    
    for (int i = 0; i < L1_ELEMENTS; i += BLOCK_SIZE) {
        for (int j = 0; j < BLOCK_SIZE && (i + j) < L1_ELEMENTS; j++) {
            sum += l1_array[i + j];
        }
    }
    
    /* Verify system cache line size */
    long sys_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 cache line size: %ld bytes\n", sys_line_size);
    
    free(l1_array);
    free(l2_array);
    free(l3_array);
    
    return sum;
}

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    printf("Running on x86 architecture\n");
    
    /* Force CPU initialization again in main */
    __builtin_cpu_init();
    
    /* Additional CPUID calls */
    unsigned int eax, ebx, ecx, edx;
    
    /* Query CPUID leaf 0 to get vendor string */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );
    
    /* Query CPUID leaf 1 for family/model/stepping */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    
    printf("CPUID leaf 1: eax=0x%08x\n", eax);
    
    /* Query cache information from leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 4; i++) {  /* Query first few cache levels */
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
        printf("Cache level %d: eax=0x%08x, ebx=0x%08x, ecx=0x%08x, edx=0x%08x\n",
               i, eax, ebx, ecx, edx);
    }
#endif
    
    long result = perform_cache_sensitive_computation();
    printf("Computation result: %ld\n", result);
    
    return 0;
}
