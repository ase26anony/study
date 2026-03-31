/* cache_detection.c - Trigger GCC's CPUID cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables matching the uncovered structure fields */
volatile unsigned int level1_sizekb = 0;
volatile unsigned int level1_assoc = 0;
volatile unsigned int level1_line = 0;
volatile unsigned int level2_sizekb = 0;
volatile unsigned int level2_assoc = 0;
volatile unsigned int level2_line = 0;

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache (8KB-32KB) */
    char buffer1[8 * 1024];
    for (int i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = i & 0xFF;
        sum += buffer1[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to Core2 L2 cache (256KB-4MB) */
    char buffer2[256 * 1024];
    for (int i = 0; i < sizeof(buffer2); i += 64) {
        buffer2[i] = i & 0xFF;
        sum += buffer2[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to Haswell L3 cache (8MB) */
    char buffer3[8 * 1024 * 1024];
    for (int i = 0; i < sizeof(buffer3); i += 256) {
        buffer3[i] = i & 0xFF;
        sum += buffer3[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading - mirrors driver logic */
void decode_cpuid_cache_descriptors(void) {
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
}

/* Cache-sensitive benchmark */
void cache_sensitive_benchmark(unsigned int cache_size_kb) {
    volatile int sum = 0;
    size_t array_size = cache_size_kb * 1024;
    char *buffer = malloc(array_size);
    
    if (!buffer) return;
    
    /* Fill array */
    for (size_t i = 0; i < array_size; i++) {
        buffer[i] = (i * 13) & 0xFF;
    }
    
    /* Access pattern that benefits from cache line awareness */
    for (size_t iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < array_size; i += 64) {
            sum += buffer[i];
        }
    }
    
    free(buffer);
    (void)sum;
}

int main(void) {
    volatile int checksum = 0;
    
    /* Initialize CPU detection - triggers driver's cache detection */
    __builtin_cpu_init();
    
    /* Volatile flags based on CPU features */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* CPU model checks */
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    
    /* Complex control flow based on CPU detection */
    if (has_sse2) {
        checksum += 1;
        generic_cache_test();
        
        if (is_core2) {
            checksum += 2;
            core2_cache_test();
            cache_sensitive_benchmark(256);  /* Core2 L2 typical size */
            goto core2_path;
        }
    }
    
    if (has_avx) {
        checksum += 4;
        if (is_nehalem) {
            checksum += 8;
            cache_sensitive_benchmark(1024);  /* Nehalem L3 */
        }
    }
    
    if (has_avx2) {
        checksum += 16;
        if (is_haswell) {
            checksum += 32;
            haswell_cache_test();
            cache_sensitive_benchmark(8192);  /* Haswell L3 */
        }
    }
    
core2_path:
    /* Direct CPUID decoding */
    decode_cpuid_cache_descriptors();
    
    /* Use decoded values to control array sizes */
    volatile unsigned int l1_size = level1_sizekb;
    volatile unsigned int l2_size = level2_sizekb;
    
    if (l1_size > 0) {
        char l1_buffer[l1_size * 1024];
        for (unsigned int i = 0; i < sizeof(l1_buffer); i += level1_line) {
            l1_buffer[i] = checksum & 0xFF;
            checksum += l1_buffer[i];
        }
    }
    
    if (l2_size > 0) {
        char *l2_buffer = malloc(l2_size * 1024);
        if (l2_buffer) {
            for (unsigned int i = 0; i < l2_size * 1024; i += level2_line) {
                l2_buffer[i] = (checksum + i) & 0xFF;
                checksum += l2_buffer[i];
            }
            free(l2_buffer);
        }
    }
    
    /* Print results */
    printf("Checksum: %d\n", checksum);
    printf("Detected L1: %uKB, %u-way, line %uB\n", 
           level1_sizekb, level1_assoc, level1_line);
    printf("Detected L2: %uKB, %u-way, line %uB\n", 
           level2_sizekb, level2_assoc, level2_line);
    
    return 0;
}
