/* cache_detection.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
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

/* Function to manually decode CPUID leaf 0x2 descriptors */
__attribute__((noinline, optimize("O0")))
void decode_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j, valid_bytes = 0;
    
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
    
    /* Process valid descriptor bytes (non-zero, non-0xFF) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0 || desc == 0xFF) continue;
        
        valid_bytes++;
        
        /* Mirror the switch cases from driver-i386.cc */
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
                /* Skip if Xeon MP - we don't have that flag */
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
}

/* Architecture-specific functions with different -mtune optimizations */
__attribute__((optimize("-mtune=generic")))
void benchmark_generic(void) {
    volatile int sum = 0;
    /* Use array sizes that match common cache sizes */
    int array8k[2048];  /* ~8KB */
    int array16k[4096]; /* ~16KB */
    
    for (int i = 0; i < 2048; i++) {
        array8k[i] = i;
        sum += array8k[i];
    }
    
    for (int i = 0; i < 4096; i++) {
        array16k[i] = i * 2;
        sum += array16k[i];
    }
    
    /* Prevent optimization */
    asm volatile ("" : "+r" (sum));
}

__attribute__((optimize("-mtune=core2")))
void benchmark_core2(void) {
    volatile int sum = 0;
    /* Larger arrays for L2 cache testing */
    int array256k[65536];  /* ~256KB */
    int array1m[262144];   /* ~1MB */
    
    for (int i = 0; i < 65536; i += 64) {
        array256k[i] = i;
        sum += array256k[i];
    }
    
    for (int i = 0; i < 262144; i += 128) {
        array1m[i] = i * 3;
        sum += array1m[i];
    }
    
    asm volatile ("" : "+r" (sum));
}

__attribute__((optimize("-mtune=haswell")))
void benchmark_haswell(void) {
    volatile int sum = 0;
    /* Even larger arrays */
    int array2m[524288];   /* ~2MB */
    int array4m[1048576];  /* ~4MB */
    
    for (int i = 0; i < 524288; i += 256) {
        array2m[i] = i;
        sum += array2m[i];
    }
    
    for (int i = 0; i < 1048576; i += 512) {
        array4m[i] = i * 5;
        sum += array4m[i];
    }
    
    asm volatile ("" : "+r" (sum));
}

__attribute__((optimize("-mtune=pentium4")))
void benchmark_pentium4(void) {
    volatile int sum = 0;
    /* Pentium 4 specific cache sizes */
    int array32k[8192];    /* 32KB L1 */
    int array512k[131072]; /* 512KB L2 */
    
    for (int i = 0; i < 8192; i++) {
        array32k[i] = i;
        sum += array32k[i];
    }
    
    for (int i = 0; i < 131072; i += 32) {
        array512k[i] = i * 7;
        sum += array512k[i];
    }
    
    asm volatile ("" : "+r" (sum));
}

/* Main function with complex control flow based on CPU features */
int main(void) {
    volatile unsigned long long checksum = 0;
    
    /* Initialize CPU detection (triggers GCC's internal cache detection) */
    __builtin_cpu_init();
    
    /* Volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    /* Complex control flow with goto to prevent optimization */
    if (has_sse2) {
        benchmark_generic();
        checksum += 1;
        goto sse2_block;
    } else {
        goto legacy_block;
    }
    
sse2_block:
    if (has_avx) {
        benchmark_haswell();
        checksum += 2;
        goto avx_block;
    } else {
        benchmark_core2();
        checksum += 3;
        goto no_avx_block;
    }
    
avx_block:
    if (has_avx2) {
        /* Additional AVX2-specific benchmark */
        volatile int sum = 0;
        int array8m[2097152]; /* 8MB */
        
        for (int i = 0; i < 2097152; i += 1024) {
            array8m[i] = i;
            sum += array8m[i];
        }
        checksum += sum;
        goto final_block;
    }
    
no_avx_block:
    if (is_intel) {
        benchmark_pentium4();
        checksum += 4;
    } else if (is_amd) {
        /* AMD-specific path */
        volatile int sum = 0;
        int array128k[32768]; /* 128KB */
        
        for (int i = 0; i < 32768; i += 16) {
            array128k[i] = i;
            sum += array128k[i];
        }
        checksum += sum;
    }
    
legacy_block:
    /* Always execute the manual CPUID decoding */
    decode_cpuid_cache_descriptors();
    checksum += l1_size_kb + l2_size_kb;
    
final_block:
    /* Perform cache-size-sensitive memory access pattern */
    volatile int final_sum = 0;
    
    /* Test different array sizes that match switch case values */
    int test_sizes[] = {8, 16, 32, 128, 256, 512, 1024, 2048, 4096};
    for (int s = 0; s < 9; s++) {
        int size_kb = test_sizes[s];
        int elements = (size_kb * 1024) / sizeof(int);
        int *array = (int*)malloc(elements * sizeof(int));
        
        if (array) {
            /* Strided access to test cache effects */
            for (int i = 0; i < elements; i += 64) {
                array[i] = i + size_kb;
                final_sum += array[i];
            }
            
            /* Force memory barrier */
            asm volatile ("mfence" ::: "memory");
            
            free(array);
        }
    }
    
    checksum += final_sum;
    
    /* Print results to prevent dead code elimination */
    printf("Cache Parameters: L1=%uKB/%u-way/%uB, L2=%uKB/%u-way/%uB\n",
           l1_size_kb, l1_assoc, l1_line,
           l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %llu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
