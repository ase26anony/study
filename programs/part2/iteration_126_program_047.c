/* cache_detection.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* Function optimized for different microarchitectures */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(int* data, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 64) {
        sum += data[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(int* data, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 128) {
        sum += data[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_tuned_function(int* data, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 256) {
        sum += data[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(int* data, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 512) {
        sum += data[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading (mirrors driver logic) */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor_bytes[16];
    int byte_index = 0;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2), "c"(0)
    );
    
    /* Extract descriptor bytes from registers */
    memcpy(&descriptor_bytes[0], &eax, 4);
    memcpy(&descriptor_bytes[4], &ebx, 4);
    memcpy(&descriptor_bytes[8], &ecx, 4);
    memcpy(&descriptor_bytes[12], &edx, 4);
    
    /* Process descriptor bytes (mirroring driver-i386.cc switch) */
    for (int i = 0; i < 16; i++) {
        uint8_t desc = descriptor_bytes[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || (desc & 0x80)) continue;
        
        /* Manual switch over cache descriptor bytes */
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
                /* xeon_mp check omitted for simplicity */
                l2_size_kb = 4096; l2_assoc = 16; l2_line = 64;
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
                /* Unknown descriptor */
                break;
        }
    }
}

/* Cache-sensitive benchmark functions */
void benchmark_l1_cache(void) {
    /* Use volatile to prevent optimization of CPU feature checks */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    
    /* Array sizes matching cache sizes from uncovered block */
    int* array_8k = malloc(8192/4);      /* 8KB */
    int* array_16k = malloc(16384/4);    /* 16KB */
    int* array_32k = malloc(32768/4);    /* 32KB */
    int* array_256k = malloc(262144/4);  /* 256KB */
    int* array_1m = malloc(1048576/4);   /* 1MB */
    int* array_2m = malloc(2097152/4);   /* 2MB */
    
    if (!array_8k || !array_16k || !array_32k || 
        !array_256k || !array_1m || !array_2m) {
        goto cleanup;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 8192/4; i++) array_8k[i] = i;
    for (int i = 0; i < 16384/4; i++) array_16k[i] = i;
    for (int i = 0; i < 32768/4; i++) array_32k[i] = i;
    for (int i = 0; i < 262144/4; i++) array_256k[i] = i;
    for (int i = 0; i < 1048576/4; i++) array_1m[i] = i;
    for (int i = 0; i < 2097152/4; i++) array_2m[i] = i;
    
    /* Complex control flow based on CPU features */
    volatile int checksum = 0;
    
    if (has_sse2) {
        generic_tuned_function(array_8k, 8192/4);
        checksum += array_8k[0];
        goto sse2_block;
    } else {
        goto legacy_block;
    }
    
sse2_block:
    if (has_avx) {
        haswell_tuned_function(array_256k, 262144/4);
        checksum += array_256k[100];
        goto avx_block;
    } else {
        goto sse_only_block;
    }
    
avx_block:
    if (is_core2) {
        core2_tuned_function(array_1m, 1048576/4);
        checksum += array_1m[500];
    } else if (is_nehalem) {
        nehalem_tuned_function(array_2m, 2097152/4);
        checksum += array_2m[1000];
    }
    goto final_block;
    
sse_only_block:
    generic_tuned_function(array_16k, 16384/4);
    checksum += array_16k[50];
    goto final_block;
    
legacy_block:
    generic_tuned_function(array_32k, 32768/4);
    checksum += array_32k[25];
    /* fall through */
    
final_block:
    /* Use all arrays to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        checksum += array_8k[i % (8192/4)];
        checksum += array_16k[i % (16384/4)];
        checksum += array_32k[i % (32768/4)];
        checksum += array_256k[i % (262144/4)];
        checksum += array_1m[i % (1048576/4)];
        checksum += array_2m[i % (2097152/4)];
    }
    
    printf("Benchmark checksum: %d\n", checksum);
    
cleanup:
    free(array_8k);
    free(array_16k);
    free(array_32k);
    free(array_256k);
    free(array_1m);
    free(array_2m);
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    printf("CPU Detection Results:\n");
    printf("  SSE2: %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("  AVX: %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("  AVX2: %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    printf("  Core2: %s\n", __builtin_cpu_is("core2") ? "yes" : "no");
    printf("  Nehalem: %s\n", __builtin_cpu_is("nehalem") ? "yes" : "no");
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    printf("\nDetected Cache Parameters:\n");
    printf("  L1: %uKB, %u-way, %u-byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %uKB, %u-way, %u-byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    
    /* Run cache-sensitive benchmarks */
    benchmark_l1_cache();
    
    /* Additional architecture-specific code paths */
    volatile int use_sse_path = __builtin_cpu_supports("sse");
    volatile int use_avx_path = __builtin_cpu_supports("avx");
    volatile int use_avx2_path = __builtin_cpu_supports("avx2");
    
    /* Force multiple optimization paths */
    if (use_sse_path) {
        int* data = malloc(32768);
        if (data) {
            generic_tuned_function(data, 32768/4);
            free(data);
        }
    }
    
    if (use_avx_path) {
        int* data = malloc(131072);
        if (data) {
            haswell_tuned_function(data, 131072/4);
            free(data);
        }
    }
    
    if (use_avx2_path) {
        int* data = malloc(524288);
        if (data) {
            haswell_tuned_function(data, 524288/4);
            free(data);
        }
    }
    
    return 0;
}
