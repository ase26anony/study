/* test_cache_detection.c - Trigger GCC driver CPU cache detection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Function multiversioning with different target architectures */
/* Each target may cause different CPUID cache descriptor parsing */

/* Core2 architecture - may trigger descriptors like 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 1;
    }
}

/* Nehalem architecture - may trigger descriptors like 0x0a, 0x0c, 0x0d */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 - 2;
    }
}

/* Sandy Bridge architecture - may trigger descriptors like 0x2c, 0x3a, 0x3b */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 3;
    }
}

/* Ivy Bridge architecture - may trigger descriptors like 0x78, 0x79, 0x7a */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 - 5;
    }
}

/* Haswell architecture - may trigger descriptors like 0x3c, 0x3d, 0x3e */
__attribute__((target("arch=haswell")))
void compute_haswell(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 13 + 7;
    }
}

/* Function with target clones - forces driver to detect CPU features for each version */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge,arch=haswell")))
void compute_multiversion(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

/* ifunc resolver for runtime dispatch */
static void (*resolve_compute(void)) (int*, int) {
    if (__builtin_cpu_supports("avx2")) {
        return compute_haswell;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("ssse3")) {
        return compute_core2;
    } else {
        return compute_multiversion;
    }
}

/* ifunc function - triggers CPU detection during resolution */
void compute_ifunc(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Constructor that runs CPUID queries before main */
__attribute__((constructor))
static void init_cpu_detection(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Execute CPUID leaf 2 (cache descriptors) directly */
    unsigned int eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Also query CPUID leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
    }
}

/* Compile-time assertion for x86 features */
_Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");

#else
/* Dummy functions for non-x86 platforms */
void compute_multiversion(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

void compute_ifunc(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

__attribute__((constructor))
static void init_cpu_detection(void) {
    /* Nothing to do on non-x86 */
}
#endif

/* Cache-sensitive computation with prefetching */
static void cache_sensitive_computation(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Use __builtin_cpu_is to trigger CPU detection */
    if (__builtin_cpu_is("intel")) {
        printf("Intel CPU detected\n");
    }
    if (__builtin_cpu_is("core2")) {
        printf("Core2 microarchitecture\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("Nehalem microarchitecture\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("Sandy Bridge microarchitecture\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("Ivy Bridge microarchitecture\n");
    }
    
    /* Check various CPU features to trigger detection */
    const char* features[] = {
        "sse", "sse2", "sse3", "ssse3", "sse4.1", "sse4.2",
        "avx", "avx2", "fma", "aes", "pclmul"
    };
    
    printf("CPU features: ");
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf("%s ", features[i]);
        }
    }
    printf("\n");
#endif
    
    /* Create arrays sized to match specific cache sizes */
    const int size_8kb = 8192 / sizeof(int);      /* 8KB L1 cache */
    const int size_16kb = 16384 / sizeof(int);    /* 16KB L1 cache */
    const int size_32kb = 32768 / sizeof(int);    /* 32KB L1 cache */
    const int size_256kb = 262144 / sizeof(int);  /* 256KB L2 cache */
    const int size_1mb = 1048576 / sizeof(int);   /* 1MB L2 cache */
    const int size_2mb = 2097152 / sizeof(int);   /* 2MB L2 cache */
    
    /* Allocate and initialize arrays */
    int* data_small = (int*)malloc(size_8kb * sizeof(int));
    int* data_medium = (int*)malloc(size_256kb * sizeof(int));
    int* data_large = (int*)malloc(size_2mb * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize data */
    for (int i = 0; i < size_8kb; i++) data_small[i] = i;
    for (int i = 0; i < size_256kb; i++) data_medium[i] = i % 256;
    for (int i = 0; i < size_2mb; i++) data_large[i] = i % 1024;
    
    /* Perform cache-sensitive computations with prefetching */
    for (int iter = 0; iter < 100; iter++) {
        /* Process small array (fits in L1) */
        for (int i = 0; i < size_8kb; i += 8) {
            /* Prefetch with different locality hints */
            __builtin_prefetch(&data_small[i + 16], 0, 0); /* Low locality */
            __builtin_prefetch(&data_small[i + 32], 0, 1); /* Medium locality */
            __builtin_prefetch(&data_small[i + 64], 0, 3); /* High locality */
            
            /* Computation */
            data_small[i] = data_small[i] * 3 + data_small[i+1];
        }
        
        /* Process medium array (fits in L2) */
        for (int i = 0; i < size_256kb; i += 16) {
            __builtin_prefetch(&data_medium[i + 64], 0, 2);
            data_medium[i] = data_medium[i] * 5 - data_medium[i+2];
        }
        
        /* Process large array (exceeds L2) */
        for (int i = 0; i < size_2mb; i += 32) {
            __builtin_prefetch(&data_large[i + 128], 0, 1);
            data_large[i] = data_large[i] * 7 + data_large[i+4];
        }
        
        /* Call multiversion functions */
        compute_multiversion(data_small, size_8kb);
        compute_ifunc(data_medium, size_256kb);
        
#if defined(__i386__) || defined(__x86_64__)
        /* Call architecture-specific functions */
        compute_core2(data_small, size_8kb);
        compute_nehalem(data_medium, size_256kb);
        compute_sandybridge(data_large, size_2mb);
        compute_ivybridge(data_small, size_8kb);
        compute_haswell(data_medium, size_256kb);
#endif
    }
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (int i = 0; i < size_8kb; i++) checksum += data_small[i];
    for (int i = 0; i < size_256kb; i++) checksum += data_medium[i];
    for (int i = 0; i < size_2mb; i++) checksum += data_large[i];
    
    printf("Computed checksum: %llu\n", checksum);
    
    /* Clean up */
    free(data_small);
    free(data_medium);
    free(data_large);
}

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Print cache information if available */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (cache_line > 0) {
        printf("L1 cache line size: %ld bytes\n", cache_line);
    }
    
    /* Additional CPUID queries */
    unsigned int eax, ebx, ecx, edx;
    
    /* Query CPUID leaf 0 to get vendor string */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    char vendor[13];
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';
    printf("CPU vendor: %s\n", vendor);
    
    /* Query CPUID leaf 1 for family/model/stepping */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    printf("CPU family: %d, model: %d, stepping: %d\n", 
           (eax >> 8) & 0xF, (eax >> 4) & 0xF, eax & 0xF);
#endif
    
    /* Perform cache-sensitive computation */
    cache_sensitive_computation();
    
    return 0;
}
