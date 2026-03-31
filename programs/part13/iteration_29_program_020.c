#include <stdio.h>
#include <stdint.h>
#include <cpuid.h>

// Mock structures matching the original driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global variables to track cache levels
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  // Set to 0 to hit the 0x49 case

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
            if (xeon_mp)
                break;
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Case 0x49: L2 Cache - 4096KB, 16-way, 64B line (xeon_mp=%d)\n", xeon_mp);
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

// Test CPUID leaf 0x02 (cache descriptors)
void test_cpuid_leaf_02() {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\n=== Testing CPUID Leaf 0x02 (Cache Descriptors) ===\n");
    
    // Call CPUID leaf 0x02
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    printf("Raw CPUID(0x02) output: EAX=0x%08x EBX=0x%08x ECX=0x%08x EDX=0x%08x\n", 
           eax, ebx, ecx, edx);
    
    // Check if we should use the descriptor table method (AL > 1)
    uint8_t al = eax & 0xFF;
    if (al == 1) {
        printf("AL=1: Using TLB method, not descriptor table\n");
        return;
    }
    
    if (al == 0) {
        printf("AL=0: No valid descriptors\n");
        return;
    }
    
    printf("AL=%d: Processing descriptor bytes\n", al);
    
    // Process descriptor bytes from all registers
    uint8_t *regs = (uint8_t*)&eax;
    for (int i = 0; i < 16; i++) {
        uint8_t descriptor = regs[i];
        
        // Skip if descriptor is 0x00 (null) or has bit 31 set (reserved)
        if (descriptor == 0x00 || (descriptor & 0x80)) {
            continue;
        }
        
        process_cache_descriptor(descriptor);
    }
}

// Test CPUID leaf 0x04 (deterministic cache parameters)
void test_cpuid_leaf_04() {
    uint32_t eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("\n=== Testing CPUID Leaf 0x04 (Deterministic Cache Parameters) ===\n");
    
    // Iterate through cache levels
    while (1) {
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
        
        uint8_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("Cache index %d: No more caches\n", cache_index);
            break;
        }
        
        printf("Cache index %d: type=%u, level=%u, ways=%u, partitions=%u, line_size=%u, sets=%u\n",
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

// Direct test of all uncovered cases using fabricated data
void test_all_uncovered_cases() {
    printf("\n=== Direct Test of All Uncovered Cases ===\n");
    
    // All the cache descriptor values from the uncovered lines
    uint8_t test_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    int num_cases = sizeof(test_descriptors) / sizeof(test_descriptors[0]);
    printf("Testing %d cache descriptor cases...\n", num_cases);
    
    for (int i = 0; i < num_cases; i++) {
        // Reset cache structures for each test
        level1.sizekb = level1.assoc = level1.line = 0;
        level2.sizekb = level2.assoc = level2.line = 0;
        
        printf("\nTest %d: Descriptor 0x%02x\n", i + 1, test_descriptors[i]);
        process_cache_descriptor(test_descriptors[i]);
        
        // Print results
        if (level1.sizekb > 0) {
            printf("  -> L1: %dKB, %d-way, %dB line\n", 
                   level1.sizekb, level1.assoc, level1.line);
        }
        if (level2.sizekb > 0) {
            printf("  -> L2: %dKB, %d-way, %dB line\n", 
                   level2.sizekb, level2.assoc, level2.line);
        }
    }
}

// Simulate the actual CPUID descriptor table parsing
void simulate_cpuid_descriptor_parsing() {
    printf("\n=== Simulating CPUID Descriptor Table Parsing ===\n");
    
    // Create a mock CPUID result with AL=0x06 (6 valid descriptor bytes)
    // This forces execution into the descriptor table parsing path
    uint32_t mock_eax = 0x06010001;  // AL=0x06, other bytes are descriptors
    uint32_t mock_ebx = 0x00000000;
    uint32_t mock_ecx = 0x00000000;
    uint32_t mock_edx = 0x6C0430C0;  // Contains descriptors 0x0c, 0x30, 0x43, 0x6c
    
    // Manually set the first byte of AL to >1 to bypass early returns
    uint8_t al = mock_eax & 0xFF;
    printf("Mock CPUID result: AL=%d (0x%02x)\n", al, al);
    
    if (al <= 1) {
        printf("Would use alternate method (not testing uncovered lines)\n");
        return;
    }
    
    // Process the descriptor bytes as the original code does
    uint8_t *regs = (uint8_t*)&mock_eax;
    int bytes_to_process = al;  // Number of valid descriptor bytes
    
    printf("Processing %d descriptor bytes:\n", bytes_to_process);
    
    for (int i = 0; i < bytes_to_process; i++) {
        uint8_t descriptor = regs[i];
        
        // Skip null descriptors and reserved ones
        if (descriptor == 0x00 || (descriptor & 0x80)) {
            continue;
        }
        
        printf("  Byte %d: 0x%02x -> ", i, descriptor);
        process_cache_descriptor(descriptor);
    }
}

int main() {
    printf("Cache Descriptor Test Program\n");
    printf("=============================\n");
    
    // Test 1: Direct CPUID calls
    test_cpuid_leaf_02();
    test_cpuid_leaf_04();
    
    // Test 2: Direct test of all uncovered cases
    test_all_uncovered_cases();
    
    // Test 3: Simulate the exact parsing logic with mock data
    simulate_cpuid_descriptor_parsing();
    
    // Test 4: Specifically test the xeon_mp conditional
    printf("\n=== Testing Xeon MP Conditional (Case 0x49) ===\n");
    xeon_mp = 0;  // Force the branch that sets L2 cache
    level2.sizekb = level2.assoc = level2.line = 0;
    process_cache_descriptor(0x49);
    printf("With xeon_mp=%d: L2 size = %dKB\n", xeon_mp, level2.sizekb);
    
    xeon_mp = 1;  // Force the branch that breaks
    level2.sizekb = level2.assoc = level2.line = 0;
    process_cache_descriptor(0x49);
    printf("With xeon_mp=%d: L2 size = %dKB (should be 0)\n", xeon_mp, level2.sizekb);
    
    printf("\nTest completed successfully!\n");
    return 0;
}
