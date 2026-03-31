/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPUID leaf 2 cache detection
 * logic by compiling for specific x86 microarchitectures and performing
 * cache-intensive operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our cache tests */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
#if defined(TARGET_PENTIUM3)
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#elif defined(TARGET_PENTIUM4)
/* Targets cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
__attribute__((target("arch=pentium4")))
#elif defined(TARGET_NOCONA)
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68 */
__attribute__((target("arch=nocona")))
#elif defined(TARGET_K8)
/* Targets cases: 0x78-0x87 */
__attribute__((target("arch=k8")))
#elif defined(TARGET_CORE2)
/* Targets cases: 0x48, 0x4e */
__attribute__((target("arch=core2")))
#else
/* Generic x86-64 - will use actual CPU detection */
__attribute__((target("default")))
#endif
static void cache_intensive_operation(int *buffer, size_t size, int iterations) {
    /* Simple linear congruential generator for pseudo-random access */
    uint32_t seed = 0xDEADBEEF;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Mix sequential and random access patterns */
        for (size_t i = 0; i < size; i++) {
            /* Sequential write */
            buffer[i] = (int)(i ^ seed);
            
            /* Every 16th element, do a backward reference */
            if ((i & 0xF) == 0 && i > 16) {
                buffer[i] += buffer[i - 16];
            }
        }
        
        COMPILER_BARRIER();
        
        /* Random access pattern */
        seed = seed * 1664525 + 1013904223;
        for (size_t i = 0; i < size / 4; i++) {
            uint32_t idx = seed % size;
            buffer[idx] ^= (int)seed;
            seed = seed * 1664525 + 1013904223;
        }
        
        COMPILER_BARRIER();
    }
}

/* Special function to potentially trigger case 0x49 with Xeon MP detection */
#if defined(TARGET_XEON_MP)
__attribute__((target("arch=nocona")))
static void xeon_mp_cache_test(int *buffer, size_t size) {
    /* Different access pattern that might affect Xeon MP detection */
    for (size_t i = 0; i < size; i += 64) {  /* 64-byte stride */
        buffer[i] = i;
    }
    COMPILER_BARRIER();
    
    /* Reverse access */
    for (size_t i = size - 1; i > 0; i -= 64) {
        buffer[i] += buffer[i - 64];
    }
}
#endif

/* Multiple cache size tests to ensure we hit different levels */
static void test_cache_sizes(void) {
    const size_t l1_size = 32 * 1024 / sizeof(int);      /* ~32KB */
    const size_t l2_size = 256 * 1024 / sizeof(int);     /* ~256KB */
    const size_t l3_size = 8 * 1024 * 1024 / sizeof(int); /* ~8MB */
    
    int *buffer1 = (int*)malloc(l1_size * sizeof(int));
    int *buffer2 = (int*)malloc(l2_size * sizeof(int));
    int *buffer3 = (int*)malloc(l3_size * sizeof(int));
    
    if (!buffer1 || !buffer2 || !buffer3) {
        fprintf(stderr, "Memory allocation failed\n");
        free(buffer1);
        free(buffer2);
        free(buffer3);
        return;
    }
    
    /* Initialize buffers */
    memset(buffer1, 0, l1_size * sizeof(int));
    memset(buffer2, 0, l2_size * sizeof(int));
    memset(buffer3, 0, l3_size * sizeof(int));
    
    /* Test different cache levels with different intensities */
    cache_intensive_operation(buffer1, l1_size, 1000);
    cache_intensive_operation(buffer2, l2_size, 100);
    cache_intensive_operation(buffer3, l3_size, 10);
    
#if defined(TARGET_XEON_MP)
    xeon_mp_cache_test(buffer2, l2_size);
#endif
    
    /* Prevent dead code elimination */
    volatile int checksum = 0;
    for (size_t i = 0; i < l1_size; i += 64) checksum ^= buffer1[i];
    for (size_t i = 0; i < l2_size; i += 256) checksum ^= buffer2[i];
    for (size_t i = 0; i < l3_size; i += 1024) checksum ^= buffer3[i];
    
    printf("Cache test checksum: %d\n", checksum);
    
    free(buffer1);
    free(buffer2);
    free(buffer3);
}

/* Matrix multiplication to stress cache hierarchy */
#if defined(TARGET_ALL_CASES)
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void matrix_multiply_cache_test(void) {
    const int N = 256;  /* Fits in L2 cache for some CPUs, spills for others */
    static int A[256][256];
    static int B[256][256];
    static int C[256][256];
    
    /* Initialize matrices */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = i + j;
            B[i][j] = i - j;
            C[i][j] = 0;
        }
    }
    
    /* Blocked matrix multiplication - cache aware */
    const int BLOCK = 32;
    for (int i0 = 0; i0 < N; i0 += BLOCK) {
        for (int j0 = 0; j0 < N; j0 += BLOCK) {
            for (int k0 = 0; k0 < N; k0 += BLOCK) {
                for (int i = i0; i < i0 + BLOCK && i < N; i++) {
                    for (int j = j0; j < j0 + BLOCK && j < N; j++) {
                        int sum = C[i][j];
                        for (int k = k0; k < k0 + BLOCK && k < N; k++) {
                            sum += A[i][k] * B[k][j];
                        }
                        C[i][j] = sum;
                    }
                }
            }
        }
    }
    
    /* Use result to prevent elimination */
    volatile int total = 0;
    for (int i = 0; i < N; i++) {
        total += C[i][i];
    }
    printf("Matrix total: %d\n", total);
}

int main(void) {
    printf("Starting cache detection tests...\n");
    
    /* Test 1: Various cache size operations */
    test_cache_sizes();
    
    /* Test 2: Matrix multiplication with different access patterns */
    matrix_multiply_cache_test();
    
    /* Test 3: Pointer chasing to stress cache associativity */
    {
        const int CHASE_SIZE = 1024 * 1024;  /* 1M elements */
        int *chase_buffer = (int*)malloc(CHASE_SIZE * sizeof(int));
        if (chase_buffer) {
            /* Create a linked list in random order */
            for (int i = 0; i < CHASE_SIZE - 1; i++) {
                chase_buffer[i] = i + 1;
            }
            chase_buffer[CHASE_SIZE - 1] = 0;
            
            /* Pointer chasing */
            int idx = 0;
            volatile int chase_sum = 0;
            for (int i = 0; i < CHASE_SIZE * 10; i++) {
                idx = chase_buffer[idx];
                chase_sum += idx;
            }
            
            printf("Pointer chase sum: %d\n", chase_sum);
            free(chase_buffer);
        }
    }
    
    return 0;
}
