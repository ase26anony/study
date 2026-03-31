/*
 * This program triggers Intel CPU cache detection logic by calling CPUID leaf 0x2
 * and processing cache descriptor bytes matching the uncovered cases from driver-i386.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#include <windows.h>
#define cpuid(info, x) __cpuid(info, x)
#define cpuidex(info, x, ecx_val) __cpuidex(info, x, ecx_val)
#else
#include <cpuid.h>
#define cpuid(info, x) __cpuid(x, info[0], info[1], info[2], info[3])
#define cpuidex(info, x, ecx_val) __cpuid_count(x, ecx_val, info[0], info[1], info[2], info[3])
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int level;      /* Cache level (1 or 2) */
    uint8_t type;   /* Descriptor byte value */
};

/* Function to check if CPU is Xeon MP (for case 0x49) */
static int is_xeon_mp(void) {
    uint32_t info[4];
    cpuid(info, 0x00000001);
    
    /* Extract family, model, stepping */
    uint32_t stepping = info[0] & 0xF;
    uint32_t model = (info[0] >> 4) & 0xF;
    uint32_t family = (info[0] >> 8) & 0xF;
    uint32_t extended_model = (info[0] >> 16) & 0xF;
    uint32_t extended_family = (info[0] >> 20) & 0xFF;
    
    /* For Intel Xeon MP (based on some known models) */
    /* This is a simplified check - real implementation would be more complex */
    if (family == 0xF) {
        uint32_t full_model = (extended_model << 4) | model;
        /* Check for some Xeon MP family 0xF models */
        if (full_model == 0x6 || full_model == 0x7) {
            return 1;
        }
    }
    
    return 0;
}

/* Process a cache descriptor byte and populate cache structure */
static void process_descriptor(uint8_t desc, struct cache_desc *cache, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            cache->sizekb = 8; cache->assoc = 2; cache->line = 32;
            cache->level = 1;
            break;
        case 0x0c:
            cache->sizekb = 16; cache->assoc = 4; cache->line = 32;
            cache->level = 1;
            break;
        case 0x0d:
            cache->sizekb = 16; cache->assoc = 4; cache->line = 64;
            cache->level = 1;
            break;
        case 0x0e:
            cache->sizekb = 24; cache->assoc = 6; cache->line = 64;
            cache->level = 1;
            break;
        case 0x21:
            cache->sizekb = 256; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x24:
            cache->sizekb = 1024; cache->assoc = 16; cache->line = 64;
            cache->level = 2;
            break;
        case 0x2c:
            cache->sizekb = 32; cache->assoc = 8; cache->line = 64;
            cache->level = 1;
            break;
        case 0x39:
            cache->sizekb = 128; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3a:
            cache->sizekb = 192; cache->assoc = 6; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3b:
            cache->sizekb = 128; cache->assoc = 2; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3c:
            cache->sizekb = 256; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3d:
            cache->sizekb = 384; cache->assoc = 6; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3e:
            cache->sizekb = 512; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x41:
            cache->sizekb = 128; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x42:
            cache->sizekb = 256; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x43:
            cache->sizekb = 512; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x44:
            cache->sizekb = 1024; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x45:
            cache->sizekb = 2048; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x48:
            cache->sizekb = 3072; cache->assoc = 12; cache->line = 64;
            cache->level = 2;
            break;
        case 0x49:
            if (xeon_mp) {
                /* Skip population for Xeon MP */
                cache->sizekb = 0; cache->assoc = 0; cache->line = 0;
                cache->level = 0;
            } else {
                cache->sizekb = 4096; cache->assoc = 16; cache->line = 64;
                cache->level = 2;
            }
            break;
        case 0x4e:
            cache->sizekb = 6144; cache->assoc = 24; cache->line = 64;
            cache->level = 2;
            break;
        case 0x60:
            cache->sizekb = 16; cache->assoc = 8; cache->line = 64;
            cache->level = 1;
            break;
        case 0x66:
            cache->sizekb = 8; cache->assoc = 4; cache->line = 64;
            cache->level = 1;
            break;
        case 0x67:
            cache->sizekb = 16; cache->assoc = 4; cache->line = 64;
            cache->level = 1;
            break;
        case 0x68:
            cache->sizekb = 32; cache->assoc = 4; cache->line = 64;
            cache->level = 1;
            break;
        case 0x78:
            cache->sizekb = 1024; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x79:
            cache->sizekb = 128; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7a:
            cache->sizekb = 256; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7b:
            cache->sizekb = 512; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7c:
            cache->sizekb = 1024; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7d:
            cache->sizekb = 2048; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7f:
            cache->sizekb = 512; cache->assoc = 2; cache->line = 64;
            cache->level = 2;
            break;
        case 0x80:
            cache->sizekb = 512; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x82:
            cache->sizekb = 256; cache->assoc = 8; cache->line = 32;
            cache->level = 2;
            break;
        case 0x83:
            cache->sizekb = 512; cache->assoc = 8; cache->line = 32;
            cache->level = 2;
            break;
        case 0x84:
            cache->sizekb = 1024; cache->assoc = 8; cache->line = 32;
            cache->level = 2;
            break;
        case 0x85:
            cache->sizekb = 2048; cache->assoc = 8; cache->line = 32;
            cache->level = 2;
            break;
        case 0x86:
            cache->sizekb = 512; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x87:
            cache->sizekb = 1024; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        default:
            /* Not one of our target descriptors */
            cache->sizekb = 0; cache->assoc = 0; cache->line = 0;
            cache->level = 0;
            break;
    }
    cache->type = desc;
}

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t reg, uint8_t *descriptors, int *count) {
    uint8_t *bytes = (uint8_t *)&reg;
    
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00) {
            descriptors[*count] = bytes[i];
            (*count)++;
        }
    }
}

/* Simple computation using cache line size to prevent optimization */
static void cache_line_computation(int line_size) {
    if (line_size <= 0) return;
    
    /* Allocate aligned memory based on cache line size */
    size_t array_size = line_size * 1024;
    char *buffer = (char*)malloc(array_size);
    
    if (buffer) {
        /* Access memory with stride equal to cache line size */
        volatile int sum = 0;
        for (size_t i = 0; i < array_size; i += line_size) {
            buffer[i] = (char)(i % 256);
            sum += buffer[i];
        }
        
        /* Use sum to prevent dead code elimination */
        if (sum > 0) {
            printf("Cache line computation completed (sum=%d)\n", sum);
        }
        
        free(buffer);
    }
}

int main(void) {
    uint32_t info[4];
    uint8_t descriptors[256];
    int desc_count = 0;
    struct cache_desc caches[32];
    int cache_count = 0;
    
    printf("=== Intel CPU Cache Detection Program ===\n\n");
    
    /* Step 1: Get basic processor info and check for Xeon MP */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times to get cache descriptors */
    printf("Calling CPUID leaf 0x2 (Cache and TLB Information)...\n");
    
    int iterations = 0;
    int valid_descriptor_found = 0;
    
    do {
        cpuid(info, 0x00000002);
        
        printf("Iteration %d: EAX=%08X EBX=%08X ECX=%08X EDX=%08X\n",
               iterations, info[0], info[1], info[2], info[3]);
        
        /* Extract descriptor bytes from all registers */
        extract_descriptors(info[0], descriptors, &desc_count);
        extract_descriptors(info[1], descriptors, &desc_count);
        extract_descriptors(info[2], descriptors, &desc_count);
        extract_descriptors(info[3], descriptors, &desc_count);
        
        /* Check if AL register (low byte of EAX) indicates more iterations */
        iterations++;
        
        /* According to Intel manual, we should call CPUID with EAX=2 repeatedly
         * until we get a valid descriptor set (AL[7] = 0) */
    } while (iterations < 3 && (info[0] & 0xFF) == 0x01);
    
    printf("\nTotal descriptor bytes collected: %d\n", desc_count);
    
    /* Step 3: Process all collected descriptor bytes */
    printf("\nProcessing cache descriptors:\n");
    printf("-----------------------------\n");
    
    for (int i = 0; i < desc_count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid/reserved descriptors */
        if (desc == 0x00 || desc == 0xFF) {
            continue;
        }
        
        /* Process the descriptor */
        process_descriptor(desc, &caches[cache_count], xeon_mp);
        
        /* Only count if we got valid cache info */
        if (caches[cache_count].sizekb > 0) {
            valid_descriptor_found = 1;
            
            printf("Descriptor 0x%02x -> ", desc);
            printf("L%d Cache: %dKB, %d-way, %d-byte line\n",
                   caches[cache_count].level,
                   caches[cache_count].sizekb,
                   caches[cache_count].assoc,
                   caches[cache_count].line);
            
            /* Special handling for case 0x49 */
            if (desc == 0x49) {
                if (xeon_mp) {
                    printf("  (Skipped due to Xeon MP)\n");
                } else {
                    printf("  (Applied non-Xeon MP configuration)\n");
                }
            }
            
            cache_count++;
        }
    }
    
    if (!valid_descriptor_found) {
        printf("No matching cache descriptors found from target list.\n");
        printf("This may be expected if CPU doesn't report those specific descriptors.\n");
        
        /* For testing purposes, manually trigger some cases */
        printf("\nManually testing descriptor processing:\n");
        printf("--------------------------------------\n");
        
        /* Test a few specific cases */
        uint8_t test_descriptors[] = {0x0a, 0x2c, 0x49, 0x66, 0x7a, 0x87};
        
        for (int i = 0; i < sizeof(test_descriptors); i++) {
            process_descriptor(test_descriptors[i], &caches[cache_count], xeon_mp);
            
            if (caches[cache_count].sizekb > 0) {
                printf("Test descriptor 0x%02x -> ", test_descriptors[i]);
                printf("L%d Cache: %dKB, %d-way, %d-byte line\n",
                       caches[cache_count].level,
                       caches[cache_count].sizekb,
                       caches[cache_count].assoc,
                       caches[cache_count].line);
                cache_count++;
            }
        }
    }
    
    /* Step 4: Perform computation using detected cache line sizes */
    printf("\nCache-aware computation:\n");
    printf("------------------------\n");
    
    for (int i = 0; i < cache_count; i++) {
        if (caches[i].line > 0) {
            printf("Using L%d cache line size (%d bytes) for computation...\n",
                   caches[i].level, caches[i].line);
            cache_line_computation(caches[i].line);
        }
    }
    
    /* If no valid cache line sizes, use a default */
    if (cache_count == 0) {
        printf("Using default cache line size (64 bytes) for computation...\n");
        cache_line_computation(64);
    }
    
    printf("\n=== Program completed successfully ===\n");
    
    return 0;
}
