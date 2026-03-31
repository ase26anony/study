/* cache_descriptor_trigger.c
 * Targets GCC driver-i386.cc cache descriptor decoding (lines 127-244)
 * Compile with: gcc -O2 -march=native -mtune=native -fno-omit-frame-pointer -std=gnu11
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -flto
 * Or for 32-bit: gcc -O1 -m32 -march=pentium4 -mtune=pentium4 -fno-inline
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* Global variables to store cache parameters matching the uncovered switch cases */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* CPUID leaf 0x2 descriptor extraction */
static void extract_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract descriptor bytes from registers */
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
    
    /* Process descriptors - mirroring the uncovered switch logic */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors (0x00) and indicator bytes (0x01) */
        if (desc == 0x00 || desc == 0x01)
            continue;
            
        /* Direct mirror of the uncovered switch cases */
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
                if (0) /* Placeholder for xeon_mp detection */
                    break;
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
static void benchmark_generic(void) {
    volatile int sum = 0;
    /* Use array sizes that match common cache sizes */
    char buffer8k[8 * 1024];      /* 8KB - matches case 0x0a */
    char buffer16k[16 * 1024];    /* 16KB - matches case 0x0c, 0x0d */
    char buffer32k[32 * 1024];    /* 32KB - matches case 0x2c */
    
    /* Access patterns that depend on cache parameters */
    for (int i = 0; i < sizeof(buffer8k); i += 32)  /* 32-byte line */
        sum += buffer8k[i];
    for (int i = 0; i < sizeof(buffer16k); i += 64) /* 64-byte line */
        sum += buffer16k[i];
    
    (void)sum; /* Prevent optimization */
}

__attribute__((optimize("-mtune=core2")))
static void benchmark_core2(void) {
    volatile int sum = 0;
    /* Core2 typically has 32KB L1, 256KB/4MB L2 */
    char buffer32k[32 * 1024];    /* 32KB L1 */
    char buffer256k[256 * 1024];  /* 256KB L2 - matches case 0x21 */
    char buffer4m[4 * 1024 * 1024]; /* 4MB L2 - matches case 0x49 */
    
    for (int i = 0; i < sizeof(buffer32k); i += 64)
        sum += buffer32k[i];
    for (int i = 0; i < sizeof(buffer256k); i += 64)
        sum += buffer256k[i];
    
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
static void benchmark_haswell(void) {
    volatile int sum = 0;
    /* Haswell typically has 32KB L1, 256KB L2 per core */
    char buffer32k[32 * 1024];
    char buffer256k[256 * 1024];
    
    for (int i = 0; i < sizeof(buffer32k); i += 64)
        sum += buffer32k[i];
    for (int i = 0; i < sizeof(buffer256k); i += 64)
        sum += buffer256k[i];
    
    (void)sum;
}

__attribute__((optimize("-mtune=pentium4")))
static void benchmark_pentium4(void) {
    volatile int sum = 0;
    /* Pentium 4 had 8KB L1, 256KB/512KB L2 */
    char buffer8k[8 * 1024];      /* 8KB L1 - matches case 0x0a */
    char buffer256k[256 * 1024];  /* 256KB L2 - matches case 0x21 */
    char buffer512k[512 * 1024];  /* 512KB L2 - matches case 0x3e */
    
    for (int i = 0; i < sizeof(buffer8k); i += 64)
        sum += buffer8k[i];
    for (int i = 0; i < sizeof(buffer256k); i += 64)
        sum += buffer256k[i];
    
    (void)sum;
}

/* Cache-size sensitive matrix traversal */
static unsigned long long cache_sensitive_benchmark(unsigned int cache_size_kb) {
    const unsigned int elements = (cache_size_kb * 1024) / sizeof(int);
    volatile int* array = (volatile int*)malloc(elements * sizeof(int));
    unsigned long long sum = 0;
    
    if (!array) return 0;
    
    /* Initialize */
    for (unsigned int i = 0; i < elements; i++) {
        array[i] = i & 0xFF;
    }
    
    /* Traverse with stride matching typical cache line sizes */
    for (unsigned int i = 0; i < elements; i += 16) { /* 64-byte stride for 4-byte ints */
        sum += array[i];
    }
    
    free((void*)array);
    return sum;
}

int main(void) {
    volatile unsigned long long checksum = 0;
    
    /* Initialize CPU detection - triggers GCC's internal cache detection */
    __builtin_cpu_init();
    
    /* Use volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    
    /* Extract cache descriptors directly */
    extract_cache_descriptors();
    
    /* Control flow based on CPU features - forces GCC to consider different arch paths */
    if (has_sse2) {
        benchmark_generic();
        checksum += 1;
        
        if (is_core2) {
            benchmark_core2();
            checksum += 2;
            goto core2_path;
        }
    }
    
    if (has_avx) {
        if (is_nehalem || is_haswell) {
            benchmark_haswell();
            checksum += 4;
            goto haswell_path;
        }
    }
    
    /* Fallback path */
    benchmark_pentium4();
    checksum += 8;
    goto final_benchmark;
    
core2_path:
    /* Perform benchmarks matching Core2 cache sizes */
    checksum += cache_sensitive_benchmark(32);   /* L1 */
    checksum += cache_sensitive_benchmark(256);  /* L2 */
    checksum += cache_sensitive_benchmark(4096); /* L3/L2 large */
    goto final_benchmark;
    
haswell_path:
    /* Perform benchmarks matching Haswell cache sizes */
    checksum += cache_sensitive_benchmark(32);   /* L1 */
    checksum += cache_sensitive_benchmark(256);  /* L2 */
    checksum += cache_sensitive_benchmark(8192); /* L3 */
    goto final_benchmark;
    
final_benchmark:
    /* Additional cache-size specific benchmarks covering all switch cases */
    checksum += cache_sensitive_benchmark(8);     /* 0x0a */
    checksum += cache_sensitive_benchmark(16);    /* 0x0c, 0x0d */
    checksum += cache_sensitive_benchmark(24);    /* 0x0e */
    checksum += cache_sensitive_benchmark(128);   /* 0x39, 0x3b, 0x41 */
    checksum += cache_sensitive_benchmark(192);   /* 0x3a */
    checksum += cache_sensitive_benchmark(256);   /* 0x21, 0x3c, 0x42 */
    checksum += cache_sensitive_benchmark(384);   /* 0x3d */
    checksum += cache_sensitive_benchmark(512);   /* 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
    checksum += cache_sensitive_benchmark(1024);  /* 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
    checksum += cache_sensitive_benchmark(2048);  /* 0x45, 0x7d, 0x85 */
    checksum += cache_sensitive_benchmark(3072);  /* 0x48 */
    checksum += cache_sensitive_benchmark(4096);  /* 0x49 */
    checksum += cache_sensitive_benchmark(6144);  /* 0x4e */
    
    /* Print results including detected cache parameters */
    printf("Cache parameters detected:\n");
    printf("L1: %u KB, %u-way, %u-byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u-byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
