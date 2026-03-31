#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char src_char[10] = {0,1,2,3,4,5,6,7,8,9};
static char dst_char[10] = {0};
static int src_int[10] = {0,10,20,30,40,50,60,70,80,90};
static int dst_int[10] = {0};
static long long src_ll[10] = {0,100,200,300,400,500,600,700,800,900};
static long long dst_ll[10] = {0};

// Small struct for aggregate type testing
struct SmallStruct {
    char a;
    int b;
    char c;
};

static struct SmallStruct src_struct[5] = {
    {'a', 1, 'x'},
    {'b', 2, 'y'},
    {'c', 3, 'z'},
    {'d', 4, 'w'},
    {'e', 5, 'v'}
};
static struct SmallStruct dst_struct[5] = {0};

// Union for testing
union TestUnion {
    int i;
    float f;
    char arr[4];
};

static union TestUnion src_union[3] = {{.i=42}, {.f=3.14f}, {.arr="abc"}};
static union TestUnion dst_union[3] = {0};

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Copy exactly 1 element - should trigger count <= 2 path
    dst_char[0] = src_char[0];
    checksum += dst_char[0];
    
    // Copy exactly 2 elements - should trigger count <= 2 path
    dst_int[0] = src_int[0];
    dst_int[1] = src_int[1];
    checksum += dst_int[0] + dst_int[1];
    
    // Copy 3 chars (3 bytes total) - small total size path
    for (int i = 2; i <= 4; ++i) {  // Constant bounds: i=2, i<=4
        dst_char[i] = src_char[i];
        checksum += dst_char[i];
    }
    
    // memcpy with constant size 2 (bytes)
    memcpy(&dst_char[5], &src_char[5], 2);
    checksum += dst_char[5] + dst_char[6];
    
    return checksum;
}

// Test 2: Constant large memcpy (total size > threshold)
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 3 long longs (24 bytes on 64-bit) - potentially large
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: i=0, i<=2
        dst_ll[i] = src_ll[i];
        checksum += (int)dst_ll[i];
    }
    
    // memcpy with constant size 16 bytes
    memcpy(&dst_int[3], &src_int[3], 4 * sizeof(int));  // 16 bytes
    for (int i = 3; i <= 6; ++i) {
        checksum += dst_int[i];
    }
    
    return checksum;
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    // Copy single element to register (scalar variable)
    int reg1 = src_int[2];  // Should trigger !MEM_P(target) path
    checksum += reg1;
    
    char reg2 = src_char[3];
    checksum += reg2;
    
    long long reg3 = src_ll[4];
    checksum += (int)reg3;
    
    return checksum;
}

// Test 4: Struct and union copies
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy single struct (aggregate type)
    dst_struct[0] = src_struct[0];
    checksum += src_struct[0].b;
    
    // Copy 2 structs - count <= 2
    dst_struct[1] = src_struct[1];
    dst_struct[2] = src_struct[2];
    checksum += src_struct[1].b + src_struct[2].b;
    
    // Copy union
    dst_union[0] = src_union[0];
    checksum += src_union[0].i;
    
    // Copy 2 union elements with constant bounds
    for (int i = 1; i <= 2; ++i) {
        dst_union[i] = src_union[i];
        checksum += src_union[i].arr[0];
    }
    
    return checksum;
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Variable bounds - const_bounds_p should be false
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 10) {
            dst_char[i] = src_char[i];
            checksum += dst_char[i];
        }
    }
    
    // memcpy with variable size
    int size = end - start;
    if (size > 0 && size <= 10) {
        memcpy(&dst_int[start], &src_int[start], size * sizeof(int));
        for (int i = start; i < end; ++i) {
            checksum += dst_int[i];
        }
    }
    
    return checksum;
}

// Test 6: Mixed scenarios
NOINLINE static int test_mixed_scenarios(void) {
    int checksum = 0;
    
    // Copy 1 element from middle of array
    dst_int[5] = src_int[5];
    checksum += dst_int[5];
    
    // Copy 2 adjacent elements
    dst_char[7] = src_char[7];
    dst_char[8] = src_char[8];
    checksum += dst_char[7] + dst_char[8];
    
    // Copy slice with constant bounds but different types
    short src_short[] = {1,2,3,4,5};
    short dst_short[5];
    for (int i = 1; i <= 3; ++i) {  // Copy 3 shorts (6 bytes)
        dst_short[i] = src_short[i];
        checksum += dst_short[i];
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    // Run all tests
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_variable_bounds(2, 5);  // Variable bounds
    total_checksum += test_mixed_scenarios();
    
    // Use result to prevent dead code elimination
    return total_checksum == 0 ? 1 : 0;
}
