/* 
 * GCC Cache Descriptor Trigger Program
 * Targets driver-i386.cc cache parameter decoding logic (lines 127-244)
 * Compile with: gcc -O2 -march=native -mtune=native -fno-omit-frame-pointer -std=gnu11 -o cache_detect cache_detect.c
 * Alternative compilations for coverage:
 *   gcc -O3 -march=core2 -mtune=core2 -flto -fprofile-generate -std=gnu11 -o cache_detect cache_detect.c
 *   gcc -O1 -m32 -march=pentium4 -mtune=pentium4 -fno-inline -std=gnu11 -o cache_detect cache_detect.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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
volatile int cpu_xeon_mp = 0; /* Simulated Xeon MP flag */

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(int *arr, size_t size) {
    /* Access pattern that may benefit from cache knowledge */
    for (size_t i = 0; i < size; i += 64 / sizeof(int)) {
        arr[i] = i * 3;
    }
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(int *arr, size_t size) {
    /* Different stride for Core2 */
    for (size_t i = 0; i < size; i += 32 / sizeof(int)) {
        arr[i] = i * 5;
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(int *arr, size_t size) {
    /* AVX2-friendly pattern */
    for (size_t i = 0; i < size; i += 128 / sizeof(int)) {
        arr[i] = i * 7;
    }
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, j;
    
    /* CPUID leaf 0x2 - Cache descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2), "c"(0)
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
                if (cpu_xeon_mp)
                    break;
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

/* Cache-size sensitive benchmark */
volatile unsigned long long cache_sensitive_benchmark(size_t array_size_kb) {
    size_t elements = (array_size_kb * 1024) / sizeof(int);
    int *array = (int*)malloc(elements * sizeof(int));
    volatile unsigned long long sum = 0;
    
    if (!array) return 0;
    
    /* Initialize */
    for (size_t i = 0; i < elements; i++) {
        array[i] = (i % 256);
    }
    
    /* Access with stride to exercise cache */
    for (size_t iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < elements; i += 16) {
            sum += array[i];
        }
    }
    
    free(array);
    return sum;
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Set volatile CPU flags - force runtime evaluation */
    cpu_sse2 = __builtin_cpu_supports("sse2");
    cpu_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_avx = __builtin_cpu_supports("avx");
    cpu_avx2 = __builtin_cpu_supports("avx2");
    
    /* Simulate Xeon MP detection based on CPU model */
    const char *cpu_model = "unknown";
    if (__builtin_cpu_is("intel")) {
        cpu_model = "intel";
        /* Heuristic for Xeon MP */
        if (__builtin_cpu_supports("sse") && !__builtin_cpu_supports("sse3")) {
            cpu_xeon_mp = 1;
        }
    }
    
    unsigned long long checksum = 0;
    
    /* Force control flow based on CPU features */
    volatile int use_generic = 0;
    volatile int use_core2 = 0;
    volatile int use_haswell = 0;
    
    if (cpu_sse2 && !cpu_avx) {
        use_generic = 1;
        goto generic_path;
    } else if (cpu_sse4 && !cpu_avx2) {
        use_core2 = 1;
        goto core2_path;
    } else if (cpu_avx2) {
        use_haswell = 1;
        goto haswell_path;
    } else {
        goto generic_path;
    }
    
generic_path: {
    /* Allocate arrays matching cache sizes from uncovered block */
    size_t sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    for (int i = 0; i < 10; i++) {
        checksum += cache_sensitive_benchmark(sizes[i]);
    }
    
    int arr_generic[1024];
    generic_tuned_function(arr_generic, 1024);
    checksum += arr_generic[0];
    
    if (use_core2) goto core2_path;
    if (use_haswell) goto haswell_path;
}

core2_path: {
    int arr_core2[2048];
    core2_tuned_function(arr_core2, 2048);
    checksum += arr_core2[0];
    
    /* Read CPUID descriptors directly */
    read_cpuid_cache_descriptors();
    checksum += level1_sizekb + level2_sizekb;
    
    if (use_haswell) goto haswell_path;
    goto final;
}

haswell_path: {
    int arr_haswell[4096];
    haswell_tuned_function(arr_haswell, 4096);
    checksum += arr_haswell[0];
    
    /* Additional descriptor reading */
    read_cpuid_cache_descriptors();
    checksum += level1_assoc + level2_assoc;
    
    goto final;
}

final:
    /* Print results to prevent dead code elimination */
    printf("CPU Model: %s\n", cpu_model);
    printf("SSE2: %d, SSE4.2: %d, AVX: %d, AVX2: %d\n", 
           cpu_sse2, cpu_sse4, cpu_avx, cpu_avx2);
    printf("L1 Cache: %u KB, %u-way, %u byte line\n", 
           level1_sizekb, level1_assoc, level1_line);
    printf("L2 Cache: %u KB, %u-way, %u byte line\n", 
           level2_sizekb, level2_assoc, level2_line);
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
