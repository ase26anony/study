/* cache_detector.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
typedef struct {
    volatile int sizekb;
    volatile int assoc;
    volatile int line;
} cache_params;

cache_params level1 = {0, 0, 0};
cache_params level2 = {0, 0, 0};
volatile int xeon_mp = 0;

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache */
    char buffer1[8 * 1024];  /* 8KB */
    for (int i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = i & 0xFF;
        sum += buffer1[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to Core2 L2 cache */
    char buffer2[2 * 1024 * 1024];  /* 2MB */
    for (int i = 0; i < sizeof(buffer2); i += 64) {
        buffer2[i] = i & 0xFF;
        sum += buffer2[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to Haswell L3 cache */
    char buffer3[8 * 1024 * 1024];  /* 8MB */
    for (int i = 0; i < sizeof(buffer3); i += 128) {
        buffer3[i] = i & 0xFF;
        sum += buffer3[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t *descriptors;
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract descriptor bytes from registers */
    uint8_t regs[16];
    memcpy(regs, &eax, 4);
    memcpy(regs + 4, &ebx, 4);
    memcpy(regs + 4, &ecx, 4);
    memcpy(regs + 12, &edx, 4);
    
    /* Process each non-zero descriptor byte */
    for (i = 0; i < 16; i++) {
        uint8_t desc = regs[i];
        if (desc == 0) continue;
        
        /* Switch statement mirroring driver-i386.cc uncovered block */
        switch (desc) {
            case 0x0a:
                level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
                break;
            case 0x0c:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
                break;
            case 0x0d:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
                break;
            case 0x0e:
                level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
                break;
            case 0x21:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
                break;
            case 0x24:
                level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
                break;
            case 0x2c:
                level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
                break;
            case 0x39:
                level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
                break;
            case 0x3a:
                level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
                break;
            case 0x3b:
                level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
                break;
            case 0x3c:
                level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
                break;
            case 0x3d:
                level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
                break;
            case 0x3e:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
                break;
            case 0x41:
                level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
                break;
            case 0x42:
                level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
                break;
            case 0x43:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
                break;
            case 0x44:
                level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
                break;
            case 0x45:
                level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
                break;
            case 0x48:
                level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
                break;
            case 0x49:
                if (xeon_mp) break;
                level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
                break;
            case 0x4e:
                level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
                break;
            case 0x60:
                level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
                break;
            case 0x66:
                level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
                break;
            case 0x67:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
                break;
            case 0x68:
                level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
                break;
            case 0x78:
                level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
                break;
            case 0x79:
                level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7a:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7b:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7c:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7d:
                level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7f:
                level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
                break;
            case 0x80:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
                break;
            case 0x82:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
                break;
            case 0x83:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
                break;
            case 0x84:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
                break;
            case 0x85:
                level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
                break;
            case 0x86:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
                break;
            case 0x87:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
                break;
            default:
                /* Unknown descriptor - skip */
                break;
        }
    }
}

/* Cache-sensitive benchmark */
volatile int perform_cache_benchmark(int cache_size_kb) {
    volatile int sum = 0;
    int size = cache_size_kb * 1024;
    char *buffer = malloc(size);
    
    if (!buffer) return 0;
    
    /* Fill buffer */
    for (int i = 0; i < size; i++) {
        buffer[i] = i & 0xFF;
    }
    
    /* Access with stride equal to cache line size */
    int stride = 64;  /* Typical cache line */
    for (int i = 0; i < size; i += stride) {
        sum += buffer[i];
    }
    
    free(buffer);
    return sum;
}

int main(void) {
    volatile int checksum = 0;
    
    /* Initialize CPU detection (triggers driver cache detection) */
    __builtin_cpu_init();
    
    /* Use volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    /* Force compiler to consider different CPU paths */
    if (has_sse2) {
        checksum += 1;
        generic_cache_test();
    }
    
    if (has_avx) {
        checksum += 2;
        core2_cache_test();
    }
    
    if (has_avx2) {
        checksum += 4;
        haswell_cache_test();
    }
    
    /* Direct CPUID cache descriptor reading */
    read_cpuid_cache_descriptors();
    
    /* Conditional jumps based on CPU features */
    volatile int cpu_flags = has_sse2 | (has_avx << 1) | (has_avx2 << 2);
    
    /* Goto-based control flow to prevent optimization */
    if (cpu_flags & 1) goto sse2_block;
    if (cpu_flags & 2) goto avx_block;
    if (cpu_flags & 4) goto avx2_block;
    
    goto end;
    
sse2_block:
    checksum += perform_cache_benchmark(8);   /* L1 size */
    checksum += perform_cache_benchmark(256); /* L2 size */
    goto next;
    
avx_block:
    checksum += perform_cache_benchmark(16);   /* L1 size */
    checksum += perform_cache_benchmark(1024); /* L2 size */
    goto next;
    
avx2_block:
    checksum += perform_cache_benchmark(32);   /* L1 size */
    checksum += perform_cache_benchmark(2048); /* L2 size */
    goto next;
    
next:
    /* Additional architecture-specific tests */
    if (is_intel) {
        /* Test Intel-specific cache sizes */
        checksum += perform_cache_benchmark(24);  /* Case 0x0e */
        checksum += perform_cache_benchmark(192); /* Case 0x3a */
    }
    
    if (is_amd) {
        /* Test AMD-specific cache sizes */
        checksum += perform_cache_benchmark(64);  /* Common AMD L1 */
        checksum += perform_cache_benchmark(512); /* Common AMD L2 */
    }
    
end:
    /* Print results to prevent dead code elimination */
    printf("Cache detection results:\n");
    printf("  L1: %dKB, %d-way, %dB line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("  L2: %dKB, %d-way, %dB line\n", 
           level2.sizekb, level2.assoc, level2.line);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
