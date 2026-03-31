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

static struct SmallStruct src_struct[5] = {{0}};
static struct SmallStruct dst_struct[5] = {{0}};

// Union for testing
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion src_union[5] = {{0}};
static union TestUnion dst_union[5] = {{0}};

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Case 1: Copy exactly 1 element (count = 1)
    dst_char[0] = src_char[0];
    checksum += dst_char[0];
    
    // Case 2: Copy exactly 2 elements (count = 2)
    dst_char[1] = src_char[1];
    dst_char[2] = src_char[2];
    checksum += dst_char[1] + dst_char[2];
    
    // Case 3: Copy 3 chars (3 bytes total - small size)
    // Using explicit loop with constant bounds
    for (int i = 3; i <= 5; ++i) {
        dst_char[i] = src_char[i];
        checksum += dst_char[i];
    }
    
    // Case 4: Copy 2 ints (8 bytes on 64-bit, possibly inline)
    for (int i = 0; i <= 1; ++i) {
        dst_int[i] = src_int[i];
        checksum += dst_int[i];
    }
    
    // Case 5: memcpy with constant size 2
    memcpy(&dst_char[6], &src_char[6], 2);
    checksum += dst_char[6] + dst_char[7];
    
    return checksum;
}

// Test 2: Constant bounds but larger size (might not inline)
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 3 long longs (24 bytes on 64-bit - might exceed threshold)
    for (int i = 0; i <= 2; ++i) {
        dst_ll[i] = src_ll[i];
        checksum += (int)dst_ll[i];
    }
    
    // memcpy with constant size 24
    memcpy(&dst_ll[3], &src_ll[3], 3 * sizeof(long long));
    for (int i = 3; i < 6; ++i) {
        checksum += (int)dst_ll[i];
    }
    
    return checksum;
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    // Copy single element to register (scalar variable)
    int reg1 = src_int[0];
    checksum += reg1;
    
    char reg2 = src_char[1];
    checksum += reg2;
    
    long long reg3 = src_ll[2];
    checksum += (int)reg3;
    
    // Multiple register copies
    int reg4 = src_int[3];
    int reg5 = src_int[4];
    checksum += reg4 + reg5;
    
    return checksum;
}

// Test 4: Struct and union copies
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy entire struct (small constant size)
    dst_struct[0] = src_struct[0];
    checksum += dst_struct[0].a + dst_struct[0].b;
    
    // Copy 2 struct elements
    for (int i = 1; i <= 2; ++i) {
        dst_struct[i] = src_struct[i];
        checksum += dst_struct[i].a + dst_struct[i].b;
    }
    
    // Union copy
    dst_union[0] = src_union[0];
    checksum += dst_union[0].i;
    
    // Copy union array slice
    for (int i = 1; i <= 2; ++i) {
        dst_union[i] = src_union[i];
        checksum += dst_union[i].i;
    }
    
    return checksum;
}

// Test 5: Variable bounds (should not trigger const_bounds_p)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Loop with variable bounds
    for (int i = start; i < end; ++i) {
        dst_int[i] = src_int[i];
        checksum += dst_int[i];
    }
    
    // memcpy with variable size
    int size = end - start;
    if (size > 0) {
        memcpy(&dst_char[start], &src_char[start], size);
        for (int i = start; i < end; ++i) {
            checksum += dst_char[i];
        }
    }
    
    return checksum;
}

// Test 6: Mixed scenarios
NOINLINE static int test_mixed_scenarios(void) {
    int checksum = 0;
    
    // Mixed copy: 1 element to memory, 1 to register
    dst_int[5] = src_int[5];
    int temp = src_int[6];
    checksum += dst_int[5] + temp;
    
    // Small array initialization with constant bounds
    int small[3];
    for (int i = 0; i <= 2; ++i) {
        small[i] = src_int[i];
        checksum += small[i];
    }
    
    // Nested constant-bound copies
    char buf1[4], buf2[4];
    for (int i = 0; i <= 3; ++i) {
        buf1[i] = src_char[i];
        buf2[i] = buf1[i];  // Copy from another array
        checksum += buf2[i];
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    // Initialize source arrays
    for (int i = 0; i < 10; i++) {
        src_char[i] = i;
        src_int[i] = i;
        src_ll[i] = i;
        if (i < 5) {
            src_struct[i].a = i;
            src_struct[i].b = i * 10;
            src_struct[i].c = i;
            src_union[i].i = i * 100;
        }
    }
    
    // Run all tests
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_variable_bounds(2, 5);  // Variable bounds
    total_checksum += test_mixed_scenarios();
    
    // Return checksum to prevent dead code elimination
    return total_checksum > 0 ? 0 : 1;
}
