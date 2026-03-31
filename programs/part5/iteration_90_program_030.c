/*
 * This program is designed to trigger the specific cache descriptor parsing logic
 * in driver-i386.cc lines 127-244 by executing CPUID leaf 0x2 and processing
 * the returned cache descriptor bytes according to Intel's specification.
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
    int sizekb;    /* Size in KB */
    int assoc;     /* Associativity */
    int line;      /* Line size in bytes */
};

/* Cross-platform CPUID function */
static void cpuid(uint32_t leaf, uint32_t subleaf, 
                  uint32_t* eax, uint32_t* ebx, 
                  uint32_t* ecx, uint32_t* edx) {
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

/* Check if CPU is Xeon MP (for case 0x49) */
static int is_xeon_mp() {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU signature from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF);
    uint32_t model = ((eax >> 4) & 0xF);
    uint32_t extended_family = ((eax >> 20) & 0xFF);
    uint32_t extended_model = ((eax >> 16) & 0xF);
    
    /* For Intel Xeon MP detection:
       - Family 0xF, extended family 0
       - Models that indicate Xeon MP (simplified check) */
    if (family == 0xF) {
        uint32_t full_family = family + extended_family;
        uint32_t full_model = model + (extended_model << 4);
        
        /* Check for Xeon MP models (simplified for demonstration) */
        if (full_family == 0xF && (full_model == 0x6 || full_model == 0x4)) {
            return 1;
        }
    }
    
    return 0;
}

/* Process a single cache descriptor byte */
static void process_descriptor_byte(uint8_t desc, struct cache_desc* level1, 
                                   struct cache_desc* level2, int xeon_mp) {
    switch (desc) {
        /* L1 cache descriptors */
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
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 32KB, 8-way, 64B line\n", desc);
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
            
        /* L2 cache descriptors */
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 256KB, 8-way, 64B line\n", desc);
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1024KB, 16-way, 64B line\n", desc);
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
            
        /* Valid descriptor but not in uncovered lines - ignore */
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x06:
        case 0x08:
        case 0x09:
        case 0x0b:
        case 0x10:
        case 0x15:
        case 0x1a:
        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8d:
        case 0x90:
        case 0x96:
        case 0x9b:
            printf("  Cache descriptor 0x%02x: Valid but not in target lines\n", desc);
            break;
            
        /* Invalid descriptor (bit 0 = 0) */
        default:
            if ((desc & 0x1F) != 0) {
                printf("  Cache descriptor 0x%02x: Invalid (bit 0 = 0)\n", desc);
            }
            break;
    }
}

/* Extract bytes from CPUID result registers */
static void extract_descriptor_bytes(uint32_t eax, uint32_t ebx, 
                                     uint32_t ecx, uint32_t edx,
                                     uint8_t* bytes, int* count) {
    uint8_t* regs = (uint8_t*)&eax;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00) {
            bytes[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&ebx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00) {
            bytes[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&ecx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00) {
            bytes[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&edx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00) {
            bytes[(*count)++] = regs[i];
        }
    }
}

/* Perform cache-aware computation to prevent optimization */
static void cache_aware_computation(int cache_line_size) {
    /* Allocate aligned memory based on detected cache line size */
    size_t alignment = (cache_line_size > 0) ? cache_line_size : 64;
    size_t array_size = 1024 * 1024; /* 1MB */
    
    /* Use volatile to prevent optimization */
    volatile uint8_t* buffer = (volatile uint8_t*)aligned_alloc(alignment, array_size);
    if (!buffer) return;
    
    /* Perform strided access pattern that benefits from cache line alignment */
    for (size_t i = 0; i < array_size; i += alignment) {
        buffer[i] = (uint8_t)(i & 0xFF);
    }
    
    /* Sum to create side effect */
    volatile uint8_t sum = 0;
    for (size_t i = 0; i < array_size; i += alignment) {
        sum += buffer[i];
    }
    
    free((void*)buffer);
    
    /* Use sum to prevent dead code elimination */
    if (sum == 0) {
        printf("  Cache-aware computation completed (dummy check)\n");
    }
}

int main() {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    uint8_t descriptor_bytes[256];
    int byte_count = 0;
    int iteration = 0;
    int xeon_mp = 0;
    
    printf("Starting CPUID-based cache detection...\n");
    
    /* Step 1: Check for Xeon MP */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("\nCalling CPUID leaf 0x2...\n");
    
    while (iteration < 10) { /* Safety limit */
        uint32_t eax, ebx, ecx, edx;
        
        cpuid(0x2, iteration, &eax, &ebx, &ecx, &edx);
        
        printf("Iteration %d: EAX=0x%08x EBX=0x%08x ECX=0x%08x EDX=0x%08x\n",
               iteration, eax, ebx, ecx, edx);
        
        /* Check if EAX[7:0] indicates number of times to call CPUID */
        uint8_t call_count = eax & 0xFF;
        if (call_count == 0) {
            call_count = 1;
        }
        
        /* Extract descriptor bytes from all registers */
        extract_descriptor_bytes(eax, ebx, ecx, edx, descriptor_bytes, &byte_count);
        
        iteration++;
        
        /* Check termination condition */
        if ((eax & 0xFF) == 0x00 && (ebx & 0xFF) == 0x00 && 
            (ecx & 0xFF) == 0x00 && (edx & 0xFF) == 0x00) {
            break;
        }
        
        if (iteration >= call_count) {
            break;
        }
    }
    
    /* Step 3: Process all collected descriptor bytes */
    printf("\nProcessing %d cache descriptor bytes:\n", byte_count);
    
    for (int i = 0; i < byte_count; i++) {
        process_descriptor_byte(descriptor_bytes[i], &level1, &level2, xeon_mp);
    }
    
    /* Step 4: Print final cache information */
    printf("\n=== Final Cache Configuration ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %d KB, %d-way associative, %d-byte line\n",
               level1.sizekb, level1.assoc, level1.line);
    } else {
        printf("L1 Cache: Not detected in target descriptors\n");
    }
    
    if (level2.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way associative, %d-byte line\n",
               level2.sizekb, level2.assoc, level2.line);
    } else {
        printf("L2 Cache: Not detected in target descriptors\n");
    }
    
    /* Step 5: Perform cache-aware computation */
    printf("\nPerforming cache-aware computation...\n");
    int cache_line = (level1.line > 0) ? level1.line : 
                     (level2.line > 0) ? level2.line : 64;
    cache_aware_computation(cache_line);
    
    printf("\nCache detection complete.\n");
    
    return 0;
}
