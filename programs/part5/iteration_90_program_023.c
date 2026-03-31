/*
 * This program is designed to trigger the specific cache descriptor parsing logic
 * in driver-i386.cc lines 127-244 by executing CPUID leaf 0x2 and processing
 * the returned cache descriptor bytes.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
};

/* Execute CPUID with given leaf and subleaf */
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

/* Check if CPU is Xeon MP (simplified detection) */
static int is_xeon_mp(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU family/model/stepping from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family and model */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 16) & 0xFF0);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    
    /* Simplified Xeon MP detection:
     * Family 0xF (Pentium 4/Xeon), Model 0x3, 0x4, or stepping-based */
    if (family == 0xF) {
        /* Check for Xeon MP specific models */
        if (model == 0x3 || model == 0x4) {
            /* Additional check: Xeon MP has specific feature bits */
            cpuid(0x0, 0, &eax, &ebx, &ecx, &edx);
            char vendor[13];
            memcpy(vendor, &ebx, 4);
            memcpy(vendor + 4, &edx, 4);
            memcpy(vendor + 8, &ecx, 4);
            vendor[12] = '\0';
            
            /* Check for "GenuineIntel" and MP capability */
            if (strcmp(vendor, "GenuineIntel") == 0) {
                /* Check for MP feature in leaf 0x1 edx bit 19 (CLFLUSH) */
                cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
                if (edx & (1 << 19)) {
                    return 1; /* Likely Xeon MP */
                }
            }
        }
    }
    
    return 0;
}

/* Parse cache descriptor byte and populate cache_desc structure */
static void parse_cache_descriptor(uint8_t desc, struct cache_desc *level1,
                                   struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("  L1 Cache: 8KB, 2-way, 32-byte line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("  L1 Cache: 16KB, 4-way, 32-byte line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  L1 Cache: 16KB, 4-way, 64-byte line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("  L1 Cache: 24KB, 6-way, 64-byte line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("  L2 Cache: 1024KB, 16-way, 64-byte line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("  L1 Cache: 32KB, 8-way, 64-byte line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 128KB, 4-way, 64-byte line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("  L2 Cache: 192KB, 6-way, 64-byte line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("  L2 Cache: 128KB, 2-way, 64-byte line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 256KB, 4-way, 64-byte line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("  L2 Cache: 384KB, 6-way, 64-byte line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 512KB, 4-way, 64-byte line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 128KB, 4-way, 32-byte line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 256KB, 4-way, 32-byte line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 512KB, 4-way, 32-byte line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 1024KB, 4-way, 32-byte line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 2048KB, 4-way, 32-byte line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("  L2 Cache: 3072KB, 12-way, 64-byte line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("  Descriptor 0x49: Xeon MP detected, skipping L2 cache\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("  L2 Cache: 4096KB, 16-way, 64-byte line (non-Xeon MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("  L2 Cache: 6144KB, 24-way, 64-byte line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("  L1 Cache: 16KB, 8-way, 64-byte line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("  L1 Cache: 8KB, 4-way, 64-byte line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  L1 Cache: 16KB, 4-way, 64-byte line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("  L1 Cache: 32KB, 4-way, 64-byte line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 1024KB, 4-way, 64-byte line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 128KB, 8-way, 64-byte line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 512KB, 8-way, 64-byte line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 1024KB, 8-way, 64-byte line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 2048KB, 8-way, 64-byte line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("  L2 Cache: 512KB, 2-way, 64-byte line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 512KB, 8-way, 64-byte line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("  L2 Cache: 256KB, 8-way, 32-byte line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("  L2 Cache: 512KB, 8-way, 32-byte line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("  L2 Cache: 1024KB, 8-way, 32-byte line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("  L2 Cache: 2048KB, 8-way, 32-byte line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 512KB, 4-way, 64-byte line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 1024KB, 8-way, 64-byte line\n");
            break;
        default:
            /* Ignore other descriptors as per Intel spec */
            if (desc != 0x00 && (desc & 0x80) == 0) {
                printf("  Unknown cache descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Demonstrate cache line size usage to prevent optimization */
static void use_cache_line(int line_size) {
    volatile char buffer[4096];
    size_t aligned_offset = (size_t)buffer & (line_size - 1);
    if (aligned_offset != 0) {
        buffer[0] = 1; /* Force memory access */
    }
    buffer[line_size] = 2; /* Access at cache line boundary */
}

int main(void) {
    uint32_t eax, ebx, ecx, edx;
    struct cache_desc l1_cache = {0, 0, 0};
    struct cache_desc l2_cache = {0, 0, 0};
    int xeon_mp = 0;
    
    printf("Starting CPU cache detection...\n");
    
    /* Step 1: Check if CPU is Xeon MP */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("\nProcessing CPUID leaf 0x2 cache descriptors:\n");
    
    int iterations = 0;
    uint8_t descriptors[32]; /* Buffer for descriptor bytes */
    int desc_count = 0;
    
    /* According to Intel spec, CPUID leaf 0x2 may need to be called multiple times */
    for (iterations = 0; iterations < 16; iterations++) {
        cpuid(0x2, iterations, &eax, &ebx, &ecx, &edx);
        
        /* Extract descriptor bytes from registers */
        uint8_t *reg_bytes = (uint8_t *)&eax;
        for (int i = 0; i < 4; i++) {
            if (reg_bytes[i] != 0x00) {
                descriptors[desc_count++] = reg_bytes[i];
            }
        }
        
        reg_bytes = (uint8_t *)&ebx;
        for (int i = 0; i < 4; i++) {
            if (reg_bytes[i] != 0x00) {
                descriptors[desc_count++] = reg_bytes[i];
            }
        }
        
        reg_bytes = (uint8_t *)&ecx;
        for (int i = 0; i < 4; i++) {
            if (reg_bytes[i] != 0x00) {
                descriptors[desc_count++] = reg_bytes[i];
            }
        }
        
        reg_bytes = (uint8_t *)&edx;
        for (int i = 0; i < 4; i++) {
            if (reg_bytes[i] != 0x00) {
                descriptors[desc_count++] = reg_bytes[i];
            }
        }
        
        /* Check for terminator byte in EAX[7:0] */
        if ((eax & 0xFF) == 0x00) {
            break;
        }
    }
    
    /* Step 3: Parse all collected descriptors */
    printf("Found %d cache descriptor bytes\n", desc_count);
    for (int i = 0; i < desc_count; i++) {
        printf("Descriptor %d: 0x%02x\n", i, descriptors[i]);
        parse_cache_descriptor(descriptors[i], &l1_cache, &l2_cache, xeon_mp);
    }
    
    /* Step 4: Print final cache information */
    printf("\nFinal Cache Configuration:\n");
    if (l1_cache.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %d-byte line\n",
               l1_cache.sizekb, l1_cache.assoc, l1_cache.line);
    }
    if (l2_cache.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %d-byte line\n",
               l2_cache.sizekb, l2_cache.assoc, l2_cache.line);
    }
    
    /* Step 5: Use cache line size to prevent optimization */
    int line_size = l1_cache.line > 0 ? l1_cache.line : 
                   (l2_cache.line > 0 ? l2_cache.line : 64);
    printf("\nUsing cache line size %d for memory access pattern\n", line_size);
    use_cache_line(line_size);
    
    /* Additional CPUID calls to increase coverage probability */
    printf("\nPerforming additional CPUID calls...\n");
    for (uint32_t leaf = 0x0; leaf <= 0x4; leaf++) {
        cpuid(leaf, 0, &eax, &ebx, &ecx, &edx);
        printf("Leaf 0x%02x: EAX=0x%08x EBX=0x%08x ECX=0x%08x EDX=0x%08x\n",
               leaf, eax, ebx, ecx, edx);
    }
    
    return 0;
}
