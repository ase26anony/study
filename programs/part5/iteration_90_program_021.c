#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#define cpuid(info, x) __cpuid(info, x)
#define cpuidex(info, x, y) __cpuidex(info, x, y)
#else
#include <cpuid.h>
#define cpuid(info, x) __cpuid(x, info[0], info[1], info[2], info[3])
#define cpuidex(info, x, y) __cpuid_count(x, y, info[0], info[1], info[2], info[3])
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
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
    
    /* Adjust for extended family/model */
    if (family == 0xF) {
        family += extended_family;
        model += (extended_model << 4);
    }
    
    /* Check for Xeon MP characteristics:
       - Family 6, Model 44 (Nehalem-EX), 46 (Westmere-EX)
       - This is a simplified check; real detection would be more complex */
    if (family == 6) {
        if (model == 44 || model == 46) {
            return 1; /* Likely Xeon MP */
        }
    }
    
    return 0;
}

/* Process a single cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1, 
                               struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("L1 Cache: 8KB, 2-way, 32-byte line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("L1 Cache: 16KB, 4-way, 32-byte line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64-byte line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("L1 Cache: 24KB, 6-way, 64-byte line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 1024KB, 16-way, 64-byte line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 32KB, 8-way, 64-byte line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 128KB, 4-way, 64-byte line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 192KB, 6-way, 64-byte line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 128KB, 2-way, 64-byte line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 256KB, 4-way, 64-byte line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 384KB, 6-way, 64-byte line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64-byte line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 128KB, 4-way, 32-byte line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 256KB, 4-way, 32-byte line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 512KB, 4-way, 32-byte line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 1024KB, 4-way, 32-byte line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 2048KB, 4-way, 32-byte line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("L2 Cache: 3072KB, 12-way, 64-byte line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping L2 cache update\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 4096KB, 16-way, 64-byte line (non-Xeon MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("L2 Cache: 6144KB, 24-way, 64-byte line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 16KB, 8-way, 64-byte line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 8KB, 4-way, 64-byte line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64-byte line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 32KB, 4-way, 64-byte line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 1024KB, 4-way, 64-byte line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 128KB, 8-way, 64-byte line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64-byte line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64-byte line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 2048KB, 8-way, 64-byte line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 512KB, 2-way, 64-byte line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64-byte line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 256KB, 8-way, 32-byte line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 512KB, 8-way, 32-byte line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 1024KB, 8-way, 32-byte line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 2048KB, 8-way, 32-byte line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64-byte line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64-byte line\n");
            break;
        default:
            /* Ignore other descriptor values */
            break;
    }
}

int main(void) {
    uint32_t info[4];
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    printf("=== CPU Cache Detection Program ===\n");
    
    /* Step 1: Get basic CPU info and check for Xeon MP */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times to get cache descriptors */
    printf("\nProcessing CPUID leaf 0x2 cache descriptors:\n");
    
    /* According to Intel manual, CPUID with EAX=2 returns cache descriptors
       in EAX, EBX, ECX, EDX. The first byte of EAX indicates number of times
       CPUID with EAX=2 should be called. */
    cpuid(info, 0x00000002);
    
    uint8_t times_to_call = info[0] & 0xFF;
    if (times_to_call == 0) {
        times_to_call = 1; /* At least call once */
    }
    
    /* Process all descriptor bytes from all calls */
    for (int call_num = 0; call_num < times_to_call; call_num++) {
        if (call_num > 0) {
            /* Subsequent calls with EAX=2 */
            cpuid(info, 0x00000002);
        }
        
        /* Process each register as an array of bytes */
        uint8_t *reg_bytes = (uint8_t *)info;
        
        for (int i = 0; i < 16; i++) { /* 4 registers * 4 bytes = 16 bytes */
            uint8_t desc = reg_bytes[i];
            
            /* Skip invalid/reserved bytes */
            if (desc == 0x00) {
                continue; /* Valid terminator, but we continue to check others */
            }
            
            /* Skip if bit 7 is set (this is a register indicator, not descriptor) */
            if (desc & 0x80) {
                continue;
            }
            
            /* Process the descriptor byte */
            process_descriptor(desc, &level1, &level2, xeon_mp);
        }
    }
    
    /* Step 3: Print final cache information */
    printf("\n=== Final Cache Configuration ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Step 4: Perform computation using cache line size to prevent optimization */
    printf("\n=== Cache-Aware Computation ===\n");
    int cache_line = level1.line > 0 ? level1.line : 64; /* Default to 64 if not detected */
    int array_size = cache_line * 1024; /* Allocate multiple cache lines */
    
    volatile int *buffer = (volatile int *)aligned_alloc(cache_line, array_size);
    if (buffer) {
        /* Access memory with cache line alignment */
        for (int i = 0; i < array_size / sizeof(int); i += cache_line / sizeof(int)) {
            buffer[i] = i;
        }
        
        /* Sum to ensure computation isn't optimized away */
        int sum = 0;
        for (int i = 0; i < array_size / sizeof(int); i += cache_line / sizeof(int)) {
            sum += buffer[i];
        }
        
        printf("Cache-line aligned computation complete. Sum: %d\n", sum);
        printf("Used cache line size: %d bytes\n", cache_line);
        
        free((void *)buffer);
    }
    
    return 0;
}
