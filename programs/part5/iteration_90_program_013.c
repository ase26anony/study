/*
 * This program is designed to trigger the specific CPUID cache detection logic
 * in driver-i386.cc lines 127-244 by executing CPUID leaf 0x2 and processing
 * cache descriptor bytes matching the uncovered cases.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#include <windows.h>
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int level;      /* Cache level (1 or 2) */
    int type;       /* Cache type */
};

/* CPUID wrapper function */
static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t *eax, uint32_t *ebx,
                  uint32_t *ecx, uint32_t *edx) {
#if defined(_WIN32) || defined(_WIN64)
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

/* Extract cache descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx,
                                uint32_t ecx, uint32_t edx,
                                uint8_t *descriptors, int *count) {
    int i = 0;
    
    /* According to Intel manual, bytes are returned in AL, BL, CL, DL */
    /* But we need to process all non-zero bytes from all registers */
    
    /* Process EAX (excluding the first byte which indicates number of calls) */
    uint8_t *p = (uint8_t *)&eax;
    for (int j = 1; j < 4; j++) {  /* Skip byte 0 */
        if (p[j] != 0x00) {
            descriptors[i++] = p[j];
        }
    }
    
    /* Process EBX */
    p = (uint8_t *)&ebx;
    for (int j = 0; j < 4; j++) {
        if (p[j] != 0x00) {
            descriptors[i++] = p[j];
        }
    }
    
    /* Process ECX */
    p = (uint8_t *)&ecx;
    for (int j = 0; j < 4; j++) {
        if (p[j] != 0x00) {
            descriptors[i++] = p[j];
        }
    }
    
    /* Process EDX */
    p = (uint8_t *)&edx;
    for (int j = 0; j < 4; j++) {
        if (p[j] != 0x00) {
            descriptors[i++] = p[j];
        }
    }
    
    *count = i;
}

/* Check if CPU is Xeon MP (for case 0x49) */
static int is_xeon_mp(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU signature */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0x0F) | ((eax >> 16) & 0xF0);
    uint32_t model = ((eax >> 4) & 0x0F) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0x0F;
    
    /* Simplified check: Xeon MP typically has specific family/model combinations */
    /* This is a simplified check - real implementation would be more complex */
    if (family == 0x0F && model >= 0x04) {  /* Pentium 4 Xeon MP */
        return 1;
    }
    
    /* Check brand string for "Xeon" and "MP" */
    char brand[49] = {0};
    for (int i = 0; i < 3; i++) {
        cpuid(0x80000002 + i, 0,
              (uint32_t*)&brand[i * 16],
              (uint32_t*)&brand[i * 16 + 4],
              (uint32_t*)&brand[i * 16 + 8],
              (uint32_t*)&brand[i * 16 + 12]);
    }
    
    /* Check for "Xeon" and "MP" in brand string */
    if (strstr(brand, "Xeon") && strstr(brand, "MP")) {
        return 1;
    }
    
    return 0;
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1,
                               struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("  Cache descriptor 0x%02x: L1 8KB, 2-way, 32B line\n", desc);
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("  Cache descriptor 0x%02x: L1 16KB, 4-way, 32B line\n", desc);
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 16KB, 4-way, 64B line\n", desc);
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 24KB, 6-way, 64B line\n", desc);
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 256KB, 8-way, 64B line\n", desc);
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1024KB, 16-way, 64B line\n", desc);
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 32KB, 8-way, 64B line\n", desc);
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 128KB, 4-way, 64B line\n", desc);
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 192KB, 6-way, 64B line\n", desc);
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 128KB, 2-way, 64B line\n", desc);
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 256KB, 4-way, 64B line\n", desc);
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 384KB, 6-way, 64B line\n", desc);
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 4-way, 64B line\n", desc);
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 128KB, 4-way, 32B line\n", desc);
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 256KB, 4-way, 32B line\n", desc);
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 512KB, 4-way, 32B line\n", desc);
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 1024KB, 4-way, 32B line\n", desc);
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 2048KB, 4-way, 32B line\n", desc);
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 3072KB, 12-way, 64B line\n", desc);
            break;
        case 0x49:
            if (xeon_mp) {
                printf("  Cache descriptor 0x%02x: Skipped (Xeon MP detected)\n", desc);
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 4096KB, 16-way, 64B line\n", desc);
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 6144KB, 24-way, 64B line\n", desc);
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 16KB, 8-way, 64B line\n", desc);
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 8KB, 4-way, 64B line\n", desc);
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 16KB, 4-way, 64B line\n", desc);
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 32KB, 4-way, 64B line\n", desc);
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1024KB, 4-way, 64B line\n", desc);
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 128KB, 8-way, 64B line\n", desc);
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 256KB, 8-way, 64B line\n", desc);
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 8-way, 64B line\n", desc);
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1024KB, 8-way, 64B line\n", desc);
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 2048KB, 8-way, 64B line\n", desc);
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 2-way, 64B line\n", desc);
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 8-way, 64B line\n", desc);
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 256KB, 8-way, 32B line\n", desc);
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 512KB, 8-way, 32B line\n", desc);
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 1024KB, 8-way, 32B line\n", desc);
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 2048KB, 8-way, 32B line\n", desc);
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 4-way, 64B line\n", desc);
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1024KB, 8-way, 64B line\n", desc);
            break;
        default:
            /* Ignore other descriptors */
            break;
    }
}

/* Simple computation using cache line size to prevent optimization */
static void cache_line_optimization_test(int line_size) {
    volatile int result = 0;
    const int array_size = 1024;
    int *array = (int*)malloc(array_size * sizeof(int));
    
    if (array) {
        /* Align access to cache line boundary */
        int aligned_index = (array_size / 2) & ~(line_size / sizeof(int) - 1);
        
        /* Perform some computation */
        for (int i = 0; i < 100; i++) {
            array[aligned_index] = i;
            result += array[aligned_index];
        }
        
        free(array);
    }
    
    /* Use result to prevent dead code elimination */
    if (result > 0) {
        printf("Cache line optimization test completed (line size: %d bytes)\n", line_size);
    }
}

int main(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[256];
    int desc_count;
    int xeon_mp;
    
    /* Initialize cache structures */
    struct cache_desc level1 = {0, 0, 0, 1, 0};
    struct cache_desc level2 = {0, 0, 0, 2, 0};
    
    printf("Starting CPUID cache detection test...\n");
    
    /* Step 1: Check Xeon MP status */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("\nProcessing CPUID leaf 0x2 cache descriptors:\n");
    
    /* According to Intel manual, CPUID leaf 0x2 may need to be called multiple times */
    /* The first byte of EAX indicates the number of times to call */
    cpuid(2, 0, &eax, &ebx, &ecx, &edx);
    
    int calls_needed = eax & 0xFF;
    printf("CPUID leaf 0x2 needs %d call(s)\n", calls_needed);
    
    /* Process all calls */
    for (int call = 0; call < calls_needed; call++) {
        cpuid(2, call, &eax, &ebx, &ecx, &edx);
        
        /* Extract descriptors from this call */
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        printf("Call %d: Found %d descriptor(s)\n", call + 1, desc_count);
        
        /* Process each descriptor */
        for (int i = 0; i < desc_count; i++) {
            process_descriptor(descriptors[i], &level1, &level2, xeon_mp);
        }
    }
    
    /* Step 3: Print collected cache information */
    printf("\n=== Cache Information Summary ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %d KB, %d-way, %d byte line\n",
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way, %d byte line\n",
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Step 4: Perform cache line optimization test */
    printf("\nPerforming cache line optimization test...\n");
    int test_line_size = level1.line > 0 ? level1.line : 64; /* Default to 64 if not detected */
    cache_line_optimization_test(test_line_size);
    
    printf("\nTest completed successfully.\n");
    return 0;
}
