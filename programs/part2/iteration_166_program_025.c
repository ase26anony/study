/* test_cache_detection.c - Comprehensive test for Intel CPU cache descriptor detection */

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
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 5 + 11;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 7 + 13;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 11 + 17;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
typedef void (*compute_func_t)(int*, int);

static compute_func_t resolve_compute() {
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_nehalem;
    } else {
        return compute_core2;
    }
}

void compute_dynamic(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Target clones for automatic multiversioning */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void compute_multiversion(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        data[i] = sum % 256;
    }
}

/* Pattern D: Cache-sensitive computation with prefetching */
void cache_sensitive_computation() {
    /* Arrays sized to match various cache sizes from the uncovered lines */
    int array_8kb[2048];      /* 8KB cache */
    int array_16kb[4096];     /* 16KB cache */
    int array_32kb[8192];     /* 32KB cache */
    int array_64kb[16384];    /* 64KB cache */
    int array_128kb[32768];   /* 128KB cache */
    int array_256kb[65536];   /* 256KB cache */
    int array_512kb[131072];  /* 512KB cache */
    int array_1mb[262144];    /* 1MB cache */
    int array_2mb[524288];    /* 2MB cache */
    int array_4mb[1048576];   /* 4MB cache */
    int array_6mb[1572864];   /* 6MB cache */
    
    /* Initialize arrays */
    for (int i = 0; i < 2048; i++) array_8kb[i] = i;
    for (int i = 0; i < 4096; i++) array_16kb[i] = i;
    for (int i = 0; i < 8192; i++) array_32kb[i] = i;
    for (int i = 0; i < 16384; i++) array_64kb[i] = i;
    for (int i = 0; i < 32768; i++) array_128kb[i] = i;
    for (int i = 0; i < 65536; i++) array_256kb[i] = i;
    for (int i = 0; i < 131072; i++) array_512kb[i] = i;
    for (int i = 0; i < 262144; i++) array_1mb[i] = i;
    for (int i = 0; i < 524288; i++) array_2mb[i] = i;
    for (int i = 0; i < 1048576; i++) array_4mb[i] = i;
    for (int i = 0; i < 1572864; i++) array_6mb[i] = i;
    
    /* Perform cache-sensitive operations with prefetching */
    int sum = 0;
    
    /* Access with different strides to test various cache line sizes */
    for (int stride = 1; stride <= 16; stride *= 2) {
        /* Test 32-byte cache lines */
        for (int i = 0; i < 2048; i += stride) {
            __builtin_prefetch(&array_8kb[i + 8], 0, 3);
            sum += array_8kb[i];
        }
        
        /* Test 64-byte cache lines */
        for (int i = 0; i < 4096; i += stride) {
            __builtin_prefetch(&array_16kb[i + 16], 0, 3);
            sum += array_16kb[i];
        }
    }
    
    /* Matrix multiplication with tiling for cache optimization */
    const int N = 256;
    int A[N][N], B[N][N], C[N][N];
    
    /* Initialize matrices */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = i + j;
            B[i][j] = i - j;
            C[i][j] = 0;
        }
    }
    
    /* Cache-blocked matrix multiplication */
    const int BLOCK = 32;  /* Block size tuned for L1 cache */
    for (int i0 = 0; i0 < N; i0 += BLOCK) {
        for (int j0 = 0; j0 < N; j0 += BLOCK) {
            for (int k0 = 0; k0 < N; k0 += BLOCK) {
                for (int i = i0; i < i0 + BLOCK && i < N; i++) {
                    for (int j = j0; j < j0 + BLOCK && j < N; j++) {
                        for (int k = k0; k < k0 + BLOCK && k < N; k++) {
                            C[i][j] += A[i][k] * B[k][j];
                        }
                    }
                }
            }
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            checksum += C[i][j];
        }
    }
    
    printf("Cache-sensitive computation checksum: %d\n", checksum);
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_detection() {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Execute CPUID leaf 2 (cache descriptors) via inline assembly */
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache and TLB Descriptors */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(2));
    
    printf("CPUID Leaf 2: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n", 
           eax, ebx, ecx, edx);
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    for (int i = 0; i < 8; i++) {  /* Check up to 8 cache levels */
        asm volatile ("cpuid" 
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                      : "a"(4), "c"(i));
        
        if ((eax & 0x1f) == 0) break;  /* No more caches */
        
        printf("Cache Level %d: type=%d, level=%d, ways=%d, sets=%d, line=%d, partitions=%d\n",
               i,
               eax & 0x1f,                    /* Cache type */
               (eax >> 5) & 0x7,              /* Cache level */
               ((ebx >> 22) & 0x3ff) + 1,     /* Ways of associativity */
               (ebx & 0x7ff) + 1,             /* Number of sets */
               (ebx >> 12) & 0x3ff,           /* Line size in bytes */
               ((ebx >> 22) & 0x1ff) + 1);    /* Physical line partitions */
    }
}

#else
/* Non-x86 fallback implementations */
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = 1;
}
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = 2;
}
void compute_sandybridge(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = 3;
}
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = 4;
}
void compute_dynamic(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = 5;
}
void compute_multiversion(int* data, int size) {
    for (int i = 0; i < size; i++) data[i] = 6;
}
void cache_sensitive_computation() {
    printf("Non-x86 architecture - using fallback implementation\n");
}
#endif

int main() {
    /* Pattern B: Extensive use of CPU detection builtins */
    printf("=== CPU Feature Detection ===\n");
    
#if defined(__i386__) || defined(__x86_64__)
    __builtin_cpu_init();
    
    /* Check for various Intel CPUs to trigger cache detection */
    if (__builtin_cpu_is("intel")) {
        printf("CPU vendor: Intel\n");
    }
    
    /* Check specific microarchitectures */
    const char* archs[] = {
        "core2", "nehalem", "sandybridge", "ivybridge",
        "haswell", "broadwell", "skylake", "kabylake"
    };
    
    for (int i = 0; i < 8; i++) {
        if (__builtin_cpu_is(archs[i])) {
            printf("Detected microarchitecture: %s\n", archs[i]);
        }
    }
    
    /* Check CPU features that correlate with cache configurations */
    const char* features[] = {
        "sse", "sse2", "sse3", "ssse3", "sse4.1", "sse4.2",
        "avx", "avx2", "fma", "aes", "pclmul"
    };
    
    printf("CPU features: ");
    for (int i = 0; i < 11; i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf("%s ", features[i]);
        }
    }
    printf("\n");
    
    /* Get cache line size via sysconf */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
    
    /* Compile-time assertion for x86 features */
    #ifdef __SSE2__
    printf("SSE2 enabled at compile time\n");
    #endif
    
    #ifdef __AVX__
    printf("AVX enabled at compile time\n");
    #endif
#endif
    
    printf("\n=== Cache-Sensitive Computation ===\n");
    
    /* Test data for function multiversioning */
    const int DATA_SIZE = 1024;
    int* data1 = (int*)malloc(DATA_SIZE * sizeof(int));
    int* data2 = (int*)malloc(DATA_SIZE * sizeof(int));
    int* data3 = (int*)malloc(DATA_SIZE * sizeof(int));
    
    for (int i = 0; i < DATA_SIZE; i++) {
        data1[i] = i;
        data2[i] = i * 2;
        data3[i] = i * 3;
    }
    
    /* Call all multiversioned functions */
    compute_core2(data1, DATA_SIZE);
    compute_nehalem(data2, DATA_SIZE);
    compute_sandybridge(data3, DATA_SIZE);
    
    compute_dynamic(data1, DATA_SIZE);
    compute_multiversion(data2, DATA_SIZE);
    
    /* Perform cache-sensitive computation */
    cache_sensitive_computation();
    
    /* Compute final checksum */
    int final_checksum = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        final_checksum += data1[i] + data2[i] + data3[i];
    }
    
    printf("\n=== Results ===\n");
    printf("Final checksum: %d\n", final_checksum);
    printf("Test completed successfully\n");
    
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
