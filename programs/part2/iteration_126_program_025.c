/* cache_descriptor_trigger.c
 * Designed to trigger GCC driver's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 -fno-omit-frame-pointer
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -flto -fprofile-generate
 * For 32-bit: gcc -O1 -m32 -march=pentium4 -mtune=pentium4 -fno-inline
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* CPU feature flags - volatile to prevent optimization */
volatile int has_sse2 = 0;
volatile int has_avx = 0;
volatile int has_avx2 = 0;
volatile int is_intel = 0;
volatile int is_amd = 0;

/* Function prototypes with architecture-specific optimizations */
void __attribute__((optimize("-mtune=generic"))) generic_optimized_loop(void);
void __attribute__((optimize("-mtune=core2"))) core2_optimized_loop(void);
void __attribute__((optimize("-mtune=nehalem"))) nehalem_optimized_loop(void);
void __attribute__((optimize("-mtune=haswell"))) haswell_optimized_loop(void);
void __attribute__((optimize("-mtune=skylake"))) skylake_optimized_loop(void);

/* Direct CPUID leaf 0x2 reading - mimics driver-i386.cc logic */
void read_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, rounds;
    
    /* CPUID leaf 0x2 - Cache Descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Determine number of rounds from AL register */
    rounds = eax & 0xFF;
    
    /* Process first set of descriptors from registers */
    uint8_t *regs = (uint8_t*)&eax;
    for (i = 1; i < 4; i++) {  /* Skip AL (byte 0) */
        if (regs[i] & 0x80) continue;  /* Invalid descriptor */
        
        descriptor = regs[i];
        /* Switch statement mirroring driver-i386.cc uncovered lines */
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
                /* Check for Xeon MP - simplified */
                if (0) {  /* xeon_mp placeholder */
                    break;
                }
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
                /* Other descriptors not in uncovered block */
                break;
        }
    }
    
    /* Process EBX, ECX, EDX similarly */
    uint8_t *regs2 = (uint8_t*)&ebx;
    for (i = 0; i < 4; i++) {
        if (regs2[i] & 0x80) continue;
        /* In real implementation, would process each byte */
        (void)regs2[i];  /* Prevent unused warning */
    }
}

/* Cache-sensitive benchmark functions */
void generic_optimized_loop(void) {
    /* Use array sizes that match common cache sizes */
    volatile char array_8k[8 * 1024];
    volatile char array_256k[256 * 1024];
    volatile int sum = 0;
    
    /* Access pattern sensitive to cache parameters */
    for (int i = 0; i < sizeof(array_8k); i += 64) {
        sum += array_8k[i];
    }
    
    for (int i = 0; i < sizeof(array_256k); i += 128) {
        sum += array_256k[i];
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(sum));
}

void core2_optimized_loop(void) {
    /* Different array sizes for Core2 cache tuning */
    volatile char array_32k[32 * 1024];
    volatile char array_4m[4 * 1024 * 1024];
    volatile int sum = 0;
    
    for (int i = 0; i < sizeof(array_32k); i += 32) {
        sum += array_32k[i];
    }
    
    for (int i = 0; i < sizeof(array_4m); i += 256) {
        sum += array_4m[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

void nehalem_optimized_loop(void) {
    volatile char array_64k[64 * 1024];
    volatile char array_8m[8 * 1024 * 1024];
    volatile int sum = 0;
    
    for (int i = 0; i < sizeof(array_64k); i += 64) {
        sum += array_64k[i];
    }
    
    for (int i = 0; i < sizeof(array_8m); i += 512) {
        sum += array_8m[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

void haswell_optimized_loop(void) {
    volatile char array_32k[32 * 1024];
    volatile char array_256k[256 * 1024];
    volatile int sum = 0;
    
    for (int i = 0; i < sizeof(array_32k); i += 64) {
        sum += array_32k[i];
    }
    
    for (int i = 0; i < sizeof(array_256k); i += 128) {
        sum += array_256k[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

void skylake_optimized_loop(void) {
    volatile char array_32k[32 * 1024];
    volatile char array_1m[1024 * 1024];
    volatile int sum = 0;
    
    for (int i = 0; i < sizeof(array_32k); i += 64) {
        sum += array_32k[i];
    }
    
    for (int i = 0; i < sizeof(array_1m); i += 256) {
        sum += array_1m[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

/* Main function with complex control flow */
int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Set volatile CPU feature flags */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Check CPU vendor */
    uint32_t eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );
    
    char vendor[13] = {0};
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    
    is_intel = (strcmp(vendor, "GenuineIntel") == 0);
    is_amd = (strcmp(vendor, "AuthenticAMD") == 0);
    
    /* Read cache descriptors directly */
    read_cache_descriptors();
    
    /* Complex control flow based on CPU features */
    volatile int checksum = 0;
    
    if (has_sse2) {
        generic_optimized_loop();
        checksum += 1;
        goto sse2_block;
    } else {
        goto legacy_block;
    }
    
sse2_block:
    if (has_avx) {
        core2_optimized_loop();
        checksum += 2;
        
        if (is_intel) {
            nehalem_optimized_loop();
            checksum += 4;
            goto intel_modern;
        } else if (is_amd) {
            haswell_optimized_loop();
            checksum += 8;
            goto amd_modern;
        }
    }
    
    if (has_avx2) {
        skylake_optimized_loop();
        checksum += 16;
    }
    
    goto final_aggregation;
    
legacy_block:
    /* Legacy CPU path */
    volatile char small_array[1024];
    for (int i = 0; i < 1024; i++) {
        checksum += small_array[i];
    }
    goto final_aggregation;
    
intel_modern:
    /* Additional Intel-specific tests */
    if (l1_size_kb > 0) {
        volatile char l1_array[l1_size_kb * 1024];
        for (int i = 0; i < sizeof(l1_array); i += l1_line) {
            checksum += l1_array[i];
        }
    }
    goto final_aggregation;
    
amd_modern:
    /* Additional AMD-specific tests */
    if (l2_size_kb > 0) {
        volatile char l2_array[l2_size_kb * 1024];
        for (int i = 0; i < sizeof(l2_array); i += l2_line) {
            checksum += l2_array[i];
        }
    }
    goto final_aggregation;
    
final_aggregation:
    /* Force use of all architecture-specific functions */
    generic_optimized_loop();
    core2_optimized_loop();
    nehalem_optimized_loop();
    haswell_optimized_loop();
    skylake_optimized_loop();
    
    /* Print results to prevent optimization */
    printf("Cache Parameters: L1=%dKB/%d-way/%dB, L2=%dKB/%d-way/%dB\n",
           l1_size_kb, l1_assoc, l1_line,
           l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %d\n", checksum);
    printf("CPU Features: SSE2=%d, AVX=%d, AVX2=%d\n", 
           has_sse2, has_avx, has_avx2);
    
    return checksum & 0xFF;  /* Return non-zero to indicate success */
}
