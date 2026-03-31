/*
 * This program is designed to trigger the CPUID-based cache detection logic
 * in driver-i386.cc lines 127-244. It calls CPUID leaf 0x2 to retrieve cache
 * descriptor bytes and processes them according to Intel's specification,
 * implementing the exact case statements from the uncovered code block.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#define cpuid(info, x) __cpuid(info, x)
#define cpuidex(info, x, ecx_val) __cpuidex(info, x, ecx_val)
#else
#include <cpuid.h>
#define cpuid(info, x) __cpuid(x, info[0], info[1], info[2], info[3])
#define cpuidex(info, x, ecx_val) __cpuid_count(x, ecx_val, info[0], info[1], info[2], info[3])
#endif

/* Cache descriptor structure matching driver-i386.cc logic */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int valid;      /* Whether this entry is populated */
    uint8_t type;   /* Cache descriptor byte value */
};

/* Global flag to simulate xeon_mp check from driver-i386.cc */
static int xeon_mp = 0;

/* Function to check if CPU is Xeon MP based on CPUID leaf 0x1 */
static void check_xeon_mp(void) {
    uint32_t info[4];
    cpuid(info, 0x1);
    
    /* Extract family, model, stepping */
    uint32_t family = ((info[0] >> 8) & 0xF) | ((info[0] >> 16) & 0xFF0);
    uint32_t model = ((info[0] >> 4) & 0xF) | ((info[0] >> 12) & 0xF0);
    uint32_t stepping = info[0] & 0xF;
    
    /* Simplified Xeon MP detection logic */
    /* This mimics the check in driver-i386.cc for case 0x49 */
    if (family == 0xF && model >= 0x6) {
        /* Check for Xeon MP characteristics */
        /* In real implementation, this would check specific CPU signatures */
        xeon_mp = 1;
    }
    
    printf("CPU Info: Family=%u, Model=%u, Stepping=%u, Xeon_MP=%d\n",
           family, model, stepping, xeon_mp);
}

/* Process a single cache descriptor byte */
static void process_cache_descriptor(uint8_t desc, 
                                    struct cache_desc *level1,
                                    struct cache_desc *level2) {
    /* Skip invalid descriptors */
    if (desc == 0x00 || desc == 0xFF) {
        return;
    }
    
    /* Check if this is a L1 or L2 cache descriptor */
    int is_l1 = 0;
    int is_l2 = 0;
    
    /* Determine cache level based on descriptor value ranges */
    if ((desc >= 0x06 && desc <= 0x0A) || 
        (desc >= 0x0C && desc <= 0x0E) ||
        desc == 0x2C || desc == 0x60 || 
        desc == 0x66 || desc == 0x67 || desc == 0x68) {
        is_l1 = 1;
    } else if ((desc >= 0x21 && desc <= 0x24) ||
               (desc >= 0x39 && desc <= 0x4E) ||
               (desc >= 0x78 && desc <= 0x87) ||
               desc == 0x80 || desc == 0x82 || 
               desc == 0x83 || desc == 0x84 || 
               desc == 0x85 || desc == 0x86 || desc == 0x87) {
        is_l2 = 1;
    }
    
    /* Process according to the exact cases from driver-i386.cc */
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            if (is_l1) {
                level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
        case 0x0c:
            if (is_l1) {
                level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
        case 0x0d:
            if (is_l1) {
                level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
        case 0x0e:
            if (is_l1) {
                level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
        case 0x2c:
            if (is_l1) {
                level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
        case 0x60:
            if (is_l1) {
                level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
        case 0x66:
            if (is_l1) {
                level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
        case 0x67:
            if (is_l1) {
                level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
        case 0x68:
            if (is_l1) {
                level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
                level1->valid = 1; level1->type = desc;
                printf("  L1 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level1->sizekb, level1->assoc, level1->line);
            }
            break;
            
        /* L2 Cache cases */
        case 0x21:
            if (is_l2) {
                level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x24:
            if (is_l2) {
                level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x39:
            if (is_l2) {
                level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x3a:
            if (is_l2) {
                level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x3b:
            if (is_l2) {
                level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x3c:
            if (is_l2) {
                level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x3d:
            if (is_l2) {
                level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x3e:
            if (is_l2) {
                level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x41:
            if (is_l2) {
                level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x42:
            if (is_l2) {
                level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x43:
            if (is_l2) {
                level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x44:
            if (is_l2) {
                level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x45:
            if (is_l2) {
                level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x48:
            if (is_l2) {
                level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x49:
            if (is_l2) {
                /* Special case with xeon_mp check */
                if (!xeon_mp) {
                    level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
                    level2->valid = 1; level2->type = desc;
                    printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line (non-Xeon-MP)\n",
                           desc, level2->sizekb, level2->assoc, level2->line);
                } else {
                    printf("  L2 Cache: 0x%02x skipped (Xeon-MP detected)\n", desc);
                }
            }
            break;
        case 0x4e:
            if (is_l2) {
                level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x78:
            if (is_l2) {
                level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x79:
            if (is_l2) {
                level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x7a:
            if (is_l2) {
                level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x7b:
            if (is_l2) {
                level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x7c:
            if (is_l2) {
                level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x7d:
            if (is_l2) {
                level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x7f:
            if (is_l2) {
                level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x80:
            if (is_l2) {
                level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x82:
            if (is_l2) {
                level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x83:
            if (is_l2) {
                level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x84:
            if (is_l2) {
                level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x85:
            if (is_l2) {
                level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x86:
            if (is_l2) {
                level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        case 0x87:
            if (is_l2) {
                level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
                level2->valid = 1; level2->type = desc;
                printf("  L2 Cache: 0x%02x -> %dKB, %d-way, %d-byte line\n",
                       desc, level2->sizekb, level2->assoc, level2->line);
            }
            break;
        default:
            /* Other cache descriptors not in our target list */
            if (desc != 0x00 && desc != 0xFF && 
                (desc & 0x80) == 0) {  /* Valid cache descriptor */
                printf("  Other cache descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract cache descriptor bytes from CPUID leaf 0x2 results */
static void extract_cache_descriptors(uint32_t eax, uint32_t ebx, 
                                     uint32_t ecx, uint32_t edx,
                                     uint8_t *descriptors, int *count) {
    int i = 0;
    
    /* Extract bytes from EAX (bits 7-0 are valid) */
    if ((eax & 0xFF) != 0 && (eax & 0xFF) != 0xFF) {
        descriptors[i++] = eax & 0xFF;
    }
    
    /* Extract bytes from EBX, ECX, EDX */
    for (int j = 0; j < 4; j++) {
        uint8_t byte;
        
        if (j == 0) byte = (ebx >> 0) & 0xFF;
        else if (j == 1) byte = (ebx >> 8) & 0xFF;
        else if (j == 2) byte = (ebx >> 16) & 0xFF;
        else if (j == 3) byte = (ebx >> 24) & 0xFF;
        
        if (byte != 0 && byte != 0xFF) {
            descriptors[i++] = byte;
        }
    }
    
    for (int j = 0; j < 4; j++) {
        uint8_t byte;
        
        if (j == 0) byte = (ecx >> 0) & 0xFF;
        else if (j == 1) byte = (ecx >> 8) & 0xFF;
        else if (j == 2) byte = (ecx >> 16) & 0xFF;
        else if (j == 3) byte = (ecx >> 24) & 0xFF;
        
        if (byte != 0 && byte != 0xFF) {
            descriptors[i++] = byte;
        }
    }
    
    for (int j = 0; j < 4; j++) {
        uint8_t byte;
        
        if (j == 0) byte = (edx >> 0) & 0xFF;
        else if (j == 1) byte = (edx >> 8) & 0xFF;
        else if (j == 2) byte = (edx >> 16) & 0xFF;
        else if (j == 3) byte = (edx >> 24) & 0xFF;
        
        if (byte != 0 && byte != 0xFF) {
            descriptors[i++] = byte;
        }
    }
    
    *count = i;
}

/* Simple computation using cache line size to prevent optimization */
static void cache_line_optimized_computation(int cache_line_size) {
    /* Align array to cache line boundary */
    #define ARRAY_SIZE 1024
    alignas(64) static int data[ARRAY_SIZE];
    
    /* Access elements with stride equal to cache line size */
    int stride = cache_line_size / sizeof(int);
    if (stride == 0) stride = 1;
    
    volatile int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i += stride) {
        sum += data[i];
        data[i] = i;  /* Write to prevent read-only optimization */
    }
    
    /* Use sum to prevent dead code elimination */
    if (sum == 0) {
        printf("Cache line computation completed (line size: %d bytes)\n", 
               cache_line_size);
    }
}

int main(void) {
    uint32_t info[4];
    uint8_t descriptors[64];
    int desc_count = 0;
    
    struct cache_desc level1 = {0, 0, 0, 0, 0};
    struct cache_desc level2 = {0, 0, 0, 0, 0};
    
    printf("=== CPU Cache Detection Program ===\n");
    
    /* Step 1: Check Xeon MP status */
    check_xeon_mp();
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("\nCalling CPUID leaf 0x2 (Cache Descriptors):\n");
    
    int iterations = 0;
    int max_iterations = 10;  /* Safety limit */
    
    while (iterations < max_iterations) {
        cpuid(info, 0x2);
        
        printf("Iteration %d: EAX=0x%08X, EBX=0x%08X, ECX=0x%08X, EDX=0x%08X\n",
               iterations, info[0], info[1], info[2], info[3]);
        
        /* Extract cache descriptors from this iteration */
        int new_count = 0;
        extract_cache_descriptors(info[0], info[1], info[2], info[3],
                                 descriptors + desc_count, &new_count);
        desc_count += new_count;
        
        /* Check if we should continue (bit 7 of AL indicates if we need to call again) */
        if ((info[0] & 0x80) == 0) {
            break;
        }
        
        iterations++;
    }
    
    /* Step 3: Process all collected descriptors */
    printf("\nProcessing %d cache descriptor(s):\n", desc_count);
    for (int i = 0; i < desc_count; i++) {
        process_cache_descriptor(descriptors[i], &level1, &level2);
    }
    
    /* Step 4: Print final cache information */
    printf("\n=== Final Cache Configuration ===\n");
    if (level1.valid) {
        printf("L1 Cache: %dKB, %d-way associative, %d-byte line (descriptor: 0x%02x)\n",
               level1.sizekb, level1.assoc, level1.line, level1.type);
    } else {
        printf("L1 Cache: Not detected in target descriptors\n");
    }
    
    if (level2.valid) {
        printf("L2 Cache: %dKB, %d-way associative, %d-byte line (descriptor: 0x%02x)\n",
               level2.sizekb, level2.assoc, level2.line, level2.type);
    } else {
        printf("L2 Cache: Not detected in target descriptors\n");
    }
    
    /* Step 5: Perform cache-aware computation */
    printf("\n=== Cache-Aware Computation ===\n");
    int cache_line_size = level1.line;
    if (cache_line_size == 0) {
        cache_line_size = level2.line;
    }
    if (cache_line_size == 0) {
        cache_line_size = 64;  /* Default assumption */
    }
    
    cache_line_optimized_computation(cache_line_size);
    
    /* Force use of variables to prevent optimization */
    volatile int dummy = level1.sizekb + level2.sizekb;
    (void)dummy;
    
    return 0;
}
