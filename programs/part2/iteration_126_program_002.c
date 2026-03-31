/* cache_descriptor_trigger.c
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

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        data[i] = sum;
    }
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(int* data, int size) {
    volatile int prod = 1;
    for (int i = 0; i < size; i += 8) {
        prod *= data[i] + 1;
        data[i] = prod;
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(int* data, int size) {
    volatile int diff = 0;
    for (int i = size - 1; i >= 0; i -= 4) {
        diff -= data[i];
        data[i] = diff;
    }
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
        if (regs[i] & 0x80) continue; /* Invalid descriptor */
        
        descriptor = regs[i];
        
        /* Switch statement mirroring driver-i386.cc uncovered block */
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
                /* Other descriptors not in uncovered block */
                break;
        }
    }
    
    /* Process ebx, ecx, edx similarly */
    regs = (uint8_t*)&ebx;
    for (i = 0; i < 4; i++) {
        if (regs[i] & 0x80) continue;
        /* Could add more switch cases here */
    }
}

/* Cache-sensitive benchmark functions */
volatile int benchmark_8kb(void) {
    /* 8KB array (2048 ints) - matches case 0x0a, 0x66 */
    int array8k[2048];
    volatile int result = 0;
    
    for (int i = 0; i < 2048; i++) {
        array8k[i] = i;
        result += array8k[i];
    }
    return result;
}

volatile int benchmark_256kb(void) {
    /* 256KB array (65536 ints) - matches case 0x21, 0x3c */
    static int array256k[65536];
    volatile int result = 0;
    
    for (int i = 0; i < 65536; i += 64) {
        array256k[i] = i;
        result += array256k[i];
    }
    return result;
}

volatile int benchmark_1024kb(void) {
    /* 1024KB array (262144 ints) - matches case 0x24, 0x78, 0x7c */
    static int array1024k[262144];
    volatile int result = 0;
    
    for (int i = 0; i < 262144; i += 128) {
        array1024k[i] = i;
        result += array1024k[i];
    }
    return result;
}

int main(void) {
    volatile unsigned long long checksum = 0;
    
    /* Initialize CPU detection - triggers driver cache detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force CPU feature checks */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    
    /* Control flow with goto to create complex branching */
    if (has_sse2) {
        goto sse2_block;
    } else {
        goto legacy_block;
    }
    
sse2_block:
    checksum += 0x1000;
    
    /* Call architecture-tuned functions */
    int test_array[1024];
    generic_tuned_function(test_array, 1024);
    checksum += test_array[0];
    
    if (is_core2) {
        core2_tuned_function(test_array, 1024);
        checksum += test_array[1];
        goto core2_path;
    }
    
    if (is_haswell) {
        haswell_tuned_function(test_array, 1024);
        checksum += test_array[2];
        goto haswell_path;
    }
    
core2_path:
    checksum += benchmark_8kb();
    checksum += benchmark_256kb();
    goto common_path;
    
haswell_path:
    checksum += benchmark_256kb();
    checksum += benchmark_1024kb();
    goto common_path;
    
legacy_block:
    checksum += 0x2000;
    /* Fall through */
    
common_path:
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Use cache parameters in computation */
    checksum += l1_size_kb;
    checksum += l1_assoc * 256;
    checksum += l1_line;
    checksum += l2_size_kb * 4096ULL;
    checksum += l2_assoc * 65536ULL;
    checksum += l2_line;
    
    /* Additional CPUID-based branching */
    uint32_t eax, ebx, ecx, edx;
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    volatile uint8_t family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
    volatile uint8_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    
    /* Switch on CPU family/model to trigger different optimizations */
    switch (family) {
        case 6: /* Intel Core/Core2/Nehalem/etc */
            switch (model) {
                case 0x0F: /* Pentium 4 */
                    checksum += benchmark_8kb() * 2;
                    break;
                case 0x17: /* Core 2 */
                    checksum += benchmark_256kb() * 3;
                    break;
                case 0x1A: /* Nehalem */
                case 0x1E: /* Nehalem */
                case 0x2E: /* Nehalem EX */
                    checksum += benchmark_1024kb() * 4;
                    break;
                case 0x3C: /* Haswell */
                case 0x3F: /* Haswell EP */
                case 0x45: /* Haswell */
                case 0x46: /* Haswell */
                    checksum += benchmark_1024kb() * 5;
                    break;
            }
            break;
        case 15: /* Pentium 4 */
            checksum += benchmark_8kb();
            break;
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int final_result = (int)(checksum & 0xFFFFFFFF);
    
    printf("Cache detection test complete. Checksum: %llu\n", checksum);
    printf("Detected L1: %uKB, %u-way, %uB line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("Detected L2: %uKB, %u-way, %uB line\n", 
           l2_size_kb, l2_assoc, l2_line);
    
    return final_result % 256;
}
