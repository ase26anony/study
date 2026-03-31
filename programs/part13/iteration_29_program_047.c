#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Structures matching driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Simulated CPUID results for leaf 0x02 */
static const uint8_t descriptor_bytes[] = {
    /* First set of level1 cache descriptors */
    0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x60, 0x66, 0x67, 0x68,
    
    /* Level2 cache descriptors */
    0x21, 0x24, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49, 0x4e,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7f, 0x80,
    0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    
    /* Terminator */
    0x00
};

/* Function to parse cache descriptors - replicates the uncovered switch logic */
void parse_cache_descriptor(uint8_t desc, struct cache_desc *level1, 
                           struct cache_desc *level2, int *xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("L1: 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("L1: 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1: 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("L1: 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2: 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("L2: 1MB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("L1: 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("L2: 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("L2: 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("L2: 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("L2: 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("L2: 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2: 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("L2: 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("L2: 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("L2: 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("L2: 1MB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("L2: 2MB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("L2: 3MB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (*xeon_mp) {
                printf("L2: Xeon MP detected, skipping 0x49\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("L2: 4MB, 16-way, 64B line (non-Xeon-MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("L2: 6MB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("L1: 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("L1: 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1: 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("L1: 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("L2: 1MB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("L2: 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2: 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2: 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2: 1MB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("L2: 2MB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("L2: 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2: 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("L2: 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("L2: 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("L2: 1MB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("L2: 2MB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2: 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2: 1MB, 8-way, 64B line\n");
            break;
        default:
            if (desc != 0x00 && desc != 0xff) {
                printf("Unknown descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Simulate CPUID leaf 0x02 call with descriptor table */
void simulate_cpuid_leaf2(void) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int xeon_mp = 0;  /* Set to 0 to trigger case 0x49 branch */
    
    printf("=== Simulating CPUID Leaf 0x02 Cache Descriptor Parsing ===\n");
    
    /* Simulate AL = number of valid descriptor bytes (excluding AL itself) */
    /* We'll use 30 descriptors to ensure we go through the table parsing loop */
    uint8_t al_value = sizeof(descriptor_bytes) / sizeof(descriptor_bytes[0]);
    
    printf("CPUID Leaf 0x02 AL = 0x%02x (%d descriptor bytes)\n", 
           al_value, al_value);
    
    /* Process each descriptor byte */
    for (int i = 0; i < sizeof(descriptor_bytes) / sizeof(descriptor_bytes[0]); i++) {
        parse_cache_descriptor(descriptor_bytes[i], &level1, &level2, &xeon_mp);
    }
    
    printf("\nFinal cache configuration:\n");
    printf("L1: %dKB, %d-way, %dB line\n", level1.sizekb, level1.assoc, level1.line);
    printf("L2: %dKB, %d-way, %dB line\n", level2.sizekb, level2.assoc, level2.line);
}

/* Real CPUID implementation for comparison */
#ifdef __cpuid
void real_cpuid_test(void) {
    unsigned int eax, ebx, ecx, edx;
    
    printf("\n=== Real CPUID Leaf 0x02 Test ===\n");
    
    /* Try to get real CPUID leaf 0x02 */
    __cpuid(0x02, eax, ebx, ecx, edx);
    
    printf("EAX: 0x%08x\n", eax);
    printf("EBX: 0x%08x\n", ebx);
    printf("ECX: 0x%08x\n", ecx);
    printf("EDX: 0x%08x\n", edx);
    
    /* Check if AL > 1 (valid descriptor table) */
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        printf("Valid descriptor table with %d bytes\n", al);
        
        /* Extract descriptor bytes */
        uint8_t *bytes = (uint8_t*)&eax;
        for (int i = 1; i < 4 && i < al; i++) {  /* Start from 1 to skip AL */
            if (bytes[i] != 0x00 && bytes[i] != 0xff) {
                printf("  Descriptor 0x%02x\n", bytes[i]);
            }
        }
        
        bytes = (uint8_t*)&ebx;
        for (int i = 0; i < 4 && (i + 4) < al; i++) {
            if (bytes[i] != 0x00 && bytes[i] != 0xff) {
                printf("  Descriptor 0x%02x\n", bytes[i]);
            }
        }
        
        bytes = (uint8_t*)&ecx;
        for (int i = 0; i < 4 && (i + 8) < al; i++) {
            if (bytes[i] != 0x00 && bytes[i] != 0xff) {
                printf("  Descriptor 0x%02x\n", bytes[i]);
            }
        }
        
        bytes = (uint8_t*)&edx;
        for (int i = 0; i < 4 && (i + 12) < al; i++) {
            if (bytes[i] != 0x00 && bytes[i] != 0xff) {
                printf("  Descriptor 0x%02x\n", bytes[i]);
            }
        }
    } else if (al == 1) {
        printf("CPUID leaf 0x02 returns cache information in TLB format\n");
    } else {
        printf("CPUID leaf 0x02 not supported\n");
    }
}
#endif

int main(void) {
    printf("Cache Descriptor Test Program\n");
    printf("=============================\n\n");
    
    /* Method 1: Simulate all target descriptor values */
    simulate_cpuid_leaf2();
    
    /* Method 2: Try real CPUID if available */
#ifdef __cpuid
    real_cpuid_test();
#else
    printf("\nCPUID intrinsics not available on this platform\n");
#endif
    
    /* Additional test for Xeon MP conditional */
    printf("\n=== Testing Xeon MP Conditional (case 0x49) ===\n");
    
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    /* Test with xeon_mp = 0 (should set L2 to 4MB) */
    int xeon_mp = 0;
    parse_cache_descriptor(0x49, &level1, &level2, &xeon_mp);
    
    /* Test with xeon_mp = 1 (should skip setting) */
    xeon_mp = 1;
    level2.sizekb = 0;  /* Reset */
    parse_cache_descriptor(0x49, &level1, &level2, &xeon_mp);
    printf("After xeon_mp=1, L2 size remains: %dKB\n", level2.sizekb);
    
    return 0;
}
