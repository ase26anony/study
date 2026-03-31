/*
 * This program is designed to trigger the specific cache descriptor parsing
 * logic in driver-i386.cc lines 127-244 by executing CPUID leaf 0x2 and
 * processing the returned cache descriptor bytes according to Intel's
 * specification.
 */

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
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int level;      /* Cache level (1 or 2) */
    int type;       /* Cache type */
};

/* Global flag to simulate xeon_mp condition */
static int xeon_mp = 0;

/* Parse CPUID leaf 0x1 to determine if we're on Xeon MP */
static void check_xeon_mp(void) {
    uint32_t regs[4];
    cpuid(regs, 0x1);
    
    /* Extract family, model, stepping */
    uint32_t stepping = regs[0] & 0xF;
    uint32_t model = (regs[0] >> 4) & 0xF;
    uint32_t family = (regs[0] >> 8) & 0xF;
    uint32_t extended_model = (regs[0] >> 16) & 0xF;
    uint32_t extended_family = (regs[0] >> 20) & 0xFF;
    
    /* Adjust for extended family/model */
    if (family == 0xF) {
        family += extended_family;
        model += (extended_model << 4);
    }
    
    /* Simple heuristic for Xeon MP: Family 15, Model >= 2 */
    if (family == 15 && model >= 2) {
        xeon_mp = 1;
    }
    
    /* Alternative check: look for "Intel Xeon MP" in brand string */
    uint32_t maxleaf;
    cpuid(&maxleaf, 0);
    if (maxleaf >= 0x80000004) {
        char brand[49] = {0};
        uint32_t brand_regs[4];
        
        cpuid(brand_regs, 0x80000002);
        memcpy(brand, brand_regs, 16);
        cpuid(brand_regs, 0x80000003);
        memcpy(brand + 16, brand_regs, 16);
        cpuid(brand_regs, 0x80000004);
        memcpy(brand + 32, brand_regs, 16);
        
        if (strstr(brand, "Xeon") && strstr(brand, "MP")) {
            xeon_mp = 1;
        }
    }
}

/* Process a single cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1, 
                               struct cache_desc *level2, int *l1_count, 
                               int *l2_count) {
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            level1[*l1_count].sizekb = 8; level1[*l1_count].assoc = 2; 
            level1[*l1_count].line = 32; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
        case 0x0c:
            level1[*l1_count].sizekb = 16; level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 32; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
        case 0x0d:
            level1[*l1_count].sizekb = 16; level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 64; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
        case 0x0e:
            level1[*l1_count].sizekb = 24; level1[*l1_count].assoc = 6; 
            level1[*l1_count].line = 64; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
        case 0x2c:
            level1[*l1_count].sizekb = 32; level1[*l1_count].assoc = 8; 
            level1[*l1_count].line = 64; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
        case 0x60:
            level1[*l1_count].sizekb = 16; level1[*l1_count].assoc = 8; 
            level1[*l1_count].line = 64; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
        case 0x66:
            level1[*l1_count].sizekb = 8; level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 64; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
        case 0x67:
            level1[*l1_count].sizekb = 16; level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 64; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
        case 0x68:
            level1[*l1_count].sizekb = 32; level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 64; level1[*l1_count].level = 1;
            (*l1_count)++;
            break;
            
        /* L2 Cache cases */
        case 0x21:
            level2[*l2_count].sizekb = 256; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x24:
            level2[*l2_count].sizekb = 1024; level2[*l2_count].assoc = 16; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x39:
            level2[*l2_count].sizekb = 128; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x3a:
            level2[*l2_count].sizekb = 192; level2[*l2_count].assoc = 6; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x3b:
            level2[*l2_count].sizekb = 128; level2[*l2_count].assoc = 2; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x3c:
            level2[*l2_count].sizekb = 256; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x3d:
            level2[*l2_count].sizekb = 384; level2[*l2_count].assoc = 6; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x3e:
            level2[*l2_count].sizekb = 512; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x41:
            level2[*l2_count].sizekb = 128; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x42:
            level2[*l2_count].sizekb = 256; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x43:
            level2[*l2_count].sizekb = 512; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x44:
            level2[*l2_count].sizekb = 1024; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x45:
            level2[*l2_count].sizekb = 2048; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x48:
            level2[*l2_count].sizekb = 3072; level2[*l2_count].assoc = 12; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x49:
            /* Special case with xeon_mp check */
            if (!xeon_mp) {
                level2[*l2_count].sizekb = 4096; level2[*l2_count].assoc = 16; 
                level2[*l2_count].line = 64; level2[*l2_count].level = 2;
                (*l2_count)++;
            }
            break;
        case 0x4e:
            level2[*l2_count].sizekb = 6144; level2[*l2_count].assoc = 24; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x78:
            level2[*l2_count].sizekb = 1024; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x79:
            level2[*l2_count].sizekb = 128; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x7a:
            level2[*l2_count].sizekb = 256; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x7b:
            level2[*l2_count].sizekb = 512; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x7c:
            level2[*l2_count].sizekb = 1024; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x7d:
            level2[*l2_count].sizekb = 2048; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x7f:
            level2[*l2_count].sizekb = 512; level2[*l2_count].assoc = 2; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x80:
            level2[*l2_count].sizekb = 512; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x82:
            level2[*l2_count].sizekb = 256; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x83:
            level2[*l2_count].sizekb = 512; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x84:
            level2[*l2_count].sizekb = 1024; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x85:
            level2[*l2_count].sizekb = 2048; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 32; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x86:
            level2[*l2_count].sizekb = 512; level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
        case 0x87:
            level2[*l2_count].sizekb = 1024; level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64; level2[*l2_count].level = 2;
            (*l2_count)++;
            break;
            
        /* Valid descriptor but not in our target list - ignore */
        case 0x00: /* Null descriptor - terminator */
        case 0x01: /* TLB descriptors */
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
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
            /* Valid but not targeted - do nothing */
            break;
            
        default:
            /* Invalid or reserved descriptor */
            break;
    }
}

/* Extract and process cache descriptors from CPUID leaf 0x2 */
static void get_cache_descriptors(struct cache_desc *level1, 
                                  struct cache_desc *level2, 
                                  int *l1_count, int *l2_count) {
    uint32_t regs[4];
    uint8_t descriptors[16];
    int iteration = 0;
    int max_iterations = 10; /* Safety limit */
    
    *l1_count = 0;
    *l2_count = 0;
    
    /* According to Intel spec, CPUID leaf 0x2 may need to be called multiple times */
    while (iteration < max_iterations) {
        cpuid(regs, 0x2);
        
        /* Extract descriptor bytes from registers */
        descriptors[0] = (regs[0] >> 0) & 0xFF;
        descriptors[1] = (regs[0] >> 8) & 0xFF;
        descriptors[2] = (regs[0] >> 16) & 0xFF;
        descriptors[3] = (regs[0] >> 24) & 0xFF;
        descriptors[4] = (regs[1] >> 0) & 0xFF;
        descriptors[5] = (regs[1] >> 8) & 0xFF;
        descriptors[6] = (regs[1] >> 16) & 0xFF;
        descriptors[7] = (regs[1] >> 24) & 0xFF;
        descriptors[8] = (regs[2] >> 0) & 0xFF;
        descriptors[9] = (regs[2] >> 8) & 0xFF;
        descriptors[10] = (regs[2] >> 16) & 0xFF;
        descriptors[11] = (regs[2] >> 24) & 0xFF;
        descriptors[12] = (regs[3] >> 0) & 0xFF;
        descriptors[13] = (regs[3] >> 8) & 0xFF;
        descriptors[14] = (regs[3] >> 16) & 0xFF;
        descriptors[15] = (regs[3] >> 24) & 0xFF;
        
        /* Process each descriptor byte */
        for (int i = 0; i < 16; i++) {
            uint8_t desc = descriptors[i];
            
            /* Check for valid descriptor (bit 31 of EAX indicates validity) */
            if (iteration == 0 && i == 0 && (regs[0] & 0x80000000)) {
                /* First byte is valid flag, not a descriptor */
                continue;
            }
            
            /* Terminator byte indicates end of descriptors */
            if (desc == 0x00) {
                return;
            }
            
            /* Process the descriptor */
            process_descriptor(desc, level1, level2, l1_count, l2_count);
            
            /* Safety check to avoid buffer overflow */
            if (*l1_count >= 10 || *l2_count >= 10) {
                return;
            }
        }
        
        iteration++;
    }
}

/* Simple computation using cache line size to prevent optimization */
static void cache_line_optimized_computation(int line_size) {
    volatile int result = 0;
    int array_size = 1024 * 1024; /* 1MB */
    char *buffer = (char*)malloc(array_size);
    
    if (buffer) {
        /* Align access to cache line boundary */
        uintptr_t addr = (uintptr_t)buffer;
        uintptr_t aligned_addr = (addr + line_size - 1) & ~(line_size - 1);
        char *aligned_buffer = (char*)aligned_addr;
        
        /* Perform computation that uses cache line size */
        for (int i = 0; i < array_size - line_size; i += line_size) {
            aligned_buffer[i] = (char)(i & 0xFF);
            result += aligned_buffer[i];
        }
        
        free(buffer);
    }
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("."); /* Minimal side effect */
    }
}

int main(void) {
    struct cache_desc level1[10];
    struct cache_desc level2[10];
    int l1_count, l2_count;
    
    printf("Starting cache descriptor detection...\n");
    
    /* Step 1: Check for Xeon MP condition */
    check_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Get cache descriptors */
    get_cache_descriptors(level1, level2, &l1_count, &l2_count);
    
    /* Step 3: Print results */
    printf("\nDetected L1 Caches (%d):\n", l1_count);
    for (int i = 0; i < l1_count; i++) {
        printf("  L1[%d]: %dKB, %d-way, %d-byte line\n", 
               i, level1[i].sizekb, level1[i].assoc, level1[i].line);
    }
    
    printf("\nDetected L2 Caches (%d):\n", l2_count);
    for (int i = 0; i < l2_count; i++) {
        printf("  L2[%d]: %dKB, %d-way, %d-byte line\n", 
               i, level2[i].sizekb, level2[i].assoc, level2[i].line);
    }
    
    /* Special note for case 0x49 */
    printf("\nNote: Case 0x49 (4MB L2) %s processed\n", 
           xeon_mp ? "was NOT" : "was");
    
    /* Step 4: Perform computation using cache line size */
    if (l1_count > 0) {
        cache_line_optimized_computation(level1[0].line);
    } else if (l2_count > 0) {
        cache_line_optimized_computation(level2[0].line);
    } else {
        cache_line_optimized_computation(64); /* Default */
    }
    
    printf("\nCache detection complete.\n");
    
    return 0;
}
