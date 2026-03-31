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
    int level;     /* Cache level (1 or 2) */
    int type;      /* Cache type */
};

/* CPUID wrapper for cross-platform compatibility */
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

/* Check if CPU is Xeon MP based on family/model/stepping */
static int is_xeon_mp(uint32_t eax_from_cpuid1) {
    uint32_t family = (eax_from_cpuid1 >> 8) & 0xF;
    uint32_t model = (eax_from_cpuid1 >> 4) & 0xF;
    uint32_t extended_family = (eax_from_cpuid1 >> 20) & 0xFF;
    uint32_t extended_model = (eax_from_cpuid1 >> 16) & 0xF;
    
    /* Combine extended and base family/model */
    uint32_t full_family = family;
    if (family == 0xF) {
        full_family += extended_family;
    }
    
    uint32_t full_model = model;
    if (family == 0xF || family == 0x6) {
        full_model |= (extended_model << 4);
    }
    
    /* Check for Xeon MP (specific family/model combinations) */
    /* This is simplified - actual detection would be more complex */
    if (full_family == 0xF && full_model >= 0x2) {
        return 1; /* Likely Xeon MP */
    }
    
    return 0;
}

/* Process cache descriptor byte according to uncovered logic */
static void process_cache_descriptor(uint8_t desc, struct cache_desc *level1, 
                                     struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->level = 1; level1->type = desc;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->level = 1; level1->type = desc;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x49:
            if (xeon_mp) {
                /* Break as in uncovered code - don't populate */
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        default:
            /* Not one of our target descriptors */
            break;
    }
}

/* Print cache information */
static void print_cache_info(const struct cache_desc *cache, const char *name) {
    if (cache->sizekb > 0) {
        printf("%s Cache: %d KB, %d-way, %d byte lines (Descriptor: 0x%02x)\n",
               name, cache->sizekb, cache->assoc, cache->line, cache->type);
    }
}

/* Use cache line size for computation to prevent optimization */
static void cache_line_computation(int line_size) {
    volatile char *buffer = (volatile char*)malloc(4096);
    if (!buffer) return;
    
    /* Align access to cache line boundary */
    size_t aligned_offset = (4096 - (line_size * 2)) & ~(line_size - 1);
    
    /* Perform operations that use cache line size */
    for (int i = 0; i < line_size; i++) {
        buffer[aligned_offset + i] = (char)(i & 0xFF);
    }
    
    volatile char sum = 0;
    for (int i = 0; i < line_size; i++) {
        sum += buffer[aligned_offset + i];
    }
    
    /* Use sum to prevent dead code elimination */
    if (sum == 0) {
        printf("Cache line computation completed (line size: %d)\n", line_size);
    }
    
    free((void*)buffer);
}

int main() {
    uint32_t eax, ebx, ecx, edx;
    struct cache_desc level1 = {0};
    struct cache_desc level2 = {0};
    int xeon_mp = 0;
    
    printf("=== CPU Cache Detection Program ===\n");
    
    /* Step 1: Get basic processor info (CPUID leaf 0x1) */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    printf("CPUID Leaf 0x1: EAX=0x%08x\n", eax);
    
    /* Determine Xeon MP status */
    xeon_mp = is_xeon_mp(eax);
    printf("Xeon MP detection: %s\n", xeon_mp ? "Yes" : "No");
    
    /* Step 2: Get cache descriptors (CPUID leaf 0x2) */
    printf("\nProcessing cache descriptors from CPUID leaf 0x2:\n");
    
    /* According to Intel spec, CPUID leaf 0x2 may need multiple calls */
    for (int call_num = 0; call_num < 16; call_num++) {
        cpuid(0x2, call_num, &eax, &ebx, &ecx, &edx);
        
        /* Check if this is a valid leaf 2 response */
        if ((eax & 0xFF) == 0) {
            /* No more valid descriptors */
            break;
        }
        
        /* Process descriptor bytes from all registers */
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
        
        /* Process each descriptor byte */
        for (int i = 0; i < 16; i++) {
            uint8_t desc = descriptors[i];
            
            /* Skip invalid/reserved bytes */
            if (desc == 0x00) {
                /* Terminator byte - stop processing */
                goto done_processing;
            }
            
            if (desc == 0xFF) {
                /* Padding byte - skip */
                continue;
            }
            
            /* Check if this is one of our target descriptors */
            if ((desc >= 0x0a && desc <= 0x0e) ||
                desc == 0x21 || desc == 0x24 || desc == 0x2c ||
                (desc >= 0x39 && desc <= 0x45) ||
                desc == 0x48 || desc == 0x49 || desc == 0x4e ||
                (desc >= 0x60 && desc <= 0x68) ||
                (desc >= 0x78 && desc <= 0x87)) {
                
                printf("  Found target descriptor: 0x%02x\n", desc);
                process_cache_descriptor(desc, &level1, &level2, xeon_mp);
            }
        }
    }
    
done_processing:
    
    /* Step 3: Print collected cache information */
    printf("\n=== Detected Cache Information ===\n");
    print_cache_info(&level1, "L1");
    print_cache_info(&level2, "L2");
    
    /* Special handling for case 0x49 */
    if (level2.type == 0x49) {
        printf("Note: Case 0x49 processed with Xeon MP = %s\n", 
               xeon_mp ? "true (skipped)" : "false (populated)");
    }
    
    /* Step 4: Perform computation using cache line size */
    printf("\n=== Cache Line Computation ===\n");
    int line_size = level1.line > 0 ? level1.line : 
                   (level2.line > 0 ? level2.line : 64);
    cache_line_computation(line_size);
    
    /* Additional verification: test all target descriptors */
    printf("\n=== Testing All Target Descriptors ===\n");
    uint8_t all_targets[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    struct cache_desc test_level1 = {0};
    struct cache_desc test_level2 = {0};
    
    for (size_t i = 0; i < sizeof(all_targets); i++) {
        process_cache_descriptor(all_targets[i], &test_level1, &test_level2, 0);
        if (test_level1.sizekb > 0 || test_level2.sizekb > 0) {
            printf("Descriptor 0x%02x maps to cache structure\n", all_targets[i]);
            /* Reset for next test */
            test_level1.sizekb = 0;
            test_level2.sizekb = 0;
        }
    }
    
    printf("\nProgram completed successfully.\n");
    return 0;
}
