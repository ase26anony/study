#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsics */
#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* CPUID wrapper for cross-platform compatibility */
static void cpuid(uint32_t leaf, uint32_t subleaf, 
                  uint32_t* eax, uint32_t* ebx, 
                  uint32_t* ecx, uint32_t* edx) {
#ifdef _WIN32
    int regs[4];
    __cpuidex(regs, leaf, subleaf);
    *eax = regs[0];
    *ebx = regs[1];
    *ecx = regs[2];
    *edx = regs[3];
#else
    __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
#endif
}

/* Check if CPU is Xeon MP (simplified logic) */
static int is_xeon_mp(uint32_t family, uint32_t model, uint32_t stepping) {
    /* Simplified check: Xeon MP typically has specific family/model combinations
       This mimics the logic that would be in driver-i386.cc */
    if (family == 0xF && model >= 0x6) {
        /* Family 15h (Pentium 4 Xeon MP) */
        return 1;
    }
    /* Add more specific checks as needed */
    return 0;
}

int main() {
    uint32_t eax, ebx, ecx, edx;
    uint32_t family, model, stepping;
    int xeon_mp = 0;
    
    /* Cache structures matching driver-i386.cc */
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    /* Step 1: Get basic CPU info (CPUID leaf 0x1) */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family/model/stepping for Xeon MP check */
    family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
    model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    stepping = eax & 0xF;
    
    xeon_mp = is_xeon_mp(family, model, stepping);
    
    printf("CPU Info: Family=%u, Model=%u, Stepping=%u, Xeon_MP=%d\n",
           family, model, stepping, xeon_mp);
    
    /* Step 2: Get cache descriptors (CPUID leaf 0x2) */
    /* According to Intel spec, CPUID leaf 2 may need multiple calls */
    int iterations = 0;
    uint8_t descriptors[32]; /* Enough for all descriptors */
    int desc_count = 0;
    
    /* First call to get initial descriptors */
    cpuid(0x2, 0, &eax, &ebx, &ecx, &edx);
    
    /* AL register contains number of iterations needed */
    int num_iterations = eax & 0xFF;
    
    /* Process first set of descriptors */
    uint8_t* reg_bytes = (uint8_t*)&eax;
    for (int i = 1; i < 4; i++) { /* Skip AL (iteration count) */
        if (reg_bytes[i] != 0x00 && reg_bytes[i] != 0xFF) {
            descriptors[desc_count++] = reg_bytes[i];
        }
    }
    
    reg_bytes = (uint8_t*)&ebx;
    for (int i = 0; i < 4; i++) {
        if (reg_bytes[i] != 0x00 && reg_bytes[i] != 0xFF) {
            descriptors[desc_count++] = reg_bytes[i];
        }
    }
    
    reg_bytes = (uint8_t*)&ecx;
    for (int i = 0; i < 4; i++) {
        if (reg_bytes[i] != 0x00 && reg_bytes[i] != 0xFF) {
            descriptors[desc_count++] = reg_bytes[i];
        }
    }
    
    reg_bytes = (uint8_t*)&edx;
    for (int i = 0; i < 4; i++) {
        if (reg_bytes[i] != 0x00 && reg_bytes[i] != 0xFF) {
            descriptors[desc_count++] = reg_bytes[i];
        }
    }
    
    /* Additional iterations if needed (simplified) */
    if (num_iterations > 1) {
        for (int iter = 1; iter < num_iterations; iter++) {
            cpuid(0x2, iter, &eax, &ebx, &ecx, &edx);
            
            /* Process additional descriptor bytes */
            uint8_t* bytes[] = {(uint8_t*)&eax, (uint8_t*)&ebx, 
                               (uint8_t*)&ecx, (uint8_t*)&edx};
            
            for (int reg = 0; reg < 4; reg++) {
                for (int i = 0; i < 4; i++) {
                    if (bytes[reg][i] != 0x00 && bytes[reg][i] != 0xFF) {
                        if (desc_count < 32) {
                            descriptors[desc_count++] = bytes[reg][i];
                        }
                    }
                }
            }
        }
    }
    
    printf("Found %d cache descriptor bytes\n", desc_count);
    
    /* Step 3: Parse descriptors using exact switch cases from uncovered lines */
    for (int i = 0; i < desc_count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Exact switch cases from driver-i386.cc lines 127-244 */
        switch (desc) {
            case 0x0a:
                level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
                printf("Descriptor 0x0a: L1 Cache - 8KB, 2-way, 32B line\n");
                break;
            case 0x0c:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
                printf("Descriptor 0x0c: L1 Cache - 16KB, 4-way, 32B line\n");
                break;
            case 0x0d:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
                printf("Descriptor 0x0d: L1 Cache - 16KB, 4-way, 64B line\n");
                break;
            case 0x0e:
                level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
                printf("Descriptor 0x0e: L1 Cache - 24KB, 6-way, 64B line\n");
                break;
            case 0x21:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
                printf("Descriptor 0x21: L2 Cache - 256KB, 8-way, 64B line\n");
                break;
            case 0x24:
                level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
                printf("Descriptor 0x24: L2 Cache - 1024KB, 16-way, 64B line\n");
                break;
            case 0x2c:
                level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
                printf("Descriptor 0x2c: L1 Cache - 32KB, 8-way, 64B line\n");
                break;
            case 0x39:
                level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
                printf("Descriptor 0x39: L2 Cache - 128KB, 4-way, 64B line\n");
                break;
            case 0x3a:
                level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
                printf("Descriptor 0x3a: L2 Cache - 192KB, 6-way, 64B line\n");
                break;
            case 0x3b:
                level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
                printf("Descriptor 0x3b: L2 Cache - 128KB, 2-way, 64B line\n");
                break;
            case 0x3c:
                level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
                printf("Descriptor 0x3c: L2 Cache - 256KB, 4-way, 64B line\n");
                break;
            case 0x3d:
                level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
                printf("Descriptor 0x3d: L2 Cache - 384KB, 6-way, 64B line\n");
                break;
            case 0x3e:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
                printf("Descriptor 0x3e: L2 Cache - 512KB, 4-way, 64B line\n");
                break;
            case 0x41:
                level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
                printf("Descriptor 0x41: L2 Cache - 128KB, 4-way, 32B line\n");
                break;
            case 0x42:
                level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
                printf("Descriptor 0x42: L2 Cache - 256KB, 4-way, 32B line\n");
                break;
            case 0x43:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
                printf("Descriptor 0x43: L2 Cache - 512KB, 4-way, 32B line\n");
                break;
            case 0x44:
                level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
                printf("Descriptor 0x44: L2 Cache - 1024KB, 4-way, 32B line\n");
                break;
            case 0x45:
                level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
                printf("Descriptor 0x45: L2 Cache - 2048KB, 4-way, 32B line\n");
                break;
            case 0x48:
                level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
                printf("Descriptor 0x48: L2 Cache - 3072KB, 12-way, 64B line\n");
                break;
            case 0x49:
                if (xeon_mp) {
                    printf("Descriptor 0x49: Skipped due to Xeon MP\n");
                    break;
                }
                level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
                printf("Descriptor 0x49: L2 Cache - 4096KB, 16-way, 64B line\n");
                break;
            case 0x4e:
                level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
                printf("Descriptor 0x4e: L2 Cache - 6144KB, 24-way, 64B line\n");
                break;
            case 0x60:
                level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
                printf("Descriptor 0x60: L1 Cache - 16KB, 8-way, 64B line\n");
                break;
            case 0x66:
                level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
                printf("Descriptor 0x66: L1 Cache - 8KB, 4-way, 64B line\n");
                break;
            case 0x67:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
                printf("Descriptor 0x67: L1 Cache - 16KB, 4-way, 64B line\n");
                break;
            case 0x68:
                level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
                printf("Descriptor 0x68: L1 Cache - 32KB, 4-way, 64B line\n");
                break;
            case 0x78:
                level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
                printf("Descriptor 0x78: L2 Cache - 1024KB, 4-way, 64B line\n");
                break;
            case 0x79:
                level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
                printf("Descriptor 0x79: L2 Cache - 128KB, 8-way, 64B line\n");
                break;
            case 0x7a:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
                printf("Descriptor 0x7a: L2 Cache - 256KB, 8-way, 64B line\n");
                break;
            case 0x7b:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
                printf("Descriptor 0x7b: L2 Cache - 512KB, 8-way, 64B line\n");
                break;
            case 0x7c:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
                printf("Descriptor 0x7c: L2 Cache - 1024KB, 8-way, 64B line\n");
                break;
            case 0x7d:
                level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
                printf("Descriptor 0x7d: L2 Cache - 2048KB, 8-way, 64B line\n");
                break;
            case 0x7f:
                level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
                printf("Descriptor 0x7f: L2 Cache - 512KB, 2-way, 64B line\n");
                break;
            case 0x80:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
                printf("Descriptor 0x80: L2 Cache - 512KB, 8-way, 64B line\n");
                break;
            case 0x82:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
                printf("Descriptor 0x82: L2 Cache - 256KB, 8-way, 32B line\n");
                break;
            case 0x83:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
                printf("Descriptor 0x83: L2 Cache - 512KB, 8-way, 32B line\n");
                break;
            case 0x84:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
                printf("Descriptor 0x84: L2 Cache - 1024KB, 8-way, 32B line\n");
                break;
            case 0x85:
                level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
                printf("Descriptor 0x85: L2 Cache - 2048KB, 8-way, 32B line\n");
                break;
            case 0x86:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
                printf("Descriptor 0x86: L2 Cache - 512KB, 4-way, 64B line\n");
                break;
            case 0x87:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
                printf("Descriptor 0x87: L2 Cache - 1024KB, 8-way, 64B line\n");
                break;
            default:
                /* Other descriptors not in uncovered lines */
                if (desc != 0x00 && desc != 0xFF) {
                    printf("Descriptor 0x%02x: Not in uncovered cases\n", desc);
                }
                break;
        }
    }
    
    /* Step 4: Print final cache information */
    printf("\nFinal Cache Configuration:\n");
    printf("L1 Cache: %dKB, %d-way, %dB line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache: %dKB, %d-way, %dB line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    /* Step 5: Use cache line size for computation (prevent optimization) */
    int cache_line = level1.line > 0 ? level1.line : 64;
    volatile int* aligned_array;
    
    /* Allocate aligned memory based on cache line */
    #ifdef _WIN32
        aligned_array = _aligned_malloc(cache_line * 16, cache_line);
    #else
        posix_memalign((void**)&aligned_array, cache_line, cache_line * 16);
    #endif
    
    if (aligned_array) {
        /* Perform cache-aware computation */
        for (int i = 0; i < 16; i++) {
            aligned_array[i * (cache_line / sizeof(int))] = i;
        }
        
        int sum = 0;
        for (int i = 0; i < 16; i++) {
            sum += aligned_array[i * (cache_line / sizeof(int))];
        }
        
        printf("Cache-aware computation result: %d\n", sum);
        
        #ifdef _WIN32
            _aligned_free(aligned_array);
        #else
            free(aligned_array);
        #endif
    }
    
    return 0;
}
