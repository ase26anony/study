/* 
 * cache_detection_test.c
 * Designed to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 cache_detection_test.c -o cache_test
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -std=gnu11 cache_detection_test.c -o cache_test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
volatile int l1_size_kb = 0;
volatile int l1_assoc = 0;
volatile int l1_line = 0;
volatile int l2_size_kb = 0;
volatile int l2_assoc = 0;
volatile int l2_line = 0;

/* Volatile CPU feature flags to force runtime evaluation */
volatile int cpu_sse2 = 0;
volatile int cpu_sse4 = 0;
volatile int cpu_avx = 0;
volatile int cpu_avx2 = 0;

/* Function declarations with architecture-specific optimizations */
void __attribute__((optimize("-mtune=generic"))) generic_cache_test(void);
void __attribute__((optimize("-mtune=core2"))) core2_cache_test(void);
void __attribute__((optimize("-mtune=nehalem"))) nehalem_cache_test(void);
void __attribute__((optimize("-mtune=haswell"))) haswell_cache_test(void);
void __attribute__((optimize("-mtune=skylake"))) skylake_cache_test(void);

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor_bytes[16];
    int i, j;
    
    /* Execute CPUID leaf 0x2 to get cache descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x2)
    );
    
    /* Extract descriptor bytes from registers */
    descriptor_bytes[0] = (eax >> 0) & 0xFF;
    descriptor_bytes[1] = (eax >> 8) & 0xFF;
    descriptor_bytes[2] = (eax >> 16) & 0xFF;
    descriptor_bytes[3] = (eax >> 24) & 0xFF;
    descriptor_bytes[4] = (ebx >> 0) & 0xFF;
    descriptor_bytes[5] = (ebx >> 8) & 0xFF;
    descriptor_bytes[6] = (ebx >> 16) & 0xFF;
    descriptor_bytes[7] = (ebx >> 24) & 0xFF;
    descriptor_bytes[8] = (ecx >> 0) & 0xFF;
    descriptor_bytes[9] = (ecx >> 8) & 0xFF;
    descriptor_bytes[10] = (ecx >> 16) & 0xFF;
    descriptor_bytes[11] = (ecx >> 24) & 0xFF;
    descriptor_bytes[12] = (edx >> 0) & 0xFF;
    descriptor_bytes[13] = (edx >> 8) & 0xFF;
    descriptor_bytes[14] = (edx >> 16) & 0xFF;
    descriptor_bytes[15] = (edx >> 24) & 0xFF;
    
    /* Process descriptor bytes - mirroring driver-i386.cc switch logic */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptor_bytes[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0x01) continue;
        
        /* Direct mapping of descriptor values to cache parameters */
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
                /* Skip Xeon MP case */
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
                /* Unknown descriptor - continue */
                break;
        }
    }
}

/* Cache-sensitive memory access patterns */
void cache_sensitive_benchmark(int cache_size_kb) {
    volatile int sum = 0;
    int i, j;
    
    /* Allocate array sized to specific cache thresholds */
    int elements = (cache_size_kb * 1024) / sizeof(int);
    if (elements <= 0) elements = 1024;
    
    int *array = (int*)malloc(elements * sizeof(int));
    if (!array) return;
    
    /* Initialize array */
    for (i = 0; i < elements; i++) {
        array[i] = i;
    }
    
    /* Access pattern that benefits from cache awareness */
    for (j = 0; j < 100; j++) {
        for (i = 0; i < elements; i += 64) { /* 64-byte stride */
            sum += array[i];
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(sum));
    
    free(array);
}

/* Architecture-specific test functions */
void __attribute__((optimize("-mtune=generic"))) generic_cache_test(void) {
    /* Generic cache test - uses 8KB L1 as baseline */
    cache_sensitive_benchmark(8);
}

void __attribute__((optimize("-mtune=core2"))) core2_cache_test(void) {
    /* Core 2 typically has 32KB L1, 2-4MB L2 */
    cache_sensitive_benchmark(32);
    cache_sensitive_benchmark(2048);
}

void __attribute__((optimize("-mtune=nehalem"))) nehalem_cache_test(void) {
    /* Nehalem: 32KB L1, 256KB L2 per core */
    cache_sensitive_benchmark(32);
    cache_sensitive_benchmark(256);
}

void __attribute__((optimize("-mtune=haswell"))) haswell_cache_test(void) {
    /* Haswell: 32KB L1, 256KB L2 */
    cache_sensitive_benchmark(32);
    cache_sensitive_benchmark(256);
}

void __attribute__((optimize("-mtune=skylake"))) skylake_cache_test(void) {
    /* Skylake: 32KB L1, 256KB L2 */
    cache_sensitive_benchmark(32);
    cache_sensitive_benchmark(256);
}

int main(void) {
    volatile int checksum = 0;
    volatile int cpu_type = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Set volatile CPU feature flags */
    cpu_sse2 = __builtin_cpu_supports("sse2");
    cpu_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_avx = __builtin_cpu_supports("avx");
    cpu_avx2 = __builtin_cpu_supports("avx2");
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Complex control flow based on CPU features */
    if (cpu_sse2) {
        checksum += 1;
        generic_cache_test();
        
        if (cpu_sse4) {
            checksum += 2;
            core2_cache_test();
            
            if (cpu_avx) {
                checksum += 4;
                nehalem_cache_test();
                
                if (cpu_avx2) {
                    checksum += 8;
                    haswell_cache_test();
                    skylake_cache_test();
                }
            }
        }
    }
    
    /* Additional cache tests based on detected parameters */
    if (l1_size_kb > 0) {
        cache_sensitive_benchmark(l1_size_kb);
        checksum += l1_size_kb;
    }
    
    if (l2_size_kb > 0) {
        cache_sensitive_benchmark(l2_size_kb);
        checksum += l2_size_kb;
    }
    
    /* Test all cache sizes from the uncovered block */
    int test_sizes[] = {8, 16, 24, 32, 128, 192, 256, 384, 512, 1024, 2048, 3072, 4096, 6144};
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        cache_sensitive_benchmark(test_sizes[i]);
        checksum += test_sizes[i];
    }
    
    /* Use goto to create complex control flow */
    volatile int selector = checksum % 5;
    
    switch (selector) {
        case 0:
            goto case0;
        case 1:
            goto case1;
        case 2:
            goto case2;
        case 3:
            goto case3;
        default:
            goto case4;
    }
    
case0:
    cache_sensitive_benchmark(8);  /* 0x0a */
    checksum += 0x0a;
    goto end;
    
case1:
    cache_sensitive_benchmark(16); /* 0x0c, 0x0d */
    checksum += 0x0c;
    goto end;
    
case2:
    cache_sensitive_benchmark(32); /* 0x2c */
    checksum += 0x2c;
    goto end;
    
case3:
    cache_sensitive_benchmark(256); /* 0x21 */
    checksum += 0x21;
    goto end;
    
case4:
    cache_sensitive_benchmark(1024); /* 0x24 */
    checksum += 0x24;
    goto end;
    
end:
    /* Print results to prevent optimization */
    printf("Cache detection test complete.\n");
    printf("L1: %dKB, %d-way, %dB line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2: %dKB, %d-way, %dB line\n", l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF; /* Return non-zero to indicate success */
}
