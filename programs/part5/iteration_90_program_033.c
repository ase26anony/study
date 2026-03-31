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
    int sizekb;    /* Size in KB */
    int assoc;     /* Associativity */
    int line;      /* Line size in bytes */
};

/* Cross-platform CPUID wrapper */
static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t *eax, uint32_t *ebx,
                  uint32_t *ecx, uint32_t *edx) {
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

/* Extract cache descriptor bytes from CPUID leaf 0x2 results */
static void extract_descriptors(uint32_t eax, uint32_t ebx,
                                uint32_t ecx, uint32_t edx,
                                uint8_t *descriptors, int *count) {
    uint8_t *regs = (uint8_t *)&eax;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0 && regs[i] != 0xFF) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t *)&ebx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0 && regs[i] != 0xFF) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t *)&ecx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0 && regs[i] != 0xFF) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t *)&edx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0 && regs[i] != 0xFF) {
            descriptors[(*count)++] = regs[i];
        }
    }
}

/* Determine if CPU is Xeon MP (simplified check) */
static int is_xeon_mp(uint32_t family, uint32_t model, uint32_t stepping) {
    /* Simplified: Check for family 0xF (Pentium 4/Xeon) and model >= 4 */
    /* This mimics the logic that would trigger the xeon_mp condition */
    return (family == 0xF && model >= 4);
}

int main() {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[64];
    int desc_count = 0;
    int xeon_mp = 0;
    
    /* Structures for L1 and L2 cache */
    struct cache_desc l1_cache = {0, 0, 0};
    struct cache_desc l2_cache = {0, 0, 0};
    
    /* Step 1: Get basic processor info (CPUID leaf 0x1) */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family/model/stepping for Xeon MP check */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    xeon_mp = is_xeon_mp(family, model, stepping);
    printf("CPU: Family=%u, Model=%u, Stepping=%u, Xeon_MP=%d\n",
           family, model, stepping, xeon_mp);
    
    /* Step 2: Get cache descriptors (CPUID leaf 0x2) */
    /* Call multiple times as per Intel spec */
    for (int call_num = 0; call_num < 3; call_num++) {
        cpuid(0x2, 0, &eax, &ebx, &ecx, &edx);
        
        /* Check if this is the first call and eax indicates valid data */
        if (call_num == 0 && (eax & 0xFF) == 0) {
            /* No valid cache descriptors */
            break;
        }
        
        /* Extract descriptors from registers */
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        /* Check if we have terminator byte */
        if (descriptors[desc_count - 1] == 0x00) {
            break;
        }
    }
    
    printf("Found %d cache descriptor bytes\n", desc_count);
    
    /* Step 3: Parse descriptors and populate cache structures */
    for (int i = 0; i < desc_count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0xFF) continue;
        
        printf("Processing descriptor: 0x%02x\n", desc);
        
        /* Exact switch cases from uncovered lines */
        switch (desc) {
            case 0x0a:
                l1_cache.sizekb = 8; l1_cache.assoc = 2; l1_cache.line = 32;
                printf("  -> L1: 8KB, 2-way, 32B line\n");
                break;
            case 0x0c:
                l1_cache.sizekb = 16; l1_cache.assoc = 4; l1_cache.line = 32;
                printf("  -> L1: 16KB, 4-way, 32B line\n");
                break;
            case 0x0d:
                l1_cache.sizekb = 16; l1_cache.assoc = 4; l1_cache.line = 64;
                printf("  -> L1: 16KB, 4-way, 64B line\n");
                break;
            case 0x0e:
                l1_cache.sizekb = 24; l1_cache.assoc = 6; l1_cache.line = 64;
                printf("  -> L1: 24KB, 6-way, 64B line\n");
                break;
            case 0x21:
                l2_cache.sizekb = 256; l2_cache.assoc = 8; l2_cache.line = 64;
                printf("  -> L2: 256KB, 8-way, 64B line\n");
                break;
            case 0x24:
                l2_cache.sizekb = 1024; l2_cache.assoc = 16; l2_cache.line = 64;
                printf("  -> L2: 1024KB, 16-way, 64B line\n");
                break;
            case 0x2c:
                l1_cache.sizekb = 32; l1_cache.assoc = 8; l1_cache.line = 64;
                printf("  -> L1: 32KB, 8-way, 64B line\n");
                break;
            case 0x39:
                l2_cache.sizekb = 128; l2_cache.assoc = 4; l2_cache.line = 64;
                printf("  -> L2: 128KB, 4-way, 64B line\n");
                break;
            case 0x3a:
                l2_cache.sizekb = 192; l2_cache.assoc = 6; l2_cache.line = 64;
                printf("  -> L2: 192KB, 6-way, 64B line\n");
                break;
            case 0x3b:
                l2_cache.sizekb = 128; l2_cache.assoc = 2; l2_cache.line = 64;
                printf("  -> L2: 128KB, 2-way, 64B line\n");
                break;
            case 0x3c:
                l2_cache.sizekb = 256; l2_cache.assoc = 4; l2_cache.line = 64;
                printf("  -> L2: 256KB, 4-way, 64B line\n");
                break;
            case 0x3d:
                l2_cache.sizekb = 384; l2_cache.assoc = 6; l2_cache.line = 64;
                printf("  -> L2: 384KB, 6-way, 64B line\n");
                break;
            case 0x3e:
                l2_cache.sizekb = 512; l2_cache.assoc = 4; l2_cache.line = 64;
                printf("  -> L2: 512KB, 4-way, 64B line\n");
                break;
            case 0x41:
                l2_cache.sizekb = 128; l2_cache.assoc = 4; l2_cache.line = 32;
                printf("  -> L2: 128KB, 4-way, 32B line\n");
                break;
            case 0x42:
                l2_cache.sizekb = 256; l2_cache.assoc = 4; l2_cache.line = 32;
                printf("  -> L2: 256KB, 4-way, 32B line\n");
                break;
            case 0x43:
                l2_cache.sizekb = 512; l2_cache.assoc = 4; l2_cache.line = 32;
                printf("  -> L2: 512KB, 4-way, 32B line\n");
                break;
            case 0x44:
                l2_cache.sizekb = 1024; l2_cache.assoc = 4; l2_cache.line = 32;
                printf("  -> L2: 1024KB, 4-way, 32B line\n");
                break;
            case 0x45:
                l2_cache.sizekb = 2048; l2_cache.assoc = 4; l2_cache.line = 32;
                printf("  -> L2: 2048KB, 4-way, 32B line\n");
                break;
            case 0x48:
                l2_cache.sizekb = 3072; l2_cache.assoc = 12; l2_cache.line = 64;
                printf("  -> L2: 3072KB, 12-way, 64B line\n");
                break;
            case 0x49:
                if (xeon_mp) {
                    printf("  -> L2: 0x49 skipped (Xeon MP detected)\n");
                    break;
                }
                l2_cache.sizekb = 4096; l2_cache.assoc = 16; l2_cache.line = 64;
                printf("  -> L2: 4096KB, 16-way, 64B line (non-Xeon MP)\n");
                break;
            case 0x4e:
                l2_cache.sizekb = 6144; l2_cache.assoc = 24; l2_cache.line = 64;
                printf("  -> L2: 6144KB, 24-way, 64B line\n");
                break;
            case 0x60:
                l1_cache.sizekb = 16; l1_cache.assoc = 8; l1_cache.line = 64;
                printf("  -> L1: 16KB, 8-way, 64B line\n");
                break;
            case 0x66:
                l1_cache.sizekb = 8; l1_cache.assoc = 4; l1_cache.line = 64;
                printf("  -> L1: 8KB, 4-way, 64B line\n");
                break;
            case 0x67:
                l1_cache.sizekb = 16; l1_cache.assoc = 4; l1_cache.line = 64;
                printf("  -> L1: 16KB, 4-way, 64B line\n");
                break;
            case 0x68:
                l1_cache.sizekb = 32; l1_cache.assoc = 4; l1_cache.line = 64;
                printf("  -> L1: 32KB, 4-way, 64B line\n");
                break;
            case 0x78:
                l2_cache.sizekb = 1024; l2_cache.assoc = 4; l2_cache.line = 64;
                printf("  -> L2: 1024KB, 4-way, 64B line\n");
                break;
            case 0x79:
                l2_cache.sizekb = 128; l2_cache.assoc = 8; l2_cache.line = 64;
                printf("  -> L2: 128KB, 8-way, 64B line\n");
                break;
            case 0x7a:
                l2_cache.sizekb = 256; l2_cache.assoc = 8; l2_cache.line = 64;
                printf("  -> L2: 256KB, 8-way, 64B line\n");
                break;
            case 0x7b:
                l2_cache.sizekb = 512; l2_cache.assoc = 8; l2_cache.line = 64;
                printf("  -> L2: 512KB, 8-way, 64B line\n");
                break;
            case 0x7c:
                l2_cache.sizekb = 1024; l2_cache.assoc = 8; l2_cache.line = 64;
                printf("  -> L2: 1024KB, 8-way, 64B line\n");
                break;
            case 0x7d:
                l2_cache.sizekb = 2048; l2_cache.assoc = 8; l2_cache.line = 64;
                printf("  -> L2: 2048KB, 8-way, 64B line\n");
                break;
            case 0x7f:
                l2_cache.sizekb = 512; l2_cache.assoc = 2; l2_cache.line = 64;
                printf("  -> L2: 512KB, 2-way, 64B line\n");
                break;
            case 0x80:
                l2_cache.sizekb = 512; l2_cache.assoc = 8; l2_cache.line = 64;
                printf("  -> L2: 512KB, 8-way, 64B line\n");
                break;
            case 0x82:
                l2_cache.sizekb = 256; l2_cache.assoc = 8; l2_cache.line = 32;
                printf("  -> L2: 256KB, 8-way, 32B line\n");
                break;
            case 0x83:
                l2_cache.sizekb = 512; l2_cache.assoc = 8; l2_cache.line = 32;
                printf("  -> L2: 512KB, 8-way, 32B line\n");
                break;
            case 0x84:
                l2_cache.sizekb = 1024; l2_cache.assoc = 8; l2_cache.line = 32;
                printf("  -> L2: 1024KB, 8-way, 32B line\n");
                break;
            case 0x85:
                l2_cache.sizekb = 2048; l2_cache.assoc = 8; l2_cache.line = 32;
                printf("  -> L2: 2048KB, 8-way, 32B line\n");
                break;
            case 0x86:
                l2_cache.sizekb = 512; l2_cache.assoc = 4; l2_cache.line = 64;
                printf("  -> L2: 512KB, 4-way, 64B line\n");
                break;
            case 0x87:
                l2_cache.sizekb = 1024; l2_cache.assoc = 8; l2_cache.line = 64;
                printf("  -> L2: 1024KB, 8-way, 64B line\n");
                break;
            default:
                printf("  -> Unknown descriptor 0x%02x\n", desc);
                break;
        }
    }
    
    /* Step 4: Print final cache information */
    printf("\n=== Cache Information ===\n");
    printf("L1 Cache: %dKB, %d-way, %dB line\n",
           l1_cache.sizekb, l1_cache.assoc, l1_cache.line);
    printf("L2 Cache: %dKB, %d-way, %dB line\n",
           l2_cache.sizekb, l2_cache.assoc, l2_cache.line);
    
    /* Step 5: Use cache line size in computation to prevent optimization */
    int cache_line = l1_cache.line > 0 ? l1_cache.line : 64;
    size_t array_size = cache_line * 1024;
    volatile char *buffer = (volatile char *)malloc(array_size);
    
    if (buffer) {
        /* Aligned access using cache line size */
        for (size_t i = 0; i < array_size; i += cache_line) {
            buffer[i] = (char)(i % 256);
        }
        
        /* Compute checksum to ensure work is done */
        volatile char checksum = 0;
        for (size_t i = 0; i < array_size; i += cache_line) {
            checksum ^= buffer[i];
        }
        
        printf("Cache-aware computation complete (checksum: %d)\n", checksum);
        free((void *)buffer);
    }
    
    return 0;
}
