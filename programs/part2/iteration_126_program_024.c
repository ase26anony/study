/* cache_descriptor_trigger.c
 * Designed to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 -fno-omit-frame-pointer
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -std=gnu11
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* Global variables matching the cache parameter structure */
volatile unsigned int level1_sizekb = 0;
volatile unsigned int level1_assoc = 0;
volatile unsigned int level1_line = 0;
volatile unsigned int level2_sizekb = 0;
volatile unsigned int level2_assoc = 0;
volatile unsigned int level2_line = 0;

/* Volatile CPU feature flags to force runtime evaluation */
volatile int cpu_sse2 = 0;
volatile int cpu_sse4 = 0;
volatile int cpu_avx = 0;
volatile int cpu_avx2 = 0;
volatile int cpu_xeon_mp = 0;

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache (8KB-32KB) */
    char buffer1[32 * 1024];
    for (int i = 0; i < sizeof(buffer1); i += 64) {
        buffer1[i] = i & 0xFF;
        sum += buffer1[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L2 cache (256KB-4MB) */
    char buffer2[256 * 1024];
    for (int i = 0; i < sizeof(buffer2); i += 128) {
        buffer2[i] = i & 0xFF;
        sum += buffer2[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to larger L2 cache (512KB-2MB) */
    char buffer3[1024 * 1024];
    for (int i = 0; i < sizeof(buffer3); i += 256) {
        buffer3[i] = i & 0xFF;
        sum += buffer3[i];
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
        : "a"(2), "c"(0)
    );
    
    /* Process descriptor bytes from all registers */
    uint8_t *regs = (uint8_t*)&eax;
    
    for (i = 0; i < 4; i++) {
        descriptor = regs[i];
        if (descriptor == 0) continue;
        
        /* Switch statement mirroring uncovered block */
        switch (descriptor) {
            case 0x0a:
                level1_sizekb = 8; level1_assoc = 2; level1_line = 32;
                break;
            case 0x0c:
                level1_sizekb = 16; level1_assoc = 4; level1_line = 32;
                break;
            case 0x0d:
                level1_sizekb = 16; level1_assoc = 4; level1_line = 64;
                break;
            case 0x0e:
                level1_sizekb = 24; level1_assoc = 6; level1_line = 64;
                break;
            case 0x21:
                level2_sizekb = 256; level2_assoc = 8; level2_line = 64;
                break;
            case 0x24:
                level2_sizekb = 1024; level2_assoc = 16; level2_line = 64;
                break;
            case 0x2c:
                level1_sizekb = 32; level1_assoc = 8; level1_line = 64;
                break;
            case 0x39:
                level2_sizekb = 128; level2_assoc = 4; level2_line = 64;
                break;
            case 0x3a:
                level2_sizekb = 192; level2_assoc = 6; level2_line = 64;
                break;
            case 0x3b:
                level2_sizekb = 128; level2_assoc = 2; level2_line = 64;
                break;
            case 0x3c:
                level2_sizekb = 256; level2_assoc = 4; level2_line = 64;
                break;
            case 0x3d:
                level2_sizekb = 384; level2_assoc = 6; level2_line = 64;
                break;
            case 0x3e:
                level2_sizekb = 512; level2_assoc = 4; level2_line = 64;
                break;
            case 0x41:
                level2_sizekb = 128; level2_assoc = 4; level2_line = 32;
                break;
            case 0x42:
                level2_sizekb = 256; level2_assoc = 4; level2_line = 32;
                break;
            case 0x43:
                level2_sizekb = 512; level2_assoc = 4; level2_line = 32;
                break;
            case 0x44:
                level2_sizekb = 1024; level2_assoc = 4; level2_line = 32;
                break;
            case 0x45:
                level2_sizekb = 2048; level2_assoc = 4; level2_line = 32;
                break;
            case 0x48:
                level2_sizekb = 3072; level2_assoc = 12; level2_line = 64;
                break;
            case 0x49:
                if (cpu_xeon_mp) break;
                level2_sizekb = 4096; level2_assoc = 16; level2_line = 64;
                break;
            case 0x4e:
                level2_sizekb = 6144; level2_assoc = 24; level2_line = 64;
                break;
            case 0x60:
                level1_sizekb = 16; level1_assoc = 8; level1_line = 64;
                break;
            case 0x66:
                level1_sizekb = 8; level1_assoc = 4; level1_line = 64;
                break;
            case 0x67:
                level1_sizekb = 16; level1_assoc = 4; level1_line = 64;
                break;
            case 0x68:
                level1_sizekb = 32; level1_assoc = 4; level1_line = 64;
                break;
            case 0x78:
                level2_sizekb = 1024; level2_assoc = 4; level2_line = 64;
                break;
            case 0x79:
                level2_sizekb = 128; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7a:
                level2_sizekb = 256; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7b:
                level2_sizekb = 512; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7c:
                level2_sizekb = 1024; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7d:
                level2_sizekb = 2048; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7f:
                level2_sizekb = 512; level2_assoc = 2; level2_line = 64;
                break;
            case 0x80:
                level2_sizekb = 512; level2_assoc = 8; level2_line = 64;
                break;
            case 0x82:
                level2_sizekb = 256; level2_assoc = 8; level2_line = 32;
                break;
            case 0x83:
                level2_sizekb = 512; level2_assoc = 8; level2_line = 32;
                break;
            case 0x84:
                level2_sizekb = 1024; level2_assoc = 8; level2_line = 32;
                break;
            case 0x85:
                level2_sizekb = 2048; level2_assoc = 8; level2_line = 32;
                break;
            case 0x86:
                level2_sizekb = 512; level2_assoc = 4; level2_line = 64;
                break;
            case 0x87:
                level2_sizekb = 1024; level2_assoc = 8; level2_line = 64;
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
        /* Same switch could be used here */
    }
}

/* Cache-sensitive benchmark based on detected parameters */
volatile int cache_sensitive_benchmark(void) {
    volatile int result = 0;
    int i, j;
    
    /* Use volatile flags to force conditional compilation paths */
    if (cpu_sse2) {
        /* L1 cache sized array */
        char l1_buffer[32 * 1024];  /* 32KB max from 0x2c */
        for (i = 0; i < sizeof(l1_buffer); i += level1_line ? level1_line : 64) {
            l1_buffer[i] = i & 0xFF;
            result += l1_buffer[i];
        }
        goto benchmark_l2;
    } else if (cpu_avx) {
        char l1_buffer[16 * 1024];  /* 16KB from 0x0c/0x0d */
        for (i = 0; i < sizeof(l1_buffer); i += level1_line ? level1_line : 32) {
            l1_buffer[i] = i & 0xFF;
            result += l1_buffer[i];
        }
        goto benchmark_l2;
    } else {
        char l1_buffer[8 * 1024];   /* 8KB from 0x0a/0x66 */
        for (i = 0; i < sizeof(l1_buffer); i += level1_line ? level1_line : 32) {
            l1_buffer[i] = i & 0xFF;
            result += l1_buffer[i];
        }
    }
    
benchmark_l2:
    if (cpu_sse4 || cpu_avx2) {
        /* L2 cache sized array */
        char l2_buffer[1024 * 1024];  /* 1MB from 0x24/0x78 */
        for (i = 0; i < sizeof(l2_buffer); i += level2_line ? level2_line : 64) {
            l2_buffer[i] = i & 0xFF;
            result += l2_buffer[i];
        }
    } else if (cpu_sse2) {
        char l2_buffer[256 * 1024];   /* 256KB from 0x21 */
        for (i = 0; i < sizeof(l2_buffer); i += level2_line ? level2_line : 64) {
            l2_buffer[i] = i & 0xFF;
            result += l2_buffer[i];
        }
    }
    
    return result;
}

int main(void) {
    volatile int final_result = 0;
    
    /* Initialize CPU detection - triggers GCC's internal cache detection */
    __builtin_cpu_init();
    
    /* Set volatile flags based on CPU features */
    cpu_sse2 = __builtin_cpu_supports("sse2");
    cpu_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_avx = __builtin_cpu_supports("avx");
    cpu_avx2 = __builtin_cpu_supports("avx2");
    
    /* Check for Xeon MP-like characteristics */
    {
        uint32_t eax, ebx, ecx, edx;
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(1), "c"(0)
        );
        /* Simple heuristic for multi-processor capable */
        cpu_xeon_mp = ((edx >> 28) & 1);  /* HTT bit */
    }
    
    /* Call architecture-specific functions to trigger different -mtune paths */
    generic_cache_test();
    core2_cache_test();
    haswell_cache_test();
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Perform cache-sensitive operations based on detected parameters */
    final_result = cache_sensitive_benchmark();
    
    /* Additional branching based on CPU model strings */
    if (__builtin_cpu_is("intel")) {
        if (__builtin_cpu_is("core2")) {
            /* Trigger cases common for Core 2: 0x21, 0x24, 0x39, etc. */
            volatile int core2_sum = 0;
            char core2_buffer[2048 * 1024];  /* 2MB */
            for (int i = 0; i < sizeof(core2_buffer); i += 128) {
                core2_buffer[i] = i & 0xFF;
                core2_sum += core2_buffer[i];
            }
            final_result += core2_sum;
        } else if (__builtin_cpu_is("nehalem")) {
            /* Trigger cases for Nehalem: 0x2c, 0x78, etc. */
            volatile int nehalem_sum = 0;
            char nehalem_buffer[8192 * 1024];  /* 8MB */
            for (int i = 0; i < sizeof(nehalem_buffer); i += 256) {
                nehalem_buffer[i] = i & 0xFF;
                nehalem_sum += nehalem_buffer[i];
            }
            final_result += nehalem_sum;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Cache parameters detected: L1=%uKB/%u-way/%uB, L2=%uKB/%u-way/%uB\n",
           level1_sizekb, level1_assoc, level1_line,
           level2_sizekb, level2_assoc, level2_line);
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
