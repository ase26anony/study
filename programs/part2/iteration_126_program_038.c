/* 
 * cache_detection.c - Program to trigger GCC's CPU cache detection logic
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 cache_detection.c -o cache_detection
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -std=gnu11 cache_detection.c -o cache_detection
 */

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

/* Function to manually decode CPUID leaf 0x2 cache descriptors */
__attribute__((noinline, optimize("O0")))
void decode_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x2)
    );
    
    /* Extract descriptor bytes from registers */
    descriptors[0] = (eax >> 0) & 0xFF;
    descriptors[1] = (eax >> 8) & 0xFF;
    descriptors[2] = (eax >> 16) & 0xFF;
    descriptors[3] = (eax >> 24) & 0xFF;
    descriptors[4] = (ebx >> 0) & 0xFF;
    descriptors[5] = (ebx >> 8) & 0xFF;
    descriptors[6] = (ebx >> 16) & 0xFF;
    descriptors[7] = (ebx >> 24) & 0xFF;
    descriptors[8] = (ecx >> 0) & 0xFF;
    descriptors[9] = (ecx >> 8) & 0xFF;
    descriptors[10] = (ecx >> 16) & 0xFF;
    descriptors[11] = (ecx >> 24) & 0xFF;
    descriptors[12] = (edx >> 0) & 0xFF;
    descriptors[13] = (edx >> 8) & 0xFF;
    descriptors[14] = (edx >> 16) & 0xFF;
    descriptors[15] = (edx >> 24) & 0xFF;
    
    /* Process each descriptor byte (mirroring driver-i386.cc logic) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0x01) continue;
        
        /* Switch statement matching uncovered lines 127-244 */
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
                /* Simulating xeon_mp check */
                if (0) break; /* Assume not xeon_mp */
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
}

/* Architecture-specific functions with different mtune optimizations */
__attribute__((optimize("-mtune=generic")))
void benchmark_generic(void) {
    volatile int sum = 0;
    /* Use array sizes that match common cache sizes */
    char array8k[8 * 1024];
    char array256k[256 * 1024];
    
    for (int i = 0; i < sizeof(array8k); i++) {
        array8k[i] = (char)(i & 0xFF);
        sum += array8k[i];
    }
    
    for (int i = 0; i < sizeof(array256k); i += 64) {
        array256k[i] = (char)(i & 0xFF);
        sum += array256k[i];
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(sum));
}

__attribute__((optimize("-mtune=core2")))
void benchmark_core2(void) {
    volatile int sum = 0;
    /* Core2 typically has 32KB L1, 2-4MB L2 */
    char array32k[32 * 1024];
    char array2m[2 * 1024 * 1024];
    
    for (int i = 0; i < sizeof(array32k); i++) {
        array32k[i] = (char)(i & 0xFF);
        sum += array32k[i];
    }
    
    for (int i = 0; i < sizeof(array2m); i += 128) {
        array2m[i] = (char)(i & 0xFF);
        sum += array2m[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

__attribute__((optimize("-mtune=haswell")))
void benchmark_haswell(void) {
    volatile int sum = 0;
    /* Haswell typically has 32KB L1, 256KB L2 per core */
    char array32k[32 * 1024];
    char array256k[256 * 1024];
    
    for (int i = 0; i < sizeof(array32k); i++) {
        array32k[i] = (char)(i & 0xFF);
        sum += array32k[i];
    }
    
    for (int i = 0; i < sizeof(array256k); i += 64) {
        array256k[i] = (char)(i & 0xFF);
        sum += array256k[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

__attribute__((optimize("-mtune=pentium4")))
void benchmark_pentium4(void) {
    volatile int sum = 0;
    /* Pentium 4 typically has 8KB L1, 256-512KB L2 */
    char array8k[8 * 1024];
    char array512k[512 * 1024];
    
    for (int i = 0; i < sizeof(array8k); i++) {
        array8k[i] = (char)(i & 0xFF);
        sum += array8k[i];
    }
    
    for (int i = 0; i < sizeof(array512k); i += 64) {
        array512k[i] = (char)(i & 0xFF);
        sum += array512k[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

/* Matrix traversal function that is cache-size sensitive */
__attribute__((noinline))
unsigned long long cache_sensitive_matrix(int size_kb) {
    int dimension = (int)sqrt(size_kb * 1024 / sizeof(double));
    if (dimension < 16) dimension = 16;
    
    double *matrix = (double*)malloc(dimension * dimension * sizeof(double));
    volatile double sum = 0.0;
    
    /* Initialize matrix */
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            matrix[i * dimension + j] = (i + j) * 0.1;
        }
    }
    
    /* Traverse in different patterns */
    for (int iter = 0; iter < 10; iter++) {
        /* Row-major */
        for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                sum += matrix[i * dimension + j];
            }
        }
        
        /* Column-major */
        for (int j = 0; j < dimension; j++) {
            for (int i = 0; i < dimension; i++) {
                sum += matrix[i * dimension + j];
            }
        }
    }
    
    free(matrix);
    return (unsigned long long)sum;
}

int main(void) {
    volatile unsigned long long checksum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Use volatile flags to force evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Force control flow based on CPU features */
    if (has_sse2) {
        benchmark_generic();
        checksum += 1;
        
        if (has_avx) {
            benchmark_haswell();
            checksum += 2;
            
            if (has_avx2) {
                benchmark_core2(); /* Some AVX2 CPUs */
                checksum += 4;
            }
        } else {
            benchmark_pentium4();
            checksum += 8;
        }
    }
    
    /* Decode CPUID cache descriptors directly */
    decode_cpuid_cache_descriptors();
    
    /* Perform cache-sensitive operations based on detected parameters */
    if (l1_size_kb > 0) {
        checksum += cache_sensitive_matrix(l1_size_kb);
    }
    
    if (l2_size_kb > 0) {
        checksum += cache_sensitive_matrix(l2_size_kb);
    }
    
    /* Use goto to create complex control flow */
    volatile int cpu_type = 0;
    
    if (__builtin_cpu_is("intel")) {
        cpu_type = 1;
        goto intel_path;
    } else if (__builtin_cpu_is("amd")) {
        cpu_type = 2;
        goto amd_path;
    } else {
        goto unknown_path;
    }
    
intel_path:
    checksum += 0x1000;
    /* Fall through */
    
amd_path:
    checksum += 0x2000;
    /* Fall through */
    
unknown_path:
    checksum += 0x4000;
    
    /* Print results to prevent dead code elimination */
    printf("Cache Detection Results:\n");
    printf("  L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("  CPU Features: SSE2=%d, AVX=%d, AVX2=%d\n", has_sse2, has_avx, has_avx2);
    printf("  Final checksum: %llu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
