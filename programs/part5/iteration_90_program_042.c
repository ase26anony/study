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
    int level;  /* 1 for L1, 2 for L2 */
    int type;   /* Cache type */
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

/* Check if CPU is Xeon MP (simplified check based on family/model) */
static int is_xeon_mp(uint32_t eax_from_cpuid1) {
    /* Extract family, model, stepping from CPUID leaf 1 EAX */
    uint32_t family = ((eax_from_cpuid1 >> 8) & 0xF);
    uint32_t model = ((eax_from_cpuid1 >> 4) & 0xF);
    uint32_t extended_family = ((eax_from_cpuid1 >> 20) & 0xFF);
    uint32_t extended_model = ((eax_from_cpuid1 >> 16) & 0xF);
    
    /* Simplified check: Xeon MP typically has specific family/model combinations */
    /* This is a simplified heuristic - real detection would be more complex */
    if (family == 0xF) {
        uint32_t full_family = family + extended_family;
        uint32_t full_model = (extended_model << 4) | model;
        
        /* Some Xeon MP models */
        if (full_family == 0xF && (full_model == 0x6 || full_model == 0x7)) {
            return 1;
        }
    }
    return 0;
}

/* Process cache descriptor byte according to uncovered logic */
static void process_cache_descriptor(uint8_t desc, struct cache_desc* level1, 
                                    struct cache_desc* level2, int xeon_mp) {
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
                /* Special case: break if Xeon MP */
                printf("Case 0x49: Xeon MP detected, skipping L2 cache setup\n");
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

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx,
                               uint8_t* descriptors, int* count) {
    int i = 0;
    
    /* EAX bits 7-0 are valid descriptors */
    uint8_t eax_desc = eax & 0xFF;
    if (eax_desc != 0 && eax_desc != 1) {
        descriptors[i++] = eax_desc;
    }
    
    /* EBX, ECX, EDX contain descriptors in each byte */
    for (int j = 0; j < 4; j++) {
        uint8_t desc = (ebx >> (j * 8)) & 0xFF;
        if (desc != 0 && desc != 1) {
            descriptors[i++] = desc;
        }
    }
    
    for (int j = 0; j < 4; j++) {
        uint8_t desc = (ecx >> (j * 8)) & 0xFF;
        if (desc != 0 && desc != 1) {
            descriptors[i++] = desc;
        }
    }
    
    for (int j = 0; j < 4; j++) {
        uint8_t desc = (edx >> (j * 8)) & 0xFF;
        if (desc != 0 && desc != 1) {
            descriptors[i++] = desc;
        }
    }
    
    *count = i;
}

/* Simple computation using cache line size to prevent optimization */
static void cache_aware_computation(int cache_line_size) {
    /* Allocate aligned memory based on cache line size */
    int array_size = 1024;
    char* buffer = (char*)aligned_alloc(cache_line_size, array_size);
    
    if (buffer) {
        /* Perform computation that respects cache lines */
        for (int i = 0; i < array_size; i += cache_line_size) {
            buffer[i] = (char)(i % 256);
        }
        
        volatile char result = buffer[0]; /* Prevent optimization */
        (void)result;
        
        free(buffer);
    }
}

int main() {
    uint32_t eax, ebx, ecx, edx;
    struct cache_desc l1_cache = {0};
    struct cache_desc l2_cache = {0};
    int xeon_mp = 0;
    
    printf("Starting CPUID cache detection...\n");
    
    /* Step 1: Get basic processor info (CPUID leaf 0x1) */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    xeon_mp = is_xeon_mp(eax);
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Get cache descriptors (CPUID leaf 0x2) */
    /* According to Intel spec, CPUID leaf 2 may need to be called multiple times */
    uint8_t all_descriptors[64];
    int total_descriptors = 0;
    
    for (int call_num = 0; call_num < 3; call_num++) {
        cpuid(0x2, call_num, &eax, &ebx, &ecx, &edx);
        
        /* Check if this is a valid call (AL bit 7 = 0) */
        if ((eax & 0x80) != 0) {
            /* Invalid call, break */
            break;
        }
        
        /* Extract descriptors from this call */
        uint8_t descriptors[16];
        int desc_count = 0;
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        /* Add to total list */
        for (int i = 0; i < desc_count; i++) {
            all_descriptors[total_descriptors++] = descriptors[i];
        }
        
        /* Check for terminator */
        if ((eax & 0xFF) == 0) {
            /* Found terminator */
            break;
        }
    }
    
    printf("Found %d cache descriptor bytes\n", total_descriptors);
    
    /* Step 3: Process all descriptor bytes */
    for (int i = 0; i < total_descriptors; i++) {
        uint8_t desc = all_descriptors[i];
        printf("Processing descriptor 0x%02x\n", desc);
        
        /* Process using the exact logic from uncovered lines */
        process_cache_descriptor(desc, &l1_cache, &l2_cache, xeon_mp);
    }
    
    /* Step 4: Print results */
    printf("\n=== Cache Detection Results ===\n");
    
    if (l1_cache.sizekb > 0) {
        printf("L1 Cache: %d KB, %d-way associative, %d byte line size (type: 0x%02x)\n",
               l1_cache.sizekb, l1_cache.assoc, l1_cache.line, l1_cache.type);
    } else {
        printf("L1 Cache: Not detected by target descriptors\n");
    }
    
    if (l2_cache.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way associative, %d byte line size (type: 0x%02x)\n",
               l2_cache.sizekb, l2_cache.assoc, l2_cache.line, l2_cache.type);
    } else {
        printf("L2 Cache: Not detected by target descriptors\n");
    }
    
    /* Step 5: Perform cache-aware computation */
    printf("\nPerforming cache-aware computation...\n");
    int cache_line_size = l1_cache.line > 0 ? l1_cache.line : 
                         l2_cache.line > 0 ? l2_cache.line : 64;
    cache_aware_computation(cache_line_size);
    
    /* Print all descriptors found for debugging */
    printf("\nAll descriptor bytes found: ");
    for (int i = 0; i < total_descriptors; i++) {
        printf("0x%02x ", all_descriptors[i]);
    }
    printf("\n");
    
    /* Check for specific target descriptors */
    printf("\nTarget descriptors found: ");
    const uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    int found_targets = 0;
    for (int i = 0; i < total_descriptors; i++) {
        for (size_t j = 0; j < sizeof(target_descriptors); j++) {
            if (all_descriptors[i] == target_descriptors[j]) {
                printf("0x%02x ", all_descriptors[i]);
                found_targets++;
                break;
            }
        }
    }
    
    if (found_targets == 0) {
        printf("None");
    }
    printf("\n");
    
    return 0;
}
