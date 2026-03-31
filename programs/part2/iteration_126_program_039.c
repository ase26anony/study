/* cache_descriptor_trigger.c
 * Designed to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 -fno-omit-frame-pointer
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -std=gnu11
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    char buffer[8192]; /* 8KB - matches case 0x0a */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    char buffer[32768]; /* 32KB - matches case 0x2c */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    char buffer[32768]; /* 32KB L1 */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, j;
    
    /* CPUID leaf 0x2 - Cache Descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Process descriptor bytes from registers */
    uint8_t descriptors[16];
    descriptors[0] = eax & 0xFF;
    descriptors[1] = (eax >> 8) & 0xFF;
    descriptors[2] = (eax >> 16) & 0xFF;
    descriptors[3] = (eax >> 24) & 0xFF;
    descriptors[4] = ebx & 0xFF;
    descriptors[5] = (ebx >> 8) & 0xFF;
    descriptors[6] = (ebx >> 16) & 0xFF;
    descriptors[7] = (ebx >> 24) & 0xFF;
    descriptors[8] = ecx & 0xFF;
    descriptors[9] = (ecx >> 8) & 0xFF;
    descriptors[10] = (ecx >> 16) & 0xFF;
    descriptors[11] = (ecx >> 24) & 0xFF;
    descriptors[12] = edx & 0xFF;
    descriptors[13] = (edx >> 8) & 0xFF;
    descriptors[14] = (edx >> 16) & 0xFF;
    descriptors[15] = (edx >> 24) & 0xFF;
    
    /* Switch statement mirroring driver-i386.cc uncovered block */
    for (i = 0; i < 16; i++) {
        descriptor = descriptors[i];
        
        /* Skip invalid descriptors */
        if (descriptor == 0 || (descriptor & 0x80))
            continue;
            
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
}

/* Cache-sensitive benchmark function */
void cache_sensitive_benchmark(volatile int use_sse2, volatile int use_avx) {
    int i, j;
    volatile int sum = 0;
    
    /* Array sizes matching cache sizes from uncovered block */
    int sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    /* Control flow based on CPU features */
    if (use_sse2) {
        goto sse2_block;
    } else if (use_avx) {
        goto avx_block;
    } else {
        goto generic_block;
    }
    
sse2_block:
    for (j = 0; j < num_sizes; j++) {
        int size_kb = sizes[j];
        char *array = malloc(size_kb * 1024);
        
        if (array) {
            /* Access pattern sensitive to cache size */
            for (i = 0; i < size_kb * 1024; i += 64) {
                array[i] = (char)(i & 0xFF);
                sum += array[i];
            }
            free(array);
        }
    }
    goto end;
    
avx_block:
    for (j = 0; j < num_sizes; j++) {
        int size_kb = sizes[j];
        char *array = malloc(size_kb * 1024);
        
        if (array) {
            /* Different stride for AVX */
            for (i = 0; i < size_kb * 1024; i += 128) {
                array[i] = (char)(i & 0xFF);
                sum += array[i];
            }
            free(array);
        }
    }
    goto end;
    
generic_block:
    for (j = 0; j < num_sizes; j++) {
        int size_kb = sizes[j];
        char *array = malloc(size_kb * 1024);
        
        if (array) {
            for (i = 0; i < size_kb * 1024; i += 32) {
                array[i] = (char)(i & 0xFF);
                sum += array[i];
            }
            free(array);
        }
    }
    
end:
    /* Prevent optimization */
    asm volatile ("" : : "r"(sum));
}

int main(void) {
    volatile int checksum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Volatile flags based on CPU features */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Call architecture-specific functions */
    generic_cache_test();
    checksum += 1;
    
    if (has_sse2) {
        core2_cache_test();
        checksum += 2;
    }
    
    if (has_avx || has_avx2) {
        haswell_cache_test();
        checksum += 4;
    }
    
    /* Direct CPUID cache descriptor reading */
    read_cpuid_cache_descriptors();
    checksum += l1_size_kb + l2_size_kb;
    
    /* Cache-sensitive benchmark with control flow */
    cache_sensitive_benchmark(has_sse2, has_avx);
    checksum += 8;
    
    /* Additional CPU model checks */
    volatile int is_core2 = 0;
    volatile int is_nehalem = 0;
    volatile int is_haswell = 0;
    
    /* Use __builtin_cpu_is to trigger more detection paths */
    if (__builtin_cpu_is("core2")) {
        is_core2 = 1;
        checksum += 16;
    }
    if (__builtin_cpu_is("nehalem")) {
        is_nehalem = 1;
        checksum += 32;
    }
    if (__builtin_cpu_is("haswell")) {
        is_haswell = 1;
        checksum += 64;
    }
    
    /* Matrix traversal - cache size sensitive */
    {
        int matrix_size = 256; /* 256x256 int matrix ~ 256KB */
        if (l2_size_kb >= 256) {
            matrix_size = 512; /* 512x512 int matrix ~ 1MB */
        }
        
        int *matrix = malloc(matrix_size * matrix_size * sizeof(int));
        if (matrix) {
            for (int i = 0; i < matrix_size; i++) {
                for (int j = 0; j < matrix_size; j++) {
                    matrix[i * matrix_size + j] = i + j;
                    checksum += matrix[i * matrix_size + j];
                }
            }
            free(matrix);
        }
    }
    
    /* Print results */
    printf("Cache parameters detected:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
