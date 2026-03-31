/* driver_cache_test.c - Target uncovered lines in driver-i386.cc cache detection */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Helper functions with different target attributes */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 8) {
        sum += data[i];
    }
    __asm__ volatile("" : : "r"(sum));
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 16) {
        sum += data[i];
    }
    __asm__ volatile("" : : "r"(sum));
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 32) {
        sum += data[i];
    }
    __asm__ volatile("" : : "r"(sum));
}

__attribute__((target("arch=skylake")))
void skylake_optimized_loop(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 64) {
        sum += data[i];
    }
    __asm__ volatile("" : : "r"(sum));
}

/* Function to read CPUID cache descriptors */
static uint32_t read_cpuid_cache_descriptors(uint32_t leaf) {
    uint32_t eax, ebx, ecx, edx;
    
    __asm__ volatile (
        "cpuid\n\t"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(leaf)
    );
    
    return eax; /* Cache descriptor bytes are in eax for leaf 2 */
}

/* Different access patterns to hint cache usage */
void sequential_access(int* arr, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    __asm__ volatile("" : : "r"(sum));
}

void stride_access(int* arr, int size, int stride) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += stride) {
        sum += arr[i];
    }
    __asm__ volatile("" : : "r"(sum));
}

void randomish_access(int* arr, int size) {
    volatile int sum = 0;
    int index = 0;
    for (int i = 0; i < size; i++) {
        index = (index * 1103515245 + 12345) & (size - 1);
        sum += arr[index];
    }
    __asm__ volatile("" : : "r"(sum));
}

/* Main function with CPU detection and various code paths */
int main() {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to exercise cache detection */
    __attribute__((aligned(64))) static int large_array[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int medium_array[256 * 1024]; /* 1MB */
    
    /* Fill arrays with pseudo-random data */
    uint32_t seed = 42;
    for (int i = 0; i < 1024 * 1024; i++) {
        seed = seed * 1103515245 + 12345;
        large_array[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    /* Conditional compilation based on CPU features */
#ifdef __SSE2__
    checksum += 1;
#endif
    
#ifdef __SSE4_2__
    checksum += 2;
#endif
    
#ifdef __AVX__
    checksum += 4;
#endif
    
#ifdef __AVX2__
    checksum += 8;
#endif
    
    /* Execute different code paths based on CPU support */
    if (__builtin_cpu_supports("sse2")) {
        sequential_access(large_array, 1024 * 1024);
        checksum += 0x10;
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        stride_access(large_array, 1024 * 1024, 8);
        checksum += 0x20;
    }
    
    if (__builtin_cpu_supports("avx")) {
        stride_access(large_array, 1024 * 1024, 16);
        checksum += 0x40;
    }
    
    if (__builtin_cpu_supports("avx2")) {
        stride_access(large_array, 1024 * 1024, 32);
        checksum += 0x80;
    }
    
    /* Call target-specific functions */
    core2_optimized_loop(large_array, 1024 * 1024);
    nehalem_optimized_loop(medium_array, 256 * 1024);
    sandybridge_optimized_loop(large_array, 1024 * 1024);
    skylake_optimized_loop(medium_array, 256 * 1024);
    
    /* Read CPUID cache information */
    uint32_t cache_info = 0;
    
    /* Try to read cache descriptors - leaf 2 */
    __asm__ volatile (
        "mov $2, %%eax\n\t"
        "cpuid\n\t"
        : "=a"(cache_info)
        : 
        : "%ebx", "%ecx", "%edx"
    );
    
    checksum += cache_info;
    
    /* Try deterministic cache parameters - leaf 4 */
    for (int i = 0; i < 4; i++) {
        uint32_t eax_val = 4;
        uint32_t ecx_val = i;
        uint32_t eax, ebx, ecx, edx;
        
        __asm__ volatile (
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(eax_val), "c"(ecx_val)
        );
        
        checksum += eax + ebx + ecx + edx;
    }
    
    /* Use prefetch hints */
    for (int i = 0; i < 1024 * 1024; i += 64) {
        __builtin_prefetch(&large_array[i + 64], 0, 3);
        checksum += large_array[i];
    }
    
    /* Matrix multiplication to exercise cache */
    __attribute__((aligned(64))) static int mat_a[256][256];
    __attribute__((aligned(64))) static int mat_b[256][256];
    __attribute__((aligned(64))) static int mat_c[256][256];
    
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            mat_a[i][j] = i + j;
            mat_b[i][j] = i - j;
        }
    }
    
    /* Simple matrix multiply with different access patterns */
    for (int i = 0; i < 256; i++) {
        for (int k = 0; k < 256; k++) {
            for (int j = 0; j < 256; j++) {
                mat_c[i][j] += mat_a[i][k] * mat_b[k][j];
            }
        }
    }
    
    /* Extract some result to prevent optimization */
    for (int i = 0; i < 256; i += 32) {
        checksum += mat_c[i][i];
    }
    
    /* Random access pattern */
    randomish_access(large_array, 1024 * 1024);
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
