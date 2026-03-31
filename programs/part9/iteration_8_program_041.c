#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char src_char[10] = {0,1,2,3,4,5,6,7,8,9};
static char dst_char[10] = {0};
static int src_int[10] = {0,1,2,3,4,5,6,7,8,9};
static int dst_int[10] = {0};
static long long src_ll[10] = {0,1,2,3,4,5,6,7,8,9};
static long long dst_ll[10] = {0};

// Small struct for aggregate type testing
struct SmallStruct {
    char a;
    int b;
    char c;
};

static struct SmallStruct src_struct[5] = {{1,2,3},{4,5,6},{7,8,9},{10,11,12},{13,14,15}};
static struct SmallStruct dst_struct[5] = {{0}};

// Union for testing
union TestUnion {
    int i;
    float f;
    char arr[4];
};

static union TestUnion src_union[3] = {{.i=42}, {.f=3.14f}, {.arr="xyz"}};
static union TestUnion dst_union[3] = {{0}};

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Copy exactly 1 element - should trigger count <= 2 path
    dst_char[0] = src_char[0];
    checksum += dst_char[0];
    
    // Copy exactly 2 elements - should trigger count <= 2 path
    dst_char[1] = src_char[1];
    dst_char[2] = src_char[2];
    checksum += dst_char[1] + dst_char[2];
    
    // Copy 3 chars (3 bytes total) - small total size path
    for (int i = 3; i <= 5; ++i) {  // Constant bounds: i=3 to i=5 inclusive
        dst_char[i] = src_char[i];
        checksum += dst_char[i];
    }
    
    // Copy 2 ints (8 bytes on 64-bit) - count <= 2 path
    for (int i = 0; i <= 1; ++i) {
        dst_int[i] = src_int[i];
        checksum += dst_int[i];
    }
    
    return checksum;
}

// Test 2: Constant large memcpy (total size > threshold)
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 3 long longs (24 bytes on 64-bit) - potentially large total size
    for (int i = 0; i <= 2; ++i) {  // Constant bounds
        dst_ll[i] = src_ll[i];
        checksum += (int)dst_ll[i];
    }
    
    // Use memcpy with constant size for array slice
    memcpy(&dst_char[5], &src_char[5], 4 * sizeof(char));  // 4 bytes
    
    // Check copied values
    for (int i = 5; i <= 8; ++i) {
        checksum += dst_char[i];
    }
    
    return checksum;
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    // Copy single element to register (scalar variable)
    int reg1 = src_int[3];  // Should trigger !MEM_P(target) path
    checksum += reg1;
    
    char reg2 = src_char[4];
    checksum += reg2;
    
    long long reg3 = src_ll[2];
    checksum += (int)reg3;
    
    // Store back to memory
    dst_int[3] = reg1;
    dst_char[4] = reg2;
    dst_ll[2] = reg3;
    
    return checksum;
}

// Test 4: Struct and union copies
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy single struct (small aggregate)
    dst_struct[0] = src_struct[0];  // Should be treated as constant-sized copy
    checksum += dst_struct[0].a + dst_struct[0].b + dst_struct[0].c;
    
    // Copy 2 structs
    for (int i = 1; i <= 2; ++i) {  // Constant bounds
        dst_struct[i] = src_struct[i];
        checksum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    
    // Copy union
    dst_union[0] = src_union[0];
    checksum += dst_union[0].i;
    
    // Copy union array slice
    for (int i = 1; i <= 2; ++i) {
        dst_union[i] = src_union[i];
        checksum += dst_union[i].arr[0];
    }
    
    return checksum;
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p path)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Variable bounds - compiler can't determine as constant
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 10) {
            dst_int[i] = src_int[i];
            checksum += dst_int[i];
        }
    }
    
    // Variable-sized memcpy
    int count = end - start + 1;
    if (count > 0 && count <= 10) {
        memcpy(&dst_char[start], &src_char[start], count * sizeof(char));
        for (int i = start; i <= end; ++i) {
            if (i >= 0 && i < 10) {
                checksum += dst_char[i];
            }
        }
    }
    
    return checksum;
}

// Test 6: Mixed patterns for additional coverage
NOINLINE static int test_mixed_patterns(void) {
    int checksum = 0;
    
    // Direct assignment of 2 elements
    dst_int[4] = src_int[4];
    dst_int[5] = src_int[5];
    checksum += dst_int[4] + dst_int[5];
    
    // Loop with constant bounds but different step
    for (int i = 6; i < 9; i += 1) {  // i=6,7,8
        dst_int[i] = src_int[i];
        checksum += dst_int[i];
    }
    
    // Nested constant bounds
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            dst_char[idx] = src_char[idx];
            checksum += dst_char[idx];
        }
    }
    
    return checksum;
}

int main(void) {
    int checksum = 0;
    
    // Run all tests
    checksum += test_const_small_memcpy();
    checksum += test_const_large_memcpy();
    checksum += test_register_target();
    checksum += test_struct_copy();
    checksum += test_variable_bounds(2, 4);  // Variable bounds
    checksum += test_mixed_patterns();
    
    // Final verification
    for (int i = 0; i < 10; ++i) {
        checksum += dst_char[i];
        checksum += dst_int[i];
        checksum += (int)dst_ll[i];
    }
    
    for (int i = 0; i < 5; ++i) {
        checksum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    
    return checksum == 0 ? 0 : 1;  // Return non-zero if any data was copied
}
