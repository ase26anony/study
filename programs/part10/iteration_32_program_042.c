/* driver_cache_test.c - Target CPU cache detection for GCC driver coverage */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Helper functions with different target architectures */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* data, int size) {
    for (int i = 0; i < size; i += 8) {
        data[i] = data[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* data, int size) {
    for (int i = 0; i < size; i += 16) {
        data[i] = data[i] * 5 + 11;
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* data, int size) {
    for (int i = 0; i < size; i += 32) {
        data[i] = data[i] * 7 + 13;
    }
}

__attribute__((target("arch=skylake")))
void skylake_optimized_loop(int* data, int size) {
    for (int i = 0; i < size; i += 64) {
        data[i] = data[i] * 11 + 17;
    }
}

/* Function to read CPUID cache descriptors */
static void read_cpuid_cache_info(uint32_t leaf, uint32_t* eax, uint32_t* ebx, 
                                  uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

/* Large array operations with different access patterns */
void sequential_access(int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        arr[i] = sum;
    }
}

void strided_access(int* arr, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        __builtin_prefetch(&arr[i + stride * 4], 0, 3);
        arr[i] = arr[i] * 2 + 1;
    }
}

void random_like_access(int* arr, int size) {
    /* Pseudo-random access pattern */
    unsigned int seed = 123456789;
    for (int i = 0; i < 1000; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        int idx = seed % size;
        arr[idx] = arr[idx] * 3 + 5;
    }
}

int main(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    __attribute__((aligned(64))) static int large_array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int large_array2[512 * 1024];   /* 2MB */
    __attribute__((aligned(64))) static int large_array3[256 * 1024];   /* 1MB */
    
    /* Initialize arrays with pseudo-random data */
    unsigned int lcg = 1;
    for (int i = 0; i < 1024 * 1024; i++) {
        lcg = lcg * 1103515245 + 12345;
        if (i < 1024 * 1024) large_array1[i] = lcg & 0xFFF;
        if (i < 512 * 1024) large_array2[i] = (lcg >> 12) & 0xFFF;
        if (i < 256 * 1024) large_array3[i] = (lcg >> 24) & 0xFFF;
    }
    
    /* Conditional compilation based on CPU features */
#ifdef __SSE2__
    if (__builtin_cpu_supports("sse2")) {
        checksum += 0x1000;
        sequential_access(large_array1, 1024 * 1024);
    }
#endif
    
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        checksum += 0x2000;
        strided_access(large_array2, 512 * 1024, 16);
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        checksum += 0x4000;
        strided_access(large_array3, 256 * 1024, 32);
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        checksum += 0x8000;
        random_like_access(large_array1, 1024 * 1024);
    }
#endif
    
    /* Call architecture-specific functions */
    core2_optimized_loop(large_array1, 1024 * 1024);
    nehalem_optimized_loop(large_array2, 512 * 1024);
    sandybridge_optimized_loop(large_array3, 256 * 1024);
    skylake_optimized_loop(large_array1, 1024 * 1024);
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    
    /* Read CPUID leaf 2 (cache descriptors) */
    read_cpuid_cache_info(2, &eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Read CPUID leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 4; i++) {
        eax = 4;
        ecx = i;
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
        checksum += eax + ebx + ecx + edx;
    }
    
    /* Matrix multiplication to suggest cache-aware optimization */
    {
        #define MATRIX_SIZE 256
        __attribute__((aligned(64))) static int matrix_a[MATRIX_SIZE][MATRIX_SIZE];
        __attribute__((aligned(64))) static int matrix_b[MATRIX_SIZE][MATRIX_SIZE];
        __attribute__((aligned(64))) static int matrix_c[MATRIX_SIZE][MATRIX_SIZE];
        
        /* Initialize matrices */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_a[i][j] = (i + j) & 0xFF;
                matrix_b[i][j] = (i - j) & 0xFF;
            }
        }
        
        /* Blocked matrix multiplication (cache-friendly) */
        int block_size = 32;
        for (int i0 = 0; i0 < MATRIX_SIZE; i0 += block_size) {
            for (int j0 = 0; j0 < MATRIX_SIZE; j0 += block_size) {
                for (int k0 = 0; k0 < MATRIX_SIZE; k0 += block_size) {
                    for (int i = i0; i < i0 + block_size && i < MATRIX_SIZE; i++) {
                        for (int j = j0; j < j0 + block_size && j < MATRIX_SIZE; j++) {
                            int sum = matrix_c[i][j];
                            for (int k = k0; k < k0 + block_size && k < MATRIX_SIZE; k++) {
                                sum += matrix_a[i][k] * matrix_b[k][j];
                            }
                            matrix_c[i][j] = sum;
                        }
                    }
                }
            }
        }
        
        /* Use result to prevent elimination */
        for (int i = 0; i < MATRIX_SIZE; i += 64) {
            checksum += matrix_c[i][i];
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
