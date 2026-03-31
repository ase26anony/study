/* 
 * cache_detection_test.c
 * Designed to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 cache_detection_test.c -o cache_test
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 cache_detection_test.c -o cache_test
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

/* Function to directly read CPUID leaf 0x2 cache descriptors */
__attribute__((noinline, optimize("O0")))
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, j;
    
    /* CPUID leaf 0x2 - Cache and TLB Descriptor */
    asm volatile ("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(2), "c"(0));
    
    /* Process descriptor bytes from registers */
    uint8_t *regs = (uint8_t*)&eax;
    
    for (i = 0; i < 4; i++) {
        descriptor = regs[i];
        if (descriptor == 0) continue;
        
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
                /* Simulating xeon_mp check */
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
        descriptor = regs[i];
        if (descriptor == 0) continue;
        /* Same switch would be repeated in actual implementation */
    }
}

/* Architecture-specific functions with different mtune optimizations */
__attribute__((optimize("-mtune=generic"), noinline))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache */
    int array[2048]; /* ~8KB */
    for (int i = 0; i < 2048; i++) {
        array[i] = i;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2"), noinline))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* Larger array for L2 cache */
    int array[262144]; /* ~1MB */
    for (int i = 0; i < 262144; i++) {
        array[i] = i % 256;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell"), noinline))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* Even larger array */
    int array[524288]; /* ~2MB */
    for (int i = 0; i < 524288; i++) {
        array[i] = i % 512;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=pentium4"), noinline))
void pentium4_cache_test(void) {
    volatile int sum = 0;
    /* Pentium 4 specific cache size */
    int array[32768]; /* ~128KB */
    for (int i = 0; i < 32768; i++) {
        array[i] = i % 128;
        sum += array[i];
    }
    (void)sum;
}

/* Cache-sensitive benchmark function */
__attribute__((noinline))
unsigned long long cache_sensitive_benchmark(size_t size_kb) {
    unsigned long long sum = 0;
    size_t elements = (size_kb * 1024) / sizeof(int);
    int *array = (int*)malloc(elements * sizeof(int));
    
    if (!array) return 0;
    
    /* Initialize array */
    for (size_t i = 0; i < elements; i++) {
        array[i] = (i * 3) % 256;
    }
    
    /* Access pattern that benefits from cache awareness */
    for (size_t iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < elements; i += 64) { /* 64-byte cache line stride */
            sum += array[i % elements];
        }
    }
    
    free(array);
    return sum;
}

int main(void) {
    volatile unsigned long long checksum = 0;
    
    /* Initialize CPU detection - triggers GCC's internal cache detection */
    __builtin_cpu_init();
    
    /* Volatile flags based on CPU features */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    /* Read cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Control flow based on CPU features - forces compiler to consider different arch paths */
    if (has_sse2) {
        generic_cache_test();
        checksum += 1;
        
        if (is_intel) {
            core2_cache_test();
            checksum += 2;
            
            /* Jump to different blocks based on features */
            if (has_avx) {
                haswell_cache_test();
                checksum += 4;
                goto avx_block;
            }
        }
        
        if (is_amd) {
            pentium4_cache_test(); /* Some AMD chips might match P4 cache patterns */
            checksum += 8;
        }
    }
    
avx_block:
    if (has_avx2) {
        checksum += 16;
    }
    
    /* Perform cache-sensitive benchmarks with sizes from uncovered block */
    checksum += cache_sensitive_benchmark(8);    /* 0x0a */
    checksum += cache_sensitive_benchmark(16);   /* 0x0c, 0x0d */
    checksum += cache_sensitive_benchmark(32);   /* 0x2c */
    checksum += cache_sensitive_benchmark(128);  /* 0x39, 0x3b, 0x41 */
    checksum += cache_sensitive_benchmark(256);  /* 0x21, 0x3c, 0x42 */
    checksum += cache_sensitive_benchmark(512);  /* 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
    checksum += cache_sensitive_benchmark(1024); /* 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
    checksum += cache_sensitive_benchmark(2048); /* 0x45, 0x7d, 0x85 */
    checksum += cache_sensitive_benchmark(4096); /* 0x49 */
    
    /* Print results to prevent optimization */
    printf("Cache detection test results:\n");
    printf("L1 Cache: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2 Cache: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %llu\n", checksum);
    printf("CPU Features: SSE2=%d, AVX=%d, AVX2=%d, Intel=%d, AMD=%d\n",
           has_sse2, has_avx, has_avx2, is_intel, is_amd);
    
    return 0;
}
