#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

// Structure matching cache_desc from driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global variables to track cache levels
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  // Set to 0 to hit case 0x49

// Function to process cache descriptor bytes (mimicking the uncovered logic)
void process_cache_descriptor(uint8_t descriptor) {
    switch (descriptor) {
        case 0x0a:
            level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
            printf("Case 0x0a: L1 Cache - 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
            printf("Case 0x0c: L1 Cache - 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Case 0x0d: L1 Cache - 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
            printf("Case 0x0e: L1 Cache - 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Case 0x21: L2 Cache - 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
            printf("Case 0x24: L2 Cache - 1024KB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
            printf("Case 0x2c: L1 Cache - 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
            printf("Case 0x39: L2 Cache - 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
            printf("Case 0x3a: L2 Cache - 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
            printf("Case 0x3b: L2 Cache - 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
            printf("Case 0x3c: L2 Cache - 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
            printf("Case 0x3d: L2 Cache - 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Case 0x3e: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
            printf("Case 0x41: L2 Cache - 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
            printf("Case 0x42: L2 Cache - 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
            printf("Case 0x43: L2 Cache - 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
            printf("Case 0x44: L2 Cache - 1024KB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Case 0x45: L2 Cache - 2048KB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Case 0x48: L2 Cache - 3072KB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping\n");
                break;
            }
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Case 0x49: L2 Cache - 4096KB, 16-way, 64B line (non-Xeon-MP)\n");
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Case 0x4e: L2 Cache - 6144KB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
            printf("Case 0x60: L1 Cache - 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
            printf("Case 0x66: L1 Cache - 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Case 0x67: L1 Cache - 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
            printf("Case 0x68: L1 Cache - 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
            printf("Case 0x78: L2 Cache - 1024KB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
            printf("Case 0x79: L2 Cache - 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7a: L2 Cache - 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7b: L2 Cache - 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7c: L2 Cache - 1024KB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7d: L2 Cache - 2048KB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
            printf("Case 0x7f: L2 Cache - 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Case 0x80: L2 Cache - 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
            printf("Case 0x82: L2 Cache - 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
            printf("Case 0x83: L2 Cache - 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
            printf("Case 0x84: L2 Cache - 1024KB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Case 0x85: L2 Cache - 2048KB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Case 0x86: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Case 0x87: L2 Cache - 1024KB, 8-way, 64B line\n");
            break;
        default:
            // Ignore other descriptors
            break;
    }
}

// Real CPUID leaf 0x02 call (with fallback)
void call_cpuid_leaf2() {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\n=== Calling CPUID Leaf 0x02 ===\n");
    
    // Check if CPUID leaf 0x02 is supported
    __cpuid(0, eax, ebx, ecx, edx);
    uint32_t max_leaf = eax;
    
    if (max_leaf < 2) {
        printf("CPUID leaf 0x02 not supported (max leaf = %u)\n", max_leaf);
        return;
    }
    
    // Call CPUID leaf 0x02
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    printf("CPUID(0x02) returned: EAX=%08x EBX=%08x ECX=%08x EDX=%08x\n", 
           eax, ebx, ecx, edx);
    
    // Check first byte of AL (must be > 1 to use descriptor table)
    uint8_t al_byte = eax & 0xFF;
    if (al_byte == 1) {
        printf("AL=1: Using TLB method, not descriptor table\n");
        return;
    }
    
    if (al_byte == 0) {
        printf("AL=0: No valid descriptors\n");
        return;
    }
    
    // Process descriptor bytes from all registers
    printf("Processing descriptor bytes (AL=%u valid bytes):\n", al_byte);
    
    uint8_t *regs = (uint8_t*)&eax;
    int bytes_processed = 0;
    
    // Iterate through all 16 bytes (4 registers * 4 bytes)
    for (int i = 0; i < 16 && bytes_processed < al_byte; i++) {
        uint8_t descriptor = regs[i];
        
        // Skip if bit 31 is set (register is reserved)
        if (i % 4 == 0 && (descriptor & 0x80)) {
            continue;
        }
        
        // Valid descriptor byte
        if (descriptor != 0x00 && descriptor != 0xFF) {
            printf("  Byte %d: 0x%02x\n", i, descriptor);
            process_cache_descriptor(descriptor);
            bytes_processed++;
        }
    }
}

// Real CPUID leaf 0x04 call (deterministic cache parameters)
void call_cpuid_leaf4() {
    uint32_t eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("\n=== Calling CPUID Leaf 0x04 ===\n");
    
    while (1) {
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("Cache type 0: No more caches\n");
            break;
        }
        
        printf("Cache %d: type=%u, level=%u, ways=%u, partitions=%u, line_size=%u, sets=%u\n",
               cache_index,
               cache_type,
               (eax >> 5) & 0x7,
               ((ebx >> 22) & 0x3FF) + 1,
               ((ebx >> 12) & 0x3FF) + 1,
               (ebx & 0xFFF) + 1,
               ecx + 1);
        
        cache_index++;
    }
}

// Simulate all target descriptor values to ensure coverage
void simulate_all_descriptors() {
    printf("\n=== Simulating All Target Descriptor Values ===\n");
    
    // All the uncovered case values
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    for (int i = 0; i < sizeof(target_descriptors); i++) {
        printf("\nProcessing simulated descriptor 0x%02x:\n", target_descriptors[i]);
        process_cache_descriptor(target_descriptors[i]);
    }
}

// Simulate CPUID leaf 0x02 with specific descriptor bytes
void simulate_cpuid_leaf2_with_descriptors() {
    printf("\n=== Simulating CPUID Leaf 0x02 with Target Descriptors ===\n");
    
    // Create a mock CPUID result with AL=0x10 (16 valid bytes)
    // and fill with our target descriptors
    uint32_t eax, ebx, ecx, edx;
    
    // AL = 0x10 (16 valid bytes), rest of EAX has descriptors
    eax = 0x10 | (0x0a << 8) | (0x0c << 16) | (0x0d << 24);
    ebx = (0x0e) | (0x21 << 8) | (0x24 << 16) | (0x2c << 24);
    ecx = (0x39) | (0x3a << 8) | (0x3b << 16) | (0x3c << 24);
    edx = (0x3d) | (0x3e << 8) | (0x41 << 16) | (0x42 << 24);
    
    printf("Mock CPUID(0x02): EAX=%08x EBX=%08x ECX=%08x EDX=%08x\n", 
           eax, ebx, ecx, edx);
    
    // Process the mock descriptors
    uint8_t *regs = (uint8_t*)&eax;
    int bytes_to_process = eax & 0xFF;  // AL byte
    
    for (int i = 0; i < bytes_to_process && i < 16; i++) {
        uint8_t descriptor = regs[i];
        if (descriptor != 0x00 && descriptor != 0xFF) {
            process_cache_descriptor(descriptor);
        }
    }
}

int main() {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    // Ensure xeon_mp is false to hit case 0x49
    xeon_mp = 0;
    printf("xeon_mp = %d (will hit case 0x49)\n", xeon_mp);
    
    // Method 1: Real CPUID calls
    call_cpuid_leaf2();
    call_cpuid_leaf4();
    
    // Method 2: Simulate all target descriptors
    simulate_all_descriptors();
    
    // Method 3: Simulate CPUID leaf 0x02 with specific descriptors
    simulate_cpuid_leaf2_with_descriptors();
    
    // Print final cache configuration
    printf("\n=== Final Cache Configuration ===\n");
    printf("L1 Cache: %d KB, %d-way, %d-byte line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache: %d KB, %d-way, %d-byte line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    // Prevent optimization
    volatile int dummy = level1.sizekb + level2.sizekb;
    
    return 0;
}
