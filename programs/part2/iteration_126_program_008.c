/* cache_detection.c - Trigger GCC's CPUID cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables matching the cache structure pattern */
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
    char buffer[8192]; /* 8KB - matches case 0x0a */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i];
    }
    
    /* Prevent optimization */
    asm volatile("" : "+r" (sum));
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    static char buffer[32768]; /* 32KB - matches case 0x2c */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i % sizeof(buffer)] = (char)(i % 256);
        sum += buffer[i % sizeof(buffer)];
    }
    
    asm volatile("" : "+r" (sum));
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    char buffer[262144]; /* 256KB - matches case 0x21 */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i];
    }
    
    asm volatile("" : "+r" (sum));
}

/* Direct CPUID leaf 0x2 reading */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j = 0;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(2), "c"(0));
    
    /* Extract descriptor bytes from registers */
    descriptors[j++] = (eax >> 0) & 0xFF;
    descriptors[j++] = (eax >> 8) & 0xFF;
    descriptors[j++] = (eax >> 16) & 0xFF;
    descriptors[j++] = (eax >> 24) & 0xFF;
    
    descriptors[j++] = (ebx >> 0) & 0xFF;
    descriptors[j++] = (ebx >> 8) & 0xFF;
    descriptors[j++] = (ebx >> 16) & 0xFF;
    descriptors[j++] = (ebx >> 24) & 0xFF;
    
    descriptors[j++] = (ecx >> 0) & 0xFF;
    descriptors[j++] = (ecx >> 8) & 0xFF;
    descriptors[j++] = (ecx >> 16) & 0xFF;
    descriptors[j++] = (ecx >> 24) & 0xFF;
    
    descriptors[j++] = (edx >> 0) & 0xFF;
    descriptors[j++] = (edx >> 8) & 0xFF;
    descriptors[j++] = (edx >> 16) & 0xFF;
    descriptors[j++] = (edx >> 24) & 0xFF;
    
    /* Process descriptors - mirroring the driver's switch logic */
    for (i = 0; i < j; i++) {
        if (descriptors[i] & 0x80) continue; /* Invalid descriptor */
        
        switch (descriptors[i]) {
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
                /* Simulating xeon_mp check */
                if (0) break; /* Assume not xeon_mp */
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
                /* Other descriptors not in uncovered block */
                break;
        }
    }
}

/* Cache-sensitive benchmark */
void cache_sensitive_benchmark(int cache_size_kb) {
    volatile int sum = 0;
    int size_bytes = cache_size_kb * 1024;
    char *buffer = malloc(size_bytes);
    
    if (!buffer) return;
    
    /* Initialize */
    for (int i = 0; i < size_bytes; i++) {
        buffer[i] = (char)(i % 256);
    }
    
    /* Access pattern that stresses cache */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < size_bytes; i += 64) {
            sum += buffer[i];
        }
    }
    
    free(buffer);
    asm volatile("" : "+r" (sum));
}

int main(void) {
    volatile int checksum = 0;
    
    /* Initialize CPU detection - triggers driver's cache detection */
    __builtin_cpu_init();
    
    /* Use volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    
    /* Call architecture-specific functions */
    generic_cache_test();
    checksum += 1;
    
    if (has_sse2) {
        core2_cache_test();
        checksum += 2;
        
        /* Jump based on CPU flags */
        if (is_core2) {
            goto core2_path;
        } else {
            goto generic_path;
        }
    }
    
core2_path:
    cache_sensitive_benchmark(256);  /* L2 size for case 0x21 */
    checksum += 4;
    goto after_benchmark;
    
generic_path:
    cache_sensitive_benchmark(8);    /* L1 size for case 0x0a */
    checksum += 8;
    
after_benchmark:
    
    if (has_avx) {
        haswell_cache_test();
        checksum += 16;
        
        cache_sensitive_benchmark(1024); /* L2 size for case 0x24 */
        checksum += 32;
    }
    
    /* Direct CPUID reading */
    read_cpuid_cache_descriptors();
    checksum += level1_sizekb + level2_sizekb;
    
    /* Additional conditional compilation paths */
#ifdef __x86_64__
    cache_sensitive_benchmark(2048); /* L2 size for case 0x45 */
    checksum += 64;
#else
    cache_sensitive_benchmark(512);  /* L2 size for case 0x3e */
    checksum += 128;
#endif
    
    /* Matrix traversal - cache size sensitive */
    {
        const int N = 256;  /* Adjust based on cache size */
        volatile int matrix_sum = 0;
        int *matrix = malloc(N * N * sizeof(int));
        
        if (matrix) {
            /* Initialize */
            for (int i = 0; i < N * N; i++) {
                matrix[i] = i % 100;
            }
            
            /* Row-major traversal */
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    matrix_sum += matrix[i * N + j];
                }
            }
            
            /* Column-major traversal (cache-unfriendly) */
            for (int j = 0; j < N; j++) {
                for (int i = 0; i < N; i++) {
                    matrix_sum += matrix[i * N + j];
                }
            }
            
            free(matrix);
            checksum += matrix_sum;
        }
    }
    
    printf("Cache detection checksum: %d\n", checksum);
    printf("Detected L1: %dKB, %d-way, %dB line\n", 
           level1_sizekb, level1_assoc, level1_line);
    printf("Detected L2: %dKB, %d-way, %dB line\n", 
           level2_sizekb, level2_assoc, level2_line);
    
    return 0;
}
