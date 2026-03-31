/* cache_detector.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
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

/* Architecture-specific functions with different tuning */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(void) {
    volatile int sum = 0;
    char buffer[8192]; /* 8KB - matches case 0x0a */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i & 0xFF;
        sum += buffer[i];
    }
    asm volatile("" : "+r" (sum));
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(void) {
    volatile int sum = 0;
    char buffer[32768]; /* 32KB - matches case 0x2c */
    for (int i = 0; i < sizeof(buffer); i += 64) { /* 64-byte line */
        buffer[i] = i & 0xFF;
        sum += buffer[i];
    }
    asm volatile("" : "+r" (sum));
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(void) {
    volatile int sum = 0;
    char buffer[262144]; /* 256KB - matches case 0x21 */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
        sum += buffer[i];
    }
    asm volatile("" : "+r" (sum));
}

__attribute__((optimize("-mtune=skylake")))
void skylake_tuned_function(void) {
    volatile int sum = 0;
    char buffer[1048576]; /* 1024KB - matches case 0x24 */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
        sum += buffer[i];
    }
    asm volatile("" : "+r" (sum));
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j;
    
    /* CPUID leaf 0x2 - Cache Descriptors */
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(2), "c"(0));
    
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
    
    /* Process descriptors - mirroring driver-i386.cc switch */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0x00) continue; /* Null descriptor */
        
        switch (desc) {
            /* L1 cache descriptors */
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
            case 0x2c:
                l1_size_kb = 32; l1_assoc = 8; l1_line = 64;
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
                
            /* L2 cache descriptors */
            case 0x21:
                l2_size_kb = 256; l2_assoc = 8; l2_line = 64;
                break;
            case 0x24:
                l2_size_kb = 1024; l2_assoc = 16; l2_line = 64;
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
                /* Check xeon_mp flag - we'll assume false */
                l2_size_kb = 4096; l2_assoc = 16; l2_line = 64;
                break;
            case 0x4e:
                l2_size_kb = 6144; l2_assoc = 24; l2_line = 64;
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

/* Cache-sensitive benchmark */
void cache_sensitive_benchmark(volatile int use_sse, volatile int use_avx) {
    unsigned long long sum = 0;
    clock_t start, end;
    double elapsed;
    
    /* Array sizes matching cache descriptor values */
    int sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    start = clock();
    
    /* Complex control flow based on CPU features */
    if (use_sse) {
        goto sse_block;
    } else if (use_avx) {
        goto avx_block;
    } else {
        goto generic_block;
    }
    
sse_block:
    for (int s = 0; s < num_sizes; s++) {
        int size_kb = sizes[s];
        char *buffer = malloc(size_kb * 1024);
        if (!buffer) continue;
        
        /* Access pattern sensitive to cache size */
        for (int i = 0; i < size_kb * 1024; i += 64) {
            buffer[i] = (i * 13) & 0xFF;
            sum += buffer[i];
        }
        
        free(buffer);
    }
    goto benchmark_end;
    
avx_block:
    for (int s = 0; s < num_sizes; s++) {
        int size_kb = sizes[s];
        char *buffer = malloc(size_kb * 1024);
        if (!buffer) continue;
        
        /* Different stride for AVX-optimized */
        for (int i = 0; i < size_kb * 1024; i += 128) {
            buffer[i] = (i * 17) & 0xFF;
            sum += buffer[i];
        }
        
        free(buffer);
    }
    goto benchmark_end;
    
generic_block:
    for (int s = 0; s < num_sizes; s++) {
        int size_kb = sizes[s];
        char *buffer = malloc(size_kb * 1024);
        if (!buffer) continue;
        
        /* Generic access pattern */
        for (int i = 0; i < size_kb * 1024; i++) {
            buffer[i] = i & 0xFF;
            sum += buffer[i];
        }
        
        free(buffer);
    }
    
benchmark_end:
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (sum));
    printf("Benchmark completed in %.6f seconds (checksum: %llu)\n", elapsed, sum);
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse4 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    printf("CPU Features: SSE2=%d, SSE4.2=%d, AVX=%d, AVX2=%d\n",
           has_sse2, has_sse4, has_avx, has_avx2);
    
    /* Call architecture-tuned functions */
    generic_tuned_function();
    if (has_sse2) {
        core2_tuned_function();
    }
    if (has_avx) {
        haswell_tuned_function();
    }
    if (has_avx2) {
        skylake_tuned_function();
    }
    
    /* Direct CPUID cache descriptor reading */
    read_cpuid_cache_descriptors();
    
    printf("Detected Cache Parameters:\n");
    printf("  L1: %u KB, %u-way, %u-byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %u KB, %u-way, %u-byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    
    /* Run cache-sensitive benchmark */
    cache_sensitive_benchmark(has_sse2, has_avx);
    
    /* Additional CPU model checks */
    if (__builtin_cpu_is("intel")) {
        printf("Intel CPU detected\n");
        
        /* More specific model checks */
        volatile int is_core2 = __builtin_cpu_is("core2");
        volatile int is_nehalem = __builtin_cpu_is("nehalem");
        volatile int is_skylake = __builtin_cpu_is("skylake");
        
        if (is_core2) {
            printf("Core 2 microarchitecture\n");
        }
        if (is_nehalem) {
            printf("Nehalem microarchitecture\n");
        }
        if (is_skylake) {
            printf("Skylake microarchitecture\n");
        }
    } else if (__builtin_cpu_is("amd")) {
        printf("AMD CPU detected\n");
    }
    
    /* Final checksum to prevent optimization */
    volatile unsigned long long final_sum = 0;
    final_sum += l1_size_kb + l1_assoc + l1_line;
    final_sum += l2_size_kb + l2_assoc + l2_line;
    final_sum += has_sse2 + has_sse4 + has_avx + has_avx2;
    
    printf("Final checksum: %llu\n", final_sum);
    
    return 0;
}
