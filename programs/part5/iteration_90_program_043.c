```c
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
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc logic */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int level;      /* Cache level (1 or 2) */
    int type;       /* Cache type */
    int valid;      /* Whether this entry is valid */
};

/* Global to simulate xeon_mp check */
static int xeon_mp = 0;

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

/* Check if CPU is Xeon MP based on CPUID leaf 0x1 */
static void check_xeon_mp(void) {
    uint32_t eax, ebx, ecx, edx;
    
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Simple heuristic for Xeon MP:
       Family 0xF (Pentium 4/Xeon), Model 0x3, 0x4, or stepping checks */
    if (family == 0xF && (model == 0x3 || model == 0x4)) {
        /* Check if it's a Xeon (bit 28 of edx from leaf 0x1) */
        if (edx & (1 << 28)) {
            /* Additional check for MP capability */
            uint32_t eax2, ebx2, ecx2, edx2;
            cpuid(0x0, 0, &eax2, &ebx2, &ecx2, &edx2);
            
            /* Check for "GenuineIntel" */
            if (memcmp(&ebx2, "Genu", 4) == 0 &&
                memcmp(&edx2, "ineI", 4) == 0 &&
                memcmp(&ecx2, "ntel", 4) == 0) {
                xeon_mp = 1;
            }
        }
    }
    
    printf("CPU Info: Family=%u, Model=%u, Stepping=%u, Xeon_MP=%d\n",
           family, model, stepping, xeon_mp);
}

/* Process a single cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc* level1, 
                               struct cache_desc* level2, int* l1_count, 
                               int* l2_count) {
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            level1[*l1_count].sizekb = 8; 
            level1[*l1_count].assoc = 2; 
            level1[*l1_count].line = 32;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1; /* Data cache */
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 8, 2, 32);
            break;
            
        case 0x0c:
            level1[*l1_count].sizekb = 16; 
            level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 32;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1;
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 16, 4, 32);
            break;
            
        case 0x0d:
            level1[*l1_count].sizekb = 16; 
            level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 64;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1;
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 16, 4, 64);
            break;
            
        case 0x0e:
            level1[*l1_count].sizekb = 24; 
            level1[*l1_count].assoc = 6; 
            level1[*l1_count].line = 64;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1;
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 24, 6, 64);
            break;
            
        case 0x2c:
            level1[*l1_count].sizekb = 32; 
            level1[*l1_count].assoc = 8; 
            level1[*l1_count].line = 64;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1;
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 32, 8, 64);
            break;
            
        case 0x60:
            level1[*l1_count].sizekb = 16; 
            level1[*l1_count].assoc = 8; 
            level1[*l1_count].line = 64;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1;
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 16, 8, 64);
            break;
            
        case 0x66:
            level1[*l1_count].sizekb = 8; 
            level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 64;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1;
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 8, 4, 64);
            break;
            
        case 0x67:
            level1[*l1_count].sizekb = 16; 
            level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 64;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1;
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 16, 4, 64);
            break;
            
        case 0x68:
            level1[*l1_count].sizekb = 32; 
            level1[*l1_count].assoc = 4; 
            level1[*l1_count].line = 64;
            level1[*l1_count].level = 1;
            level1[*l1_count].type = 1;
            level1[*l1_count].valid = 1;
            (*l1_count)++;
            printf("  Found L1 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 32, 4, 64);
            break;
            
        /* L2 Cache cases */
        case 0x21:
            level2[*l2_count].sizekb = 256; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3; /* Unified cache */
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 256, 8, 64);
            break;
            
        case 0x24:
            level2[*l2_count].sizekb = 1024; 
            level2[*l2_count].assoc = 16; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 1024, 16, 64);
            break;
            
        case 0x39:
            level2[*l2_count].sizekb = 128; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 128, 4, 64);
            break;
            
        case 0x3a:
            level2[*l2_count].sizekb = 192; 
            level2[*l2_count].assoc = 6; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 192, 6, 64);
            break;
            
        case 0x3b:
            level2[*l2_count].sizekb = 128; 
            level2[*l2_count].assoc = 2; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 128, 2, 64);
            break;
            
        case 0x3c:
            level2[*l2_count].sizekb = 256; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 256, 4, 64);
            break;
            
        case 0x3d:
            level2[*l2_count].sizekb = 384; 
            level2[*l2_count].assoc = 6; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 384, 6, 64);
            break;
            
        case 0x3e:
            level2[*l2_count].sizekb = 512; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 512, 4, 64);
            break;
            
        case 0x41:
            level2[*l2_count].sizekb = 128; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 128, 4, 32);
            break;
            
        case 0x42:
            level2[*l2_count].sizekb = 256; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 256, 4, 32);
            break;
            
        case 0x43:
            level2[*l2_count].sizekb = 512; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 512, 4, 32);
            break;
            
        case 0x44:
            level2[*l2_count].sizekb = 1024; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 1024, 4, 32);
            break;
            
        case 0x45:
            level2[*l2_count].sizekb = 2048; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 2048, 4, 32);
            break;
            
        case 0x48:
            level2[*l2_count].sizekb = 3072; 
            level2[*l2_count].assoc = 12; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 3072, 12, 64);
            break;
            
        case 0x49:
            /* Special case with xeon_mp check */
            if (!xeon_mp) {
                level2[*l2_count].sizekb = 4096; 
                level2[*l2_count].assoc = 16; 
                level2[*l2_count].line = 64;
                level2[*l2_count].level = 2;
                level2[*l2_count].type = 3;
                level2[*l2_count].valid = 1;
                (*l2_count)++;
                printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line (Xeon_MP=%d)\n", 
                       desc, 4096, 16, 64, xeon_mp);
            } else {
                printf("  Skipped L2 Cache 0x49 due to Xeon_MP=%d\n", xeon_mp);
            }
            break;
            
        case 0x4e:
            level2[*l2_count].sizekb = 6144; 
            level2[*l2_count].assoc = 24; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 6144, 24, 64);
            break;
            
        case 0x78:
            level2[*l2_count].sizekb = 1024; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 1024, 4, 64);
            break;
            
        case 0x79:
            level2[*l2_count].sizekb = 128; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 128, 8, 64);
            break;
            
        case 0x7a:
            level2[*l2_count].sizekb = 256; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 256, 8, 64);
            break;
            
        case 0x7b:
            level2[*l2_count].sizekb = 512; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 512, 8, 64);
            break;
            
        case 0x7c:
            level2[*l2_count].sizekb = 1024; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 1024, 8, 64);
            break;
            
        case 0x7d:
            level2[*l2_count].sizekb = 2048; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 2048, 8, 64);
            break;
            
        case 0x7f:
            level2[*l2_count].sizekb = 512; 
            level2[*l2_count].assoc = 2; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 512, 2, 64);
            break;
            
        case 0x80:
            level2[*l2_count].sizekb = 512; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 512, 8, 64);
            break;
            
        case 0x82:
            level2[*l2_count].sizekb = 256; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 256, 8, 32);
            break;
            
        case 0x83:
            level2[*l2_count].sizekb = 512; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 512, 8, 32);
            break;
            
        case 0x84:
            level2[*l2_count].sizekb = 1024; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 1024, 8, 32);
            break;
            
        case 0x85:
            level2[*l2_count].sizekb = 2048; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 32;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 2048, 8, 32);
            break;
            
        case 0x86:
            level2[*l2_count].sizekb = 512; 
            level2[*l2_count].assoc = 4; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 512, 4, 64);
            break;
            
        case 0x87:
            level2[*l2_count].sizekb = 1024; 
            level2[*l2_count].assoc = 8; 
            level2[*l2_count].line = 64;
            level2[*l2_count].level = 2;
            level2[*l2_count].type = 3;
            level2[*l2_count].valid = 1;
            (*l2_count)++;
            printf("  Found L2 Cache: 0x%02x -> %dKB, %d-way, %dB line\n", 
                   desc, 1024, 8, 64);
            break;
            
        /* Valid descriptor but not in our target list - still process */
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
        case 0x90:
        case 0x96:
        case 0x9b:
            /* Valid cache descriptors but not in our target lines */
            printf("  Other valid descriptor: 0x%02x (not in target lines)\n", desc);
            break;
            
        default:
            /* Invalid or reserved descriptor */
            if (desc & 0x80) {
                /* This is a valid descriptor byte */
                printf("  Valid descriptor (MSB set): 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract and process cache descriptors from CPUID leaf 0x2 */
static void get_cache_descriptors(struct cache_desc* level1, 
                                  struct cache_desc* level2,
                                  int* l1_count, int* l2_count) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int iteration = 0;
    int max_iterations = 10; /* Safety limit */
    
    printf("\nCPUID Leaf 0x2 Cache Descriptor Scan:\n");
    
    /* According to Intel spec, CPUID leaf 0x2 may need to be called multiple times */
    while (iteration < max_iterations) {
        cpuid(0x2, iteration, &eax, &ebx, &ecx, &edx);
        
        /* AL register contains the number of times CPUID with EAX=2 should be called */
        uint8_t call_count = eax & 0xFF;
        
        /* Extract descriptor bytes from registers */
        descriptors[0] = (eax >> 8) & 0xFF;
        descriptors[1] = (eax >> 16) & 0xFF;
        descriptors[2] = (eax >> 24) & 0xFF;
        descriptors[3] = ebx & 0xFF;
        descriptors[4] = (ebx >> 8) & 0xFF;
        descriptors[5] = (ebx >> 16) & 0xFF;
        descriptors[6] = (ebx >> 24) & 0xFF;
        descriptors[7] = ecx & 0xFF;
        descriptors[8] = (ecx >> 8) & 0xFF;
        descriptors[9] = (ecx >> 16) & 0xFF;
        descriptors[10] = (ecx >> 24) & 0xFF;
        descriptors[11] = edx & 0xFF;
        descriptors[12] = (edx >> 8) & 0xFF;
        descriptors[13] = (edx >> 16) & 0xFF;
        descriptors[14] = (edx >> 24) & 0xFF;
        
        printf("Iteration %d: EAX=0x%08x, EBX=0x%08x, ECX=0x%08x, EDX=0x%08x\n",
               iteration, eax, ebx, ecx, edx);
        
        /* Process each descriptor byte */
        for (int i = 0; i < 15; i++) {
            if (descriptors[i] != 0x00) {
                process_descriptor(descriptors[i], level1, level2, l1_count, l2_count);
            }
        }
        
        iteration++;
        
        /* Check termination condition */
        if (call_count == 0 || iteration >= call_count) {
            break;
        }
    }
}

/* Use cache line size information in a computation to prevent optimization */
static void cache_aware_computation(struct cache_desc* level1, int l1_count,
                                    struct cache_desc* level2, int l2_count) {
    int total_size = 0;
    int line_size = 64; /* Default */
    
    /* Find the smallest cache line size */
    for (int i = 0; i < l1_count; i++) {
        if (level1[i].valid && level1[i].line > 0) {
            if (level1[i].line < line_size) {
                line_size = level1[i].line;
            }
        }
    }
    
    for (int i = 0; i < l2_count; i++) {
        if (level2[i].valid && level2[i].line > 0) {
            if (level2[i].line < line_size) {
                line_size = level2[i].line;
            }
        }
    }
    
    /* Create an array with cache line alignment */
    int array_size = 1024 * 1024; /* 1MB */
    char* buffer = (char*)malloc(array_size + line_size);
    
    if (!buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Align to cache line boundary */
    uintptr_t addr = (uintptr_t)buffer;
    uintptr_t offset = addr % line_size;
    if (offset != 0) {
        addr += line_size - offset;
    }
    char* aligned_buffer = (char*)addr;
    
    /* Perform computation using cache line size */
    volatile int sum = 0;
    for (int i = 0; i < array_size; i += line_size) {
        aligned_buffer[i] = (char)(i & 0xFF);
        sum += aligned_buffer[i];
    }
    
    printf("\nCache-aware computation completed:\n");
    printf("  Used cache line size: %d bytes\n", line_size);
    printf("  Computed sum: %d\n", sum);
    
    free(buffer);
}

int main(void) {
    struct cache_desc level1[10];  /* L1 caches */
    struct cache_desc level2[10];  /* L2 caches */
    int l1_count = 0;
    int l2_count = 0;
    
    /* Initialize cache structures */
    memset(level1, 0, sizeof(level1));
    memset(level2, 0, sizeof(level2));
    
    printf("=== CPU Cache Descriptor Detection Program ===\n");
    
    /* Step 1: Check for Xeon MP */
    check_xeon_mp();
    
    /* Step 2: Get cache descriptors via CPUID leaf 0x2 */
    get_cache_descriptors(level1, level2, &l1_count, &l2_count);
    
    /* Step 3: Print summary */
    printf("\n=== Cache Summary ===\n");
    printf("L1 Caches found: %d\n", l1_count);
    for (int i = 0; i < l1_count; i++) {
        if (level1[i].valid) {
            printf("  L1[%d]: %dKB, %d-way, %dB line\n",
                   i, level1[i].sizekb, level1[i].assoc, level1[i].line);
        }
    }
    
    printf("L2 Caches found: %d\n", l2_count);
    for (int i = 0; i < l2_count; i++) {
        if (level2[i].valid)
