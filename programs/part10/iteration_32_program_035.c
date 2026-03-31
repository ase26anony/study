/* driver_cache_test.c - Comprehensive test for CPU cache detection logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function with Core2 target attribute */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size) {
    /* Large stride access pattern */
    for (size_t i = 0; i < size; i += 64) {
        data[i] = data[i] * 3 + 7;
    }
    /* Prefetch hints */
    for (size_t i = 0; i < size; i += 128) {
        __builtin_prefetch(&data[i + 64], 0, 3);
    }
}

/* Function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size / 2; i++) {
        data[i] = data[size - i - 1] + data[i];
    }
}

/* Function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size) {
    /* Matrix-style access */
    const int block = 32;
    for (int i = 0; i < block; i++) {
        for (int j = 0; j < block; j++) {
            data[i * block + j] = data[j * block + i] * 2;
        }
    }
}

/* Function that uses CPUID directly */
static void cpuid_cache_info(uint32_t leaf, uint32_t* eax, uint32_t* ebx, 
                             uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

/* Main computational kernel with varied access patterns */
static int compute_kernel(int use_avx, int use_sse42, int use_sse2) {
    /* Large aligned arrays to stress cache detection */
    __attribute__((aligned(64))) static int array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int array2[512 * 512];    /* 1MB */
    __attribute__((aligned(64))) static int array3[256 * 256];    /* 256KB */
    
    int checksum = 0;
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = lcg_rand() % 100;
    }
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i++) {
        array2[i] = lcg_rand() % 100;
    }
    for (size_t i = 0; i < sizeof(array3)/sizeof(array3[0]); i++) {
        array3[i] = lcg_rand() % 100;
    }
    
    /* Different computation paths based on CPU features */
    if (use_sse2) {
        /* Sequential access pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]) - 1; i++) {
            array1[i] = array1[i] + array1[i + 1];
        }
        checksum += array1[0];
    }
    
    if (use_sse42) {
        /* Strided access pattern (every 16th element) */
        for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i += 16) {
            array2[i] = array2[i] * 3 - 5;
        }
        checksum += array2[16];
    }
    
    if (use_avx) {
        /* Block access pattern */
        const int block_size = 64;
        for (int i = 0; i < sizeof(array3)/sizeof(array3[0]); i += block_size) {
            int block_sum = 0;
            for (int j = 0; j < block_size && (i + j) < sizeof(array3)/sizeof(array3[0]); j++) {
                block_sum += array3[i + j];
            }
            array3[i] = block_sum;
        }
        checksum += array3[0];
    }
    
    /* Call architecture-specific functions */
    core2_optimized_compute(array1, sizeof(array1)/sizeof(array1[0]));
    nehalem_optimized_compute(array2, sizeof(array2)/sizeof(array2[0]));
    sandybridge_optimized_compute(array3, sizeof(array3)/sizeof(array3[0]));
    
    /* Final reduction */
    for (size_t i = 0; i < 1000; i++) {
        checksum += array1[i % 1024] + array2[i % 512] + array3[i % 256];
    }
    
    return checksum;
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    /* Detect CPU features */
    int has_sse2 = __builtin_cpu_supports("sse2");
    int has_sse42 = __builtin_cpu_supports("sse4.2");
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    
    printf("CPU Features detected:\n");
    printf("  SSE2: %s\n", has_sse2 ? "yes" : "no");
    printf("  SSE4.2: %s\n", has_sse42 ? "yes" : "no");
    printf("  AVX: %s\n", has_avx ? "yes" : "no");
    printf("  AVX2: %s\n", has_avx2 ? "yes" : "no");
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors */
    cpuid_cache_info(2, &eax, &ebx, &ecx, &edx);
    printf("CPUID leaf 2: eax=%08x ebx=%08x ecx=%08x edx=%08x\n", 
           eax, ebx, ecx, edx);
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        eax = 4;
        ecx = i;
        __asm__ volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
        printf("CPUID leaf 4[%d]: eax=%08x ebx=%08x ecx=%08x edx=%08x\n",
               i, eax, ebx, ecx, edx);
    }
    
    /* Conditional compilation based on optimization level */
#ifdef __OPTIMIZE__
    printf("Optimization level: O%d\n", 
#  ifdef __OPTIMIZE_SIZE__
    1
#  elif __OPTIMIZE__ == 1
    1
#  elif __OPTIMIZE__ == 2
    2
#  elif __OPTIMIZE__ == 3
    3
#  else
    2
#  endif
    );
#endif
    
    /* Execute different computational paths based on CPU features */
    int result = 0;
    
    if (has_sse2) {
        result += compute_kernel(0, 0, 1);
    }
    
    if (has_sse42) {
        result += compute_kernel(0, 1, 0);
    }
    
    if (has_avx) {
        result += compute_kernel(1, 0, 0);
    }
    
    /* Use result to prevent dead code elimination */
    __asm__ volatile ("" : : "r"(result));
    
    printf("Computation result: %d\n", result);
    
    return 0;
}
