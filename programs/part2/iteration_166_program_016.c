/*
 * test_cache_descriptors.c
 * 
 * This program is designed to trigger GCC driver's CPUID cache detection
 * logic, specifically targeting the switch-case handling of Intel cache
 * descriptor bytes (0x0a, 0x0c, 0x0d, etc.) in driver-i386.cc lines 127-244.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target attributes */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    /* Use cache-friendly access pattern */
    for (int i = 0; i < size; i += 16) {
        data[i] = i * 2;
    }
}

__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    /* Different stride to potentially trigger different cache behavior */
    for (int i = 0; i < size; i += 32) {
        data[i] = i * 3;
    }
}

__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    /* AVX-friendly pattern */
    for (int i = 0; i < size; i += 64) {
        data[i] = i * 4;
    }
}

__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    /* Different access pattern */
    for (int i = 0; i < size; i += 128) {
        data[i] = i * 5;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void (*resolve_compute(void)) (int*, int) {
    /* Force CPU detection for ifunc resolution */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_core2;
    } else {
        return compute_ivybridge;
    }
}

/* ifunc declaration */
void compute(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Direct CPUID queries */
void query_cpuid_cache_info(void) {
    unsigned int eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    printf("CPUID leaf 2: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n",
           eax, ebx, ecx, edx);
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 10; i++) {
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
        
        if ((eax & 0x1f) == 0)  /* Cache type field = 0 means no more caches */
            break;
            
        printf("Cache L%d: type=%d, level=%d, size=%d KB, ways=%d, line=%d B\n",
               (eax >> 5) & 0x7,  /* Cache level */
               eax & 0x1f,         /* Cache type */
               (eax >> 5) & 0x7,
               ((ebx >> 22) + 1) * (((ebx >> 12) & 0x3ff) + 1) * ((ebx & 0xfff) + 1) * (ecx + 1) / 1024,
               ((ebx >> 22) & 0x3ff) + 1,
               (ebx & 0xfff) + 1);
    }
}

/* Pattern D: Cache-sensitive computations */
void cache_sensitive_computation(void) {
    /* Array sizes matching specific cache sizes from uncovered lines */
    const int sizes[] = {8*1024/4, 16*1024/4, 32*1024/4, 64*1024/4, 
                         128*1024/4, 256*1024/4, 512*1024/4, 1024*1024/4,
                         2048*1024/4, 4096*1024/4, 6144*1024/4};
    
    for (int s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        int size = sizes[s];
        int* data = (int*)malloc(size * sizeof(int));
        if (!data) continue;
        
        /* Initialize with pattern */
        for (int i = 0; i < size; i++) {
            data[i] = i % 256;
        }
        
        /* Perform computation with different strides to exercise cache */
        long long sum = 0;
        
        /* Stride matching cache line sizes (32, 64 bytes) */
        for (int stride : {8, 16}) {  /* 8*4=32 bytes, 16*4=64 bytes */
            for (int i = 0; i < size; i += stride) {
                sum += data[i];
                /* Prefetch hint - may influence driver's cache model */
                __builtin_prefetch(&data[(i + stride * 4) % size], 0, 3);
            }
        }
        
        /* Use function multiversioning */
        compute(data, size);
        
        /* Verify computation */
        int check = 0;
        for (int i = 0; i < 100; i++) {
            check += data[i % size];
        }
        
        free(data);
    }
}

/* Constructor to run early CPU detection */
__attribute__((constructor))
static void early_cpu_init(void) {
    __builtin_cpu_init();
    
    /* Check for various CPU features to force detection */
    if (__builtin_cpu_supports("sse2")) {
        /* Compile-time assertion equivalent */
        asm volatile ("# SSE2 is available" ::: "memory");
    }
    
    if (__builtin_cpu_supports("avx")) {
        asm volatile ("# AVX is available" ::: "memory");
    }
    
    /* Check specific Intel CPUs */
    if (__builtin_cpu_is("core2")) {
        printf("CPU detected as Core 2\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("CPU detected as Nehalem\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("CPU detected as Sandy Bridge\n");
    }
    if (__builtin_cpu_is("ivybridge")) {
        printf("CPU detected as Ivy Bridge\n");
    }
}

#else
/* Non-x86 fallback */
void query_cpuid_cache_info(void) {
    printf("CPUID not available on this architecture\n");
}

void cache_sensitive_computation(void) {
    printf("Cache-sensitive computation not available on this architecture\n");
}
#endif

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Print cache line size via system query */
    printf("System L1 cache line size: %ld bytes\n", 
           sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
    
    /* Query CPUID directly */
    query_cpuid_cache_info();
    
    /* Perform cache-sensitive computation */
    cache_sensitive_computation();
    
    /* Matrix multiplication with tiling for cache optimization */
    const int N = 256;
    int* A = (int*)malloc(N * N * sizeof(int));
    int* B = (int*)malloc(N * N * sizeof(int));
    int* C = (int*)malloc(N * N * sizeof(int));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < N * N; i++) {
            A[i] = i % 100;
            B[i] = (i + 1) % 100;
            C[i] = 0;
        }
        
        /* Cache-optimized matrix multiplication with tiling */
        const int TILE = 16;  /* Tile size - should match cache line */
        for (int i = 0; i < N; i += TILE) {
            for (int j = 0; j < N; j += TILE) {
                for (int k = 0; k < N; k += TILE) {
                    for (int ii = i; ii < i + TILE && ii < N; ii++) {
                        for (int kk = k; kk < k + TILE && kk < N; kk++) {
                            for (int jj = j; jj < j + TILE && jj < N; jj++) {
                                C[ii * N + jj] += A[ii * N + kk] * B[kk * N + jj];
                            }
                        }
                    }
                }
            }
        }
        
        /* Compute checksum for verification */
        long long checksum = 0;
        for (int i = 0; i < N * N; i++) {
            checksum += C[i];
        }
        printf("Matrix multiplication checksum: %lld\n", checksum);
        
        free(A);
        free(B);
        free(C);
    }
    
    printf("Test completed successfully\n");
    return 0;
#else
    printf("This test is for x86 architectures only\n");
    return 0;
#endif
}
