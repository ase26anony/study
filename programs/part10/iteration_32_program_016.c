/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in GCC's driver-i386.cc, specifically targeting the switch cases
 * for cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 * 
 * Compilation strategies:
 * 1. gcc -O2 -march=core2 -fverbose-asm -c driver_cache_test.c -o driver_cache_test_core2.o
 * 2. gcc -O2 -march=nehalem -fverbose-asm -c driver_cache_test.c -o driver_cache_test_nehalem.o
 * 3. gcc -O3 -march=native -mtune=generic -fdump-rtl-all -c driver_cache_test.c
 * 4. gcc -O1 -m32 -march=pentium4 -fno-inline -c driver_cache_test.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Helper functions with different target attributes */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* arr, size_t size) {
    /* Different stride pattern for Core2 cache hints */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Sequential access with prefetch hints */
    for (size_t i = 0; i < size; i += 4) {
        __builtin_prefetch(&arr[i + 32], 0, 3);
        arr[i] = arr[i] * 5 - 2;
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Matrix-style access pattern */
    for (size_t i = 0; i < size; i += 16) {
        for (size_t j = 0; j < 4; j++) {
            arr[i + j] = arr[i + j] + arr[i + j + 4];
        }
    }
}

__attribute__((target("avx")))
void avx_optimized_loop(float* arr, size_t size) {
    /* AVX-optimized computation */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 2.0f;
    }
}

__attribute__((target("sse2")))
void sse2_optimized_loop(float* arr, size_t size) {
    /* SSE2-optimized computation */
    for (size_t i = 0; i < size; i += 4) {
        arr[i] = arr[i] / 1.5f;
    }
}

/* Inline assembly to read CPUID cache information */
static void cpuid_cache_info(uint32_t leaf, uint32_t* eax, uint32_t* ebx, 
                             uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

/* Function to process cache descriptors from CPUID leaf 2 */
static uint32_t process_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t checksum = 0;
    
    /* CPUID leaf 2 - Cache descriptors (may return multiple calls) */
    cpuid_cache_info(2, &eax, &ebx, &ecx, &edx);
    
    /* Process descriptor bytes from registers */
    uint8_t* descriptors = (uint8_t*)&eax;
    for (int i = 0; i < 4; i++) {
        if (descriptors[i] && descriptors[i] != 0xFF) {
            checksum += descriptors[i];
        }
    }
    
    descriptors = (uint8_t*)&ebx;
    for (int i = 0; i < 4; i++) {
        if (descriptors[i] && descriptors[i] != 0xFF) {
            checksum += descriptors[i];
        }
    }
    
    descriptors = (uint8_t*)&ecx;
    for (int i = 0; i < 4; i++) {
        if (descriptors[i] && descriptors[i] != 0xFF) {
            checksum += descriptors[i];
        }
    }
    
    descriptors = (uint8_t*)&edx;
    for (int i = 0; i < 4; i++) {
        if (descriptors[i] && descriptors[i] != 0xFF) {
            checksum += descriptors[i];
        }
    }
    
    return checksum;
}

/* Deterministic cache parameters from CPUID leaf 4 */
static uint32_t get_deterministic_cache_params(void) {
    uint32_t checksum = 0;
    
    for (uint32_t level = 0; level < 4; level++) {
        uint32_t eax, ebx, ecx, edx;
        uint32_t leaf = 4;
        uint32_t subleaf = level;
        
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(leaf), "c"(subleaf)
        );
        
        /* Cache type field in bits 4:0 */
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) break; /* No more caches */
        
        checksum += cache_type;
        checksum += (eax >> 5) & 0x7;  /* Cache level */
        checksum += (eax >> 14) & 0xFFF; /* Number of threads */
        checksum += (ebx & 0xFFF) + 1;  /* Line size */
        checksum += ((ebx >> 12) & 0x3FF) + 1;  /* Physical partitions */
        checksum += ((ebx >> 22) & 0x3FF) + 1;  /* Ways of associativity */
        checksum += (ecx & 0x3FF) + 1;  /* Number of sets */
    }
    
    return checksum;
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    #define ARRAY_SIZE (1024 * 1024)
    int* int_array __attribute__((aligned(64))) = 
        (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_array __attribute__((aligned(64))) = 
        (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!int_array || !float_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (int)lcg_rand();
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Process CPUID cache information */
    checksum += process_cache_descriptors();
    checksum += get_deterministic_cache_params();
    
    /* Conditional execution based on CPU features */
    #ifdef __OPTIMIZE__
    if (__builtin_cpu_supports("avx")) {
        avx_optimized_loop(float_array, ARRAY_SIZE);
        checksum += int_array[0];
    }
    #endif
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* Matrix multiplication pattern */
        for (size_t i = 0; i < 256; i++) {
            for (size_t j = 0; j < 256; j++) {
                float sum = 0.0f;
                for (size_t k = 0; k < 256; k++) {
                    sum += float_array[i * 256 + k] * float_array[k * 256 + j];
                }
                float_array[i * 256 + j] = sum;
            }
        }
        checksum += (uint32_t)float_array[0];
    }
    
    if (__builtin_cpu_supports("sse2")) {
        sse2_optimized_loop(float_array, ARRAY_SIZE);
        checksum += int_array[1];
    }
    
    /* Call target-specific functions */
    core2_optimized_loop(int_array, ARRAY_SIZE);
    nehalem_optimized_loop(int_array, ARRAY_SIZE);
    sandybridge_optimized_loop(int_array, ARRAY_SIZE);
    
    /* Different access patterns for cache hints */
    /* Sequential access */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = int_array[i] * 2 + 1;
    }
    
    /* Strided access (every 64 bytes) */
    for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
        int_array[i] = int_array[i] - 5;
    }
    
    /* "Random" access pattern */
    for (size_t i = 0; i < ARRAY_SIZE; i += 97) {
        int_array[i % ARRAY_SIZE] = int_array[i % ARRAY_SIZE] * 3;
    }
    
    /* Histogram computation */
    int histogram[256] = {0};
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        histogram[int_array[i] & 0xFF]++;
    }
    
    /* Final checksum computation */
    for (size_t i = 0; i < ARRAY_SIZE; i += 128) {
        checksum += int_array[i];
        checksum += (uint32_t)float_array[i];
    }
    
    for (int i = 0; i < 256; i++) {
        checksum += histogram[i];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    /* Print result to avoid complete optimization */
    printf("Checksum: %u\n", checksum);
    
    free(int_array);
    free(float_array);
    
    return 0;
}
