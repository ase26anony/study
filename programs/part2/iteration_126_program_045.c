/* cache_descriptor_trigger.c
 * Targets GCC driver-i386.cc cache descriptor decoding (lines 127-244)
 * Compile with: gcc -O2 -march=native -mtune=native -fno-omit-frame-pointer -std=gnu11 -o cache_test cache_descriptor_trigger.c
 * Also try: gcc -O3 -march=core2 -mtune=core2 -flto -fprofile-generate -std=gnu11 -o cache_test_lto cache_descriptor_trigger.c
 * And: gcc -O1 -m32 -march=pentium4 -mtune=pentium4 -fno-inline -std=gnu11 -o cache_test_32 cache_descriptor_trigger.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters matching the uncovered block */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* Volatile CPU feature flags to force runtime detection */
volatile int cpu_sse2 = 0;
volatile int cpu_sse4 = 0;
volatile int cpu_avx = 0;
volatile int cpu_avx2 = 0;
volatile int cpu_xeon_mp = 0; /* Simulating the xeon_mp flag from driver */

/* Function with architecture-specific optimization hints */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(int* data, size_t size) {
    /* Access pattern that might benefit from generic cache tuning */
    for (size_t i = 0; i < size; i += 64) {
        data[i] = i * 3;
    }
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(int* data, size_t size) {
    /* Different stride for Core2 architecture */
    for (size_t i = 0; i < size; i += 32) {
        data[i] = i * 5;
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(int* data, size_t size) {
    /* AVX2-friendly access pattern */
    for (size_t i = 0; i < size; i += 128) {
        data[i] = i * 7;
    }
}

/* Direct CPUID leaf 0x2 reading - mirrors driver logic */
void read_cache_descriptors_direct(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor_bytes[16];
    int bytes_read = 0;
    
    /* CPUID leaf 0x2 - Cache Descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x2)
    );
    
    /* Extract descriptor bytes as per Intel manual */
    descriptor_bytes[bytes_read++] = (eax >> 0) & 0xFF;
    descriptor_bytes[bytes_read++] = (eax >> 8) & 0xFF;
    descriptor_bytes[bytes_read++] = (eax >> 16) & 0xFF;
    descriptor_bytes[bytes_read++] = (eax >> 24) & 0xFF;
    
    descriptor_bytes[bytes_read++] = (ebx >> 0) & 0xFF;
    descriptor_bytes[bytes_read++] = (ebx >> 8) & 0xFF;
    descriptor_bytes[bytes_read++] = (ebx >> 16) & 0xFF;
    descriptor_bytes[bytes_read++] = (ebx >> 24) & 0xFF;
    
    descriptor_bytes[bytes_read++] = (ecx >> 0) & 0xFF;
    descriptor_bytes[bytes_read++] = (ecx >> 8) & 0xFF;
    descriptor_bytes[bytes_read++] = (ecx >> 16) & 0xFF;
    descriptor_bytes[bytes_read++] = (ecx >> 24) & 0xFF;
    
    descriptor_bytes[bytes_read++] = (edx >> 0) & 0xFF;
    descriptor_bytes[bytes_read++] = (edx >> 8) & 0xFF;
    descriptor_bytes[bytes_read++] = (edx >> 16) & 0xFF;
    descriptor_bytes[bytes_read++] = (edx >> 24) & 0xFF;
    
    /* Process descriptor bytes - mirroring the uncovered switch cases */
    for (int i = 0; i < bytes_read; i++) {
        uint8_t desc = descriptor_bytes[i];
        
        /* Skip invalid descriptors (0x00) and CPUID count (0x01) */
        if (desc == 0x00 || desc == 0x01) continue;
        
        /* Direct mapping of uncovered cases */
        switch (desc) {
            case 0x0a:
                l1_size_kb = 8; l1_assoc = 2; l1_line = 32;
                break;
            case 0x0c:
                l1_size_kb = 16; l1_assoc = 4; l1_line = 32;
                break;
            case 0x0d:
                l1_size_kb = 16; l1_assoc = 4; l1_line = 64;
                break;
            case 0x0e:
                l1_size_kb = 24; l1_assoc = 6; l1_line = 64;
                break;
            case 0x21:
                l2_size_kb = 256; l2_assoc = 8; l2_line = 64;
                break;
            case 0x24:
                l2_size_kb = 1024; l2_assoc = 16; l2_line = 64;
                break;
            case 0x2c:
                l1_size_kb = 32; l1_assoc = 8; l1_line = 64;
                break;
            case 0x39:
                l2_size_kb = 128; l2_assoc = 4; l2_line = 64;
                break;
            case 0x3a:
                l2_size_kb = 192; l2_assoc = 6; l2_line = 64;
                break;
            case 0x3b:
                l2_size_kb = 128; l2_assoc = 2; l2_line = 64;
                break;
            case 0x3c:
                l2_size_kb = 256; l2_assoc = 4; l2_line = 64;
                break;
            case 0x3d:
                l2_size_kb = 384; l2_assoc = 6; l2_line = 64;
                break;
            case 0x3e:
                l2_size_kb = 512; l2_assoc = 4; l2_line = 64;
                break;
            case 0x41:
                l2_size_kb = 128; l2_assoc = 4; l2_line = 32;
                break;
            case 0x42:
                l2_size_kb = 256; l2_assoc = 4; l2_line = 32;
                break;
            case 0x43:
                l2_size_kb = 512; l2_assoc = 4; l2_line = 32;
                break;
            case 0x44:
                l2_size_kb = 1024; l2_assoc = 4; l2_line = 32;
                break;
            case 0x45:
                l2_size_kb = 2048; l2_assoc = 4; l2_line = 32;
                break;
            case 0x48:
                l2_size_kb = 3072; l2_assoc = 12; l2_line = 64;
                break;
            case 0x49:
                if (!cpu_xeon_mp) {
                    l2_size_kb = 4096; l2_assoc = 16; l2_line = 64;
                }
                break;
            case 0x4e:
                l2_size_kb = 6144; l2_assoc = 24; l2_line = 64;
                break;
            case 0x60:
                l1_size_kb = 16; l1_assoc = 8; l1_line = 64;
                break;
            case 0x66:
                l1_size_kb = 8; l1_assoc = 4; l1_line = 64;
                break;
            case 0x67:
                l1_size_kb = 16; l1_assoc = 4; l1_line = 64;
                break;
            case 0x68:
                l1_size_kb = 32; l1_assoc = 4; l1_line = 64;
                break;
            case 0x78:
                l2_size_kb = 1024; l2_assoc = 4; l2_line = 64;
                break;
            case 0x79:
                l2_size_kb = 128; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7a:
                l2_size_kb = 256; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7b:
                l2_size_kb = 512; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7c:
                l2_size_kb = 1024; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7d:
                l2_size_kb = 2048; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7f:
                l2_size_kb = 512; l2_assoc = 2; l2_line = 64;
                break;
            case 0x80:
                l2_size_kb = 512; l2_assoc = 8; l2_line = 64;
                break;
            case 0x82:
                l2_size_kb = 256; l2_assoc = 8; l2_line = 32;
                break;
            case 0x83:
                l2_size_kb = 512; l2_assoc = 8; l2_line = 32;
                break;
            case 0x84:
                l2_size_kb = 1024; l2_assoc = 8; l2_line = 32;
                break;
            case 0x85:
                l2_size_kb = 2048; l2_assoc = 8; l2_line = 32;
                break;
            case 0x86:
                l2_size_kb = 512; l2_assoc = 4; l2_line = 64;
                break;
            case 0x87:
                l2_size_kb = 1024; l2_assoc = 8; l2_line = 64;
                break;
            default:
                /* Other descriptors not in uncovered block */
                break;
        }
    }
}

/* Cache-sensitive benchmark functions using sizes from uncovered block */
void benchmark_cache_sizes(void) {
    volatile int result = 0;
    
    /* Array sizes matching cache sizes from uncovered descriptors */
    int* array_8k = malloc(8 * 1024 / sizeof(int));    /* 0x0a, 0x66 */
    int* array_16k = malloc(16 * 1024 / sizeof(int));  /* 0x0c, 0x0d, 0x60, 0x67 */
    int* array_24k = malloc(24 * 1024 / sizeof(int));  /* 0x0e */
    int* array_32k = malloc(32 * 1024 / sizeof(int));  /* 0x2c, 0x68 */
    int* array_128k = malloc(128 * 1024 / sizeof(int)); /* 0x39, 0x3b, 0x41, 0x79 */
    int* array_256k = malloc(256 * 1024 / sizeof(int)); /* 0x21, 0x3c, 0x42, 0x7a, 0x82 */
    int* array_512k = malloc(512 * 1024 / sizeof(int)); /* 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
    int* array_1m = malloc(1024 * 1024 / sizeof(int));  /* 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
    int* array_2m = malloc(2048 * 1024 / sizeof(int));  /* 0x45, 0x7d, 0x85 */
    int* array_3m = malloc(3072 * 1024 / sizeof(int));  /* 0x48 */
    int* array_4m = malloc(4096 * 1024 / sizeof(int));  /* 0x49 */
    int* array_6m = malloc(6144 * 1024 / sizeof(int));  /* 0x4e */
    
    /* Force initialization to prevent optimization */
    if (array_8k) for (int i = 0; i < 8*1024/sizeof(int); i++) array_8k[i] = i;
    if (array_16k) for (int i = 0; i < 16*1024/sizeof(int); i++) array_16k[i] = i;
    if (array_24k) for (int i = 0; i < 24*1024/sizeof(int); i++) array_24k[i] = i;
    if (array_32k) for (int i = 0; i < 32*1024/sizeof(int); i++) array_32k[i] = i;
    if (array_128k) for (int i = 0; i < 128*1024/sizeof(int); i++) array_128k[i] = i;
    if (array_256k) for (int i = 0; i < 256*1024/sizeof(int); i++) array_256k[i] = i;
    if (array_512k) for (int i = 0; i < 512*1024/sizeof(int); i++) array_512k[i] = i;
    if (array_1m) for (int i = 0; i < 1024*1024/sizeof(int); i++) array_1m[i] = i;
    if (array_2m) for (int i = 0; i < 2048*1024/sizeof(int); i++) array_2m[i] = i;
    if (array_3m) for (int i = 0; i < 3072*1024/sizeof(int); i++) array_3m[i] = i;
    if (array_4m) for (int i = 0; i < 4096*1024/sizeof(int); i++) array_4m[i] = i;
    if (array_6m) for (int i = 0; i < 6144*1024/sizeof(int); i++) array_6m[i] = i;
    
    /* Complex control flow based on CPU features */
    if (cpu_sse2) {
        generic_tuned_function(array_8k, 8*1024/sizeof(int));
        goto sse2_block;
    } else if (cpu_sse4) {
        core2_tuned_function(array_16k, 16*1024/sizeof(int));
        goto sse4_block;
    } else if (cpu_avx) {
        haswell_tuned_function(array_256k, 256*1024/sizeof(int));
        goto avx_block;
    } else {
        /* Fallback */
        for (int i = 0; i < 8*1024/sizeof(int); i++) {
            result += array_8k[i];
        }
    }
    
sse2_block:
    for (int i = 0; i < 16*1024/sizeof(int); i += 2) {
        result += array_16k[i];
    }
    if (!cpu_avx2) goto end_benchmark;
    
sse4_block:
    for (int i = 0; i < 256*1024/sizeof(int); i += 4) {
        result += array_256k[i];
    }
    if (!cpu_avx) goto end_benchmark;
    
avx_block:
    for (int i = 0; i < 1024*1024/sizeof(int); i += 8) {
        result += array_1m[i];
    }
    
end_benchmark:
    /* Use result to prevent dead code elimination */
    printf("Benchmark result: %d\n", result);
    
    /* Cleanup */
    free(array_8k); free(array_16k); free(array_24k); free(array_32k);
    free(array_128k); free(array_256k); free(array_512k); free(array_1m);
    free(array_2m); free(array_3m); free(array_4m); free(array_6m);
}

int main(void) {
    /* Initialize GCC's CPU detection - triggers driver cache detection */
    __builtin_cpu_init();
    
    /* Set volatile flags based on CPU features */
    cpu_sse2 = __builtin_cpu_supports("sse2");
    cpu_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_avx = __builtin_cpu_supports("avx");
    cpu_avx2 = __builtin_cpu_supports("avx2");
    
    /* Simulate xeon_mp detection - check for certain CPU models */
    if (__builtin_cpu_is("intel") && __builtin_cpu_supports("pni")) {
        /* Simple heuristic - in real code would check CPU family/model */
        cpu_xeon_mp = 0; /* Adjust based on actual detection */
    }
    
    printf("CPU Features: SSE2=%d, SSE4=%d, AVX=%d, AVX2=%d\n",
           cpu_sse2, cpu_sse4, cpu_avx, cpu_avx2);
    
    /* Call architecture-tuned functions to force driver to process different cache configs */
    int test_array[1024];
    generic_tuned_function(test_array, 1024);
    core2_tuned_function(test_array, 1024);
    haswell_tuned_function(test_array, 1024);
    
    /* Direct CPUID cache descriptor reading */
    read_cache_descriptors_direct();
    
    printf("Detected L1: %uKB, %u-way, %u-byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("Detected L2: %uKB, %u-way, %u-byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    
    /* Run cache-sensitive benchmarks */
    benchmark_cache_sizes();
    
    /* Final checksum computation using all detected parameters */
    unsigned int checksum = 0;
    checksum += l1_size_kb * 31;
    checksum += l1_assoc * 37;
    checksum += l1_line * 41;
    checksum += l2_size_kb * 43;
    checksum += l2_assoc * 47;
    checksum += l2_line * 53;
    checksum += cpu_sse2 * 59;
    checksum += cpu_sse4 * 61;
    checksum += cpu_avx * 67;
    checksum += cpu_avx2 * 71;
    
    printf("Final checksum: %u\n", checksum);
    
    return 0;
}
