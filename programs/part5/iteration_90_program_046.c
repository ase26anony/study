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
    int sizekb;    /* size in kilobytes */
    int assoc;     /* associativity */
    int line;      /* line size in bytes */
};

/* Cross-platform CPUID wrapper */
static void cpuid(uint32_t leaf, uint32_t subleaf, 
                  uint32_t* eax, uint32_t* ebx, 
                  uint32_t* ecx, uint32_t* edx) {
#ifdef _WIN32
    int cpu_info[4];
    __cpuidex(cpu_info, leaf, subleaf);
    *eax = cpu_info[0];
    *ebx = cpu_info[1];
    *ecx = cpu_info[2];
    *edx = cpu_info[3];
#else
    __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
#endif
}

/* Check if CPU is Xeon MP (simplified detection) */
static int is_xeon_mp(uint32_t family, uint32_t model, uint32_t stepping) {
    /* Simplified check: Xeon MP typically has specific family/model combinations
       This mimics the logic that would be in driver-i386.cc */
    if (family == 0xF && model >= 0x6) {
        /* Family 15h (Pentium 4 Xeon MP) */
        return 1;
    }
    /* Additional checks for other Xeon MP variants */
    if (family == 0x6 && model == 0xF) {
        /* Some Xeon MP models */
        return 1;
    }
    return 0;
}

/* Process cache descriptor byte */
static void process_cache_descriptor(uint8_t desc, struct cache_desc* level1, 
                                     struct cache_desc* level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("L1 Cache: 8KB, 2-way, 32B line (0x0a)\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("L1 Cache: 16KB, 4-way, 32B line (0x0c)\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64B line (0x0d)\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("L1 Cache: 24KB, 6-way, 64B line (0x0e)\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64B line (0x21)\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 1024KB, 16-way, 64B line (0x24)\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 32KB, 8-way, 64B line (0x2c)\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 128KB, 4-way, 64B line (0x39)\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 192KB, 6-way, 64B line (0x3a)\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 128KB, 2-way, 64B line (0x3b)\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 256KB, 4-way, 64B line (0x3c)\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 384KB, 6-way, 64B line (0x3d)\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64B line (0x3e)\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 128KB, 4-way, 32B line (0x41)\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 256KB, 4-way, 32B line (0x42)\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 512KB, 4-way, 32B line (0x43)\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 1024KB, 4-way, 32B line (0x44)\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 2048KB, 4-way, 32B line (0x45)\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("L2 Cache: 3072KB, 12-way, 64B line (0x48)\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Descriptor 0x49 skipped (Xeon MP detected)\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 4096KB, 16-way, 64B line (0x49)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("L2 Cache: 6144KB, 24-way, 64B line (0x4e)\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 16KB, 8-way, 64B line (0x60)\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 8KB, 4-way, 64B line (0x66)\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64B line (0x67)\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 32KB, 4-way, 64B line (0x68)\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 1024KB, 4-way, 64B line (0x78)\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 128KB, 8-way, 64B line (0x79)\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64B line (0x7a)\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64B line (0x7b)\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64B line (0x7c)\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 2048KB, 8-way, 64B line (0x7d)\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 512KB, 2-way, 64B line (0x7f)\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64B line (0x80)\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 256KB, 8-way, 32B line (0x82)\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 512KB, 8-way, 32B line (0x83)\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 1024KB, 8-way, 32B line (0x84)\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 2048KB, 8-way, 32B line (0x85)\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64B line (0x86)\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64B line (0x87)\n");
            break;
        default:
            /* Other descriptors not in uncovered lines */
            if (desc != 0x00 && (desc & 0x80) == 0) {
                printf("Unknown cache descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract bytes from CPUID result */
static void extract_descriptor_bytes(uint32_t reg, uint8_t* bytes) {
    bytes[0] = (reg >> 0) & 0xFF;
    bytes[1] = (reg >> 8) & 0xFF;
    bytes[2] = (reg >> 16) & 0xFF;
    bytes[3] = (reg >> 24) & 0xFF;
}

/* Simple computation using cache line size to prevent optimization */
static void cache_line_computation(int line_size) {
    volatile int* array = (volatile int*)malloc(1024 * sizeof(int));
    if (array) {
        /* Align access to cache line boundary */
        int index = (line_size > 0) ? (line_size / sizeof(int)) : 16;
        if (index > 1024) index = 1024 - 1;
        
        array[index] = 42;
        volatile int result = array[index];
        (void)result; /* Prevent unused variable warning */
        
        free((void*)array);
    }
}

int main(void) {
    uint32_t eax, ebx, ecx, edx;
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int xeon_mp = 0;
    
    printf("=== CPU Cache Detection Program ===\n");
    
    /* Step 1: Get basic processor info (CPUID leaf 0x1) */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family/model/stepping */
    uint32_t stepping = eax & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t family = (eax >> 8) & 0xF;
    if (family == 0xF) {
        family += (eax >> 20) & 0xFF;
        model |= ((eax >> 16) & 0xF) << 4;
    }
    
    printf("CPU Family: %u, Model: %u, Stepping: %u\n", 
           family, model, stepping);
    
    /* Determine Xeon MP status */
    xeon_mp = is_xeon_mp(family, model, stepping);
    printf("Xeon MP detected: %s\n", xeon_mp ? "Yes" : "No");
    
    /* Step 2: Get cache descriptors (CPUID leaf 0x2) */
    printf("\nProcessing CPUID leaf 0x2 cache descriptors:\n");
    
    /* According to Intel spec, CPUID leaf 0x2 may need to be called multiple times */
    int iterations = 0;
    int max_iterations = 16; /* Safety limit */
    
    while (iterations < max_iterations) {
        cpuid(0x2, iterations, &eax, &ebx, &ecx, &edx);
        
        /* Check if this is a valid cache descriptor leaf */
        if ((eax & 0xFF) == 0) {
            /* No more valid descriptors */
            break;
        }
        
        /* Extract descriptor bytes from all registers */
        uint8_t bytes[16];
        extract_descriptor_bytes(eax, &bytes[0]);
        extract_descriptor_bytes(ebx, &bytes[4]);
        extract_descriptor_bytes(ecx, &bytes[8]);
        extract_descriptor_bytes(edx, &bytes[12]);
        
        /* Process each byte */
        for (int i = 0; i < 16; i++) {
            if (bytes[i] == 0x00) {
                /* Terminator byte found */
                goto done_processing;
            }
            
            /* Skip if high bit is set (register is valid) */
            if ((bytes[i] & 0x80) == 0) {
                process_cache_descriptor(bytes[i], &level1, &level2, xeon_mp);
            }
        }
        
        iterations++;
    }
    
done_processing:
    
    /* Step 3: Print final cache information */
    printf("\n=== Final Cache Information ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %dB line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %dB line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Step 4: Perform computation using cache line size */
    printf("\nPerforming cache-aware computation...\n");
    int line_size = (level1.line > 0) ? level1.line : 
                   (level2.line > 0) ? level2.line : 64;
    cache_line_computation(line_size);
    
    printf("\nProgram completed successfully.\n");
    return 0;
}
