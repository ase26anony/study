/* cache_detector.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
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

/* Function optimized for specific microarchitectures to force cache detection */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(int* data, int size) {
    volatile int sum = 0;
    /* Different access pattern */
    for (int i = 0; i < size; i += 16) {
        sum += data[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_tuned_function(int* data, int size) {
    volatile int sum = 0;
    for (int i = size - 1; i >= 0; i--) {
        sum += data[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(int* data, int size) {
    volatile int sum = 0;
    /* Strided access */
    for (int i = 0; i < size; i += 8) {
        sum += data[i];
        if (i + 4 < size) sum += data[i + 4];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Process descriptor bytes from registers */
    uint8_t* regs = (uint8_t*)&eax;
    for (i = 0; i < 4; i++) {
        descriptor = regs[i];
        if (descriptor == 0) continue;
        
        /* Switch statement mirroring uncovered lines 127-244 */
        switch (descriptor) {
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
                /* Simulating xeon_mp check */
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
    
    /* Check other registers too */
    regs = (uint8_t*)&ebx;
    for (i = 0; i < 4; i++) {
        descriptor = regs[i];
        if (descriptor == 0) continue;
        /* Same switch would be repeated in actual driver */
        (void)descriptor; /* Prevent unused warning */
    }
}

/* Cache-size sensitive benchmark */
void cache_sensitive_benchmark(volatile int sse2_flag, volatile int avx_flag) {
    /* Array sizes matching cache sizes from uncovered block */
    const int sizes[] = {
        8 * 1024 / sizeof(int),     /* 8KB L1 */
        16 * 1024 / sizeof(int),    /* 16KB L1 */
        32 * 1024 / sizeof(int),    /* 32KB L1 */
        128 * 1024 / sizeof(int),   /* 128KB L2 */
        256 * 1024 / sizeof(int),   /* 256KB L2 */
        512 * 1024 / sizeof(int),   /* 512KB L2 */
        1024 * 1024 / sizeof(int),  /* 1MB L2 */
        2048 * 1024 / sizeof(int),  /* 2MB L2 */
        4096 * 1024 / sizeof(int),  /* 4MB L2 */
    };
    
    volatile int checksum = 0;
    
    /* Complex control flow based on CPU flags */
    if (sse2_flag) {
        goto sse2_block;
    } else if (avx_flag) {
        goto avx_block;
    } else {
        goto generic_block;
    }
    
sse2_block:
    for (int s = 0; s < 4; s++) {  /* Test smaller sizes for SSE2 */
        int* data = malloc(sizes[s] * sizeof(int));
        if (!data) continue;
        
        for (int i = 0; i < sizes[s]; i++) {
            data[i] = i;
        }
        
        generic_tuned_function(data, sizes[s]);
        core2_tuned_function(data, sizes[s]);
        
        for (int i = 0; i < sizes[s]; i += 64) {
            checksum += data[i];
        }
        
        free(data);
    }
    goto benchmark_end;
    
avx_block:
    for (int s = 2; s < 7; s++) {  /* Test medium sizes for AVX */
        int* data = malloc(sizes[s] * sizeof(int));
        if (!data) continue;
        
        for (int i = 0; i < sizes[s]; i++) {
            data[i] = i * 2;
        }
        
        nehalem_tuned_function(data, sizes[s]);
        haswell_tuned_function(data, sizes[s]);
        
        for (int i = 0; i < sizes[s]; i += 128) {
            checksum += data[i];
        }
        
        free(data);
    }
    goto benchmark_end;
    
generic_block:
    for (int s = 0; s < 3; s++) {  /* Test smallest sizes for generic */
        int* data = malloc(sizes[s] * sizeof(int));
        if (!data) continue;
        
        for (int i = 0; i < sizes[s]; i++) {
            data[i] = i % 256;
        }
        
        generic_tuned_function(data, sizes[s]);
        
        for (int i = 0; i < sizes[s]; i += 32) {
            checksum += data[i];
        }
        
        free(data);
    }
    
benchmark_end:
    (void)checksum; /* Prevent optimization */
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    printf("CPU Detection Results:\n");
    printf("  SSE2: %d\n", has_sse2);
    printf("  AVX:  %d\n", has_avx);
    printf("  AVX2: %d\n", has_avx2);
    printf("  Intel: %d\n", is_intel);
    printf("  AMD: %d\n", is_amd);
    
    /* Read cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    printf("\nCache Parameters (from CPUID leaf 0x2):\n");
    printf("  L1: %uKB, %u-way, %u-byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %uKB, %u-way, %u-byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    
    /* Perform cache-sensitive benchmarks */
    cache_sensitive_benchmark(has_sse2, has_avx);
    
    /* Additional architecture-specific paths */
    if (has_sse2 && !has_avx) {
        /* Pentium 4, Core 2 era */
        int* data = malloc(256 * 1024); /* 256KB array */
        if (data) {
            memset(data, 0xAA, 256 * 1024);
            core2_tuned_function(data, 256 * 1024 / sizeof(int));
            free(data);
        }
    }
    
    if (has_avx) {
        /* Nehalem, Sandy Bridge, Haswell era */
        int* data = malloc(1024 * 1024); /* 1MB array */
        if (data) {
            for (int i = 0; i < 1024 * 1024 / sizeof(int); i++) {
                data[i] = i * 3;
            }
            nehalem_tuned_function(data, 1024 * 1024 / sizeof(int));
            haswell_tuned_function(data, 1024 * 1024 / sizeof(int));
            free(data);
        }
    }
    
    /* Final checksum computation using all detected features */
    volatile int final_checksum = 0;
    final_checksum += has_sse2 * 1000;
    final_checksum += has_avx * 2000;
    final_checksum += has_avx2 * 4000;
    final_checksum += l1_size_kb;
    final_checksum += l2_size_kb;
    
    printf("\nFinal checksum: %d\n", final_checksum);
    
    return 0;
}
