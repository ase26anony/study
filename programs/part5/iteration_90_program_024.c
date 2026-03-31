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
    int type;   /* cache type */
};

/* Function to execute CPUID */
static void cpuid(uint32_t func, uint32_t subfunc, 
                  uint32_t* eax, uint32_t* ebx, 
                  uint32_t* ecx, uint32_t* edx) {
#ifdef _WIN32
    int cpuInfo[4];
    __cpuidex(cpuInfo, func, subfunc);
    *eax = cpuInfo[0];
    *ebx = cpuInfo[1];
    *ecx = cpuInfo[2];
    *edx = cpuInfo[3];
#else
    __cpuid_count(func, subfunc, *eax, *ebx, *ecx, *edx);
#endif
}

/* Check if CPU is Xeon MP (simplified check) */
static int is_xeon_mp(uint32_t family, uint32_t model, uint32_t stepping) {
    /* Simplified: Xeon MP typically has specific family/model combinations
       This mimics the check in driver-i386.cc */
    if (family == 0xF && model >= 0x6) {
        /* Check for MP capable Xeon */
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, 0, &eax, &ebx, &ecx, &edx);
        
        /* Check HTT bit and APIC ID */
        if ((edx & (1 << 28)) && ((ebx >> 16) & 0xFF) > 1) {
            return 1;
        }
    }
    return 0;
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc* level1, 
                               struct cache_desc* level2, int xeon_mp) {
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->level = 1; level1->type = 1; /* Data cache */
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->level = 1; level1->type = 1;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
            
        /* L2 Cache cases */
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3; /* Unified cache */
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x49:
            if (xeon_mp) {
                /* Skip as per driver logic */
                printf("Case 0x49: Xeon MP detected, skipping L2 cache configuration\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 3;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            break;
            
        default:
            /* Not one of our target cases */
            break;
    }
}

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx, 
                                uint32_t ecx, uint32_t edx,
                                uint8_t* descriptors, int* count) {
    /* According to Intel manual, bytes are valid if bit 7 is 0 */
    int idx = 0;
    
    /* Process EAX (bits 7-0, 15-8, 23-16, 31-24) */
    for (int i = 0; i < 4; i++) {
        uint8_t byte = (eax >> (i * 8)) & 0xFF;
        if (byte != 0 && (byte & 0x80) == 0) {
            descriptors[idx++] = byte;
        }
    }
    
    /* Process EBX */
    for (int i = 0; i < 4; i++) {
        uint8_t byte = (ebx >> (i * 8)) & 0xFF;
        if (byte != 0 && (byte & 0x80) == 0) {
            descriptors[idx++] = byte;
        }
    }
    
    /* Process ECX */
    for (int i = 0; i < 4; i++) {
        uint8_t byte = (ecx >> (i * 8)) & 0xFF;
        if (byte != 0 && (byte & 0x80) == 0) {
            descriptors[idx++] = byte;
        }
    }
    
    /* Process EDX */
    for (int i = 0; i < 4; i++) {
        uint8_t byte = (edx >> (i * 8)) & 0xFF;
        if (byte != 0 && (byte & 0x80) == 0) {
            descriptors[idx++] = byte;
        }
    }
    
    *count = idx;
}

/* Simple computation using cache line size */
static void cache_line_computation(int line_size) {
    /* Allocate aligned memory and perform access pattern */
    const int array_size = 1024;
    char* buffer = (char*)aligned_alloc(line_size, array_size);
    
    if (buffer) {
        /* Access pattern that uses cache line size */
        for (int i = 0; i < array_size; i += line_size) {
            buffer[i] = (char)(i & 0xFF);
        }
        
        /* Force computation */
        volatile char sum = 0;
        for (int i = 0; i < array_size; i += line_size) {
            sum += buffer[i];
        }
        
        free(buffer);
    }
}

int main() {
    uint32_t eax, ebx, ecx, edx;
    struct cache_desc level1 = {0};
    struct cache_desc level2 = {0};
    int xeon_mp_flag = 0;
    
    printf("Starting CPU cache detection...\n");
    
    /* Step 1: Get CPU family/model/stepping for Xeon MP check */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    uint32_t stepping = eax & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t family = (eax >> 8) & 0xF;
    uint32_t extended_model = (eax >> 16) & 0xF;
    uint32_t extended_family = (eax >> 20) & 0xFF;
    
    if (family == 0xF) {
        model += (extended_model << 4);
        family += extended_family;
    }
    
    printf("CPU Family: %u, Model: %u, Stepping: %u\n", 
           family, model, stepping);
    
    /* Determine Xeon MP status */
    xeon_mp_flag = is_xeon_mp(family, model, stepping);
    printf("Xeon MP detected: %s\n", xeon_mp_flag ? "Yes" : "No");
    
    /* Step 2: Get cache descriptors using CPUID leaf 0x2 */
    printf("\nReading cache descriptors from CPUID leaf 0x2...\n");
    
    /* According to Intel manual, CPUID leaf 2 may need to be called multiple times */
    uint8_t all_descriptors[64];
    int total_desc_count = 0;
    
    for (int call_num = 0; call_num < 16; call_num++) {
        cpuid(2, call_num, &eax, &ebx, &ecx, &edx);
        
        /* Check if this is a valid call (AL register indicates number of calls needed) */
        uint8_t calls_needed = eax & 0xFF;
        if (calls_needed == 0) {
            break;
        }
        
        /* Extract descriptors from this call */
        uint8_t descriptors[16];
        int desc_count = 0;
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        /* Add to total list */
        for (int i = 0; i < desc_count && total_desc_count < 64; i++) {
            all_descriptors[total_desc_count++] = descriptors[i];
        }
        
        /* Check for terminator */
        for (int i = 0; i < desc_count; i++) {
            if (descriptors[i] == 0x00) {
                goto done_collecting;
            }
        }
    }
    
done_collecting:
    
    /* Step 3: Process all collected descriptors */
    printf("Processing %d cache descriptor bytes...\n", total_desc_count);
    
    for (int i = 0; i < total_desc_count; i++) {
        uint8_t desc = all_descriptors[i];
        
        /* Skip invalid/terminator bytes */
        if (desc == 0x00 || (desc & 0x80) != 0) {
            continue;
        }
        
        printf("Descriptor 0x%02x: ", desc);
        
        /* Process through our switch statement */
        process_descriptor(desc, &level1, &level2, xeon_mp_flag);
        
        /* Print which case was hit */
        switch (desc) {
            case 0x0a: case 0x0c: case 0x0d: case 0x0e:
            case 0x2c: case 0x60: case 0x66: case 0x67: case 0x68:
                printf("L1 Cache configuration\n");
                break;
            case 0x21: case 0x24: case 0x39: case 0x3a: case 0x3b:
            case 0x3c: case 0x3d: case 0x3e: case 0x41: case 0x42:
            case 0x43: case 0x44: case 0x45: case 0x48: case 0x49:
            case 0x4e: case 0x78: case 0x79: case 0x7a: case 0x7b:
            case 0x7c: case 0x7d: case 0x7f: case 0x80: case 0x82:
            case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
                printf("L2 Cache configuration\n");
                break;
            default:
                printf("Not in target cases\n");
                break;
        }
    }
    
    /* Step 4: Print collected cache information */
    printf("\n=== Cache Configuration Summary ===\n");
    
    if (level1.sizekb > 0) {
        printf("L1 Cache: %d KB, %d-way associative, %d byte line size\n",
               level1.sizekb, level1.assoc, level1.line);
        
        /* Perform computation using L1 cache line size */
        printf("Performing computation with L1 line size (%d bytes)...\n", level1.line);
        cache_line_computation(level1.line);
    }
    
    if (level2.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way associative, %d byte line size\n",
               level2.sizekb, level2.assoc, level2.line);
        
        /* Perform computation using L2 cache line size */
        printf("Performing computation with L2 line size (%d bytes)...\n", level2.line);
        cache_line_computation(level2.line);
    }
    
    /* Special check for case 0x49 */
    for (int i = 0; i < total_desc_count; i++) {
        if (all_descriptors[i] == 0x49) {
            printf("\nNote: Descriptor 0x49 was found. ");
            printf("Xeon MP condition was %s.\n", 
                   xeon_mp_flag ? "TRUE (cache not configured)" : 
                                  "FALSE (cache configured)");
            break;
        }
    }
    
    printf("\nCache detection complete.\n");
    
    return 0;
}
