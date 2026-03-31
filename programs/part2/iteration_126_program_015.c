/* cache_descriptor_trigger.c
 * Designed to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 cache_descriptor_trigger.c -o cache_trigger
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -std=gnu11 cache_descriptor_trigger.c -o cache_trigger
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

/* CPU feature flags - volatile to prevent optimization */
volatile int has_sse2 = 0;
volatile int has_avx = 0;
volatile int has_avx2 = 0;
volatile int is_intel = 0;
volatile int is_amd = 0;

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* Use array sizes that might trigger cache parameter consideration */
    char buffer1[8 * 1024];  /* 8KB - matches case 0x0a */
    char buffer2[16 * 1024]; /* 16KB - matches cases 0x0c, 0x0d */
    
    for (int i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = i & 0xFF;
        sum += buffer1[i];
    }
    
    for (int i = 0; i < sizeof(buffer2); i++) {
        buffer2[i] = (i * 3) & 0xFF;
        sum += buffer2[i];
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (sum));
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* Larger arrays for L2 cache testing */
    char buffer1[256 * 1024];   /* 256KB - matches case 0x21 */
    char buffer2[1024 * 1024];  /* 1MB - matches case 0x24 */
    
    for (int i = 0; i < sizeof(buffer1); i += 64) {  /* 64-byte stride */
        buffer1[i] = i & 0xFF;
        sum += buffer1[i];
    }
    
    for (int i = 0; i < sizeof(buffer2); i += 128) { /* 128-byte stride */
        buffer2[i] = (i * 7) & 0xFF;
        sum += buffer2[i];
    }
    
    asm volatile("" : "+r" (sum));
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* Very large arrays for modern cache sizes */
    char buffer1[1024 * 1024];  /* 1MB */
    char buffer2[2048 * 1024];  /* 2MB - matches case 0x45 */
    char buffer3[4096 * 1024];  /* 4MB - matches case 0x49 */
    
    /* Different access patterns */
    for (int i = 0; i < sizeof(buffer1); i += 32) {
        buffer1[i] = i & 0xFF;
        sum += buffer1[i];
    }
    
    for (int i = 0; i < sizeof(buffer2); i += 64) {
        buffer2[i] = (i * 5) & 0xFF;
        sum += buffer2[i];
    }
    
    for (int i = 0; i < sizeof(buffer3); i += 128) {
        buffer3[i] = (i * 11) & 0xFF;
        sum += buffer3[i];
    }
    
    asm volatile("" : "+r" (sum));
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, j;
    
    /* Read CPUID leaf 0x2 */
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(2), "c"(0));
    
    /* Extract descriptor bytes from registers */
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
    
    /* Process descriptors - mirroring driver-i386.cc switch statement */
    for (i = 0; i < 16; i++) {
        descriptor = descriptors[i];
        
        /* Skip null descriptors and descriptors with bit 31 set */
        if (descriptor == 0 || (descriptor & 0x80))
            continue;
            
        /* Direct mapping of hex codes to cache parameters */
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
                /* Check for Xeon MP - simplified */
                if (0) {  /* xeon_mp check would go here */
                    break;
                }
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
                /* Other descriptors not in our target range */
                break;
        }
    }
}

/* Cache-sensitive benchmark function */
void cache_sensitive_benchmark(int cache_level) {
    volatile long long sum = 0;
    clock_t start, end;
    double elapsed;
    
    /* Choose array size based on detected cache */
    size_t array_size = 0;
    if (cache_level == 1 && l1_size_kb > 0) {
        array_size = (l1_size_kb * 1024) / sizeof(int);
    } else if (cache_level == 2 && l2_size_kb > 0) {
        array_size = (l2_size_kb * 1024) / sizeof(int);
    } else {
        /* Default sizes if cache detection failed */
        array_size = (cache_level == 1) ? 2048 : 65536;  /* 8KB / 256KB in ints */
    }
    
    int *array = (int*)malloc(array_size * sizeof(int));
    if (!array) return;
    
    /* Initialize array */
    for (size_t i = 0; i < array_size; i++) {
        array[i] = (i * 3 + 1) & 0x7FFF;
    }
    
    /* Time the access pattern */
    start = clock();
    
    /* Strided access to measure cache effects */
    for (int iter = 0; iter < 1000; iter++) {
        for (size_t i = 0; i < array_size; i += 16) {
            sum += array[i];
        }
    }
    
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Use results to prevent optimization */
    asm volatile("" : "+r" (sum));
    
    free(array);
}

int main(void) {
    volatile long long final_checksum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Set volatile CPU feature flags */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Check CPU vendor */
    uint32_t eax, ebx, ecx, edx;
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(0));
    
    char vendor[13];
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';
    
    is_intel = (strcmp(vendor, "GenuineIntel") == 0);
    is_amd = (strcmp(vendor, "AuthenticAMD") == 0);
    
    /* Force execution of different optimization paths */
    if (has_sse2) {
        generic_cache_test();
        final_checksum += 1;
    }
    
    if (has_avx) {
        core2_cache_test();
        final_checksum += 2;
    }
    
    if (has_avx2) {
        haswell_cache_test();
        final_checksum += 4;
    }
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Use goto for complex control flow based on CPU features */
    volatile int cpu_type = 0;
    if (is_intel) {
        cpu_type = 1;
        goto intel_path;
    } else if (is_amd) {
        cpu_type = 2;
        goto amd_path;
    } else {
        goto other_path;
    }
    
intel_path:
    /* Intel-specific cache testing */
    cache_sensitive_benchmark(1);
    cache_sensitive_benchmark(2);
    final_checksum += 8;
    goto continue_main;
    
amd_path:
    /* AMD-specific cache testing */
    cache_sensitive_benchmark(1);
    final_checksum += 16;
    goto continue_main;
    
other_path:
    /* Generic cache testing */
    cache_sensitive_benchmark(1);
    final_checksum += 32;
    
continue_main:
    /* Additional architecture checks using __builtin_cpu_is */
    if (__builtin_cpu_is("core2")) {
        /* This should trigger cache descriptor processing for Core 2 */
        volatile int core2_array[256 * 1024];  /* 1MB */
        for (int i = 0; i < 256 * 1024; i += 64) {
            core2_array[i] = i;
            final_checksum += core2_array[i];
        }
    }
    
    if (__builtin_cpu_is("nehalem")) {
        /* Nehalem architecture */
        volatile int nehalem_array[512 * 1024];  /* 2MB */
        for (int i = 0; i < 512 * 1024; i += 128) {
            nehalem_array[i] = i * 3;
            final_checksum += nehalem_array[i];
        }
    }
    
    if (__builtin_cpu_is("haswell")) {
        /* Haswell architecture */
        volatile int haswell_array[1024 * 1024];  /* 4MB */
        for (int i = 0; i < 1024 * 1024; i += 256) {
            haswell_array[i] = i * 5;
            final_checksum += haswell_array[i];
        }
    }
    
    /* Print results and final checksum */
    printf("Cache Parameters Detected:\n");
    printf("  L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("CPU Features: SSE2=%d, AVX=%d, AVX2=%d\n", has_sse2, has_avx, has_avx2);
    printf("Vendor: %s\n", vendor);
    printf("Final checksum: %lld\n", final_checksum);
    
    return (int)(final_checksum & 0x7FFFFFFF);
}
