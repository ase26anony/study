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

// Test 1: Constant bounds, small count (<=2), memory target
NOINLINE static int test_const_small_memcpy(void) {
    int sum = 0;
    
    // Copy exactly 1 element - should trigger count <= 2 path
    dst_int[0] = src_int[0];
    sum += dst_int[0];
    
    // Copy exactly 2 elements - should trigger count <= 2 path
    dst_int[1] = src_int[1];
    dst_int[2] = src_int[2];
    sum += dst_int[1] + dst_int[2];
    
    // Copy 3 chars (3 bytes total) - small total size
    for (int i = 3; i <= 5; ++i) {  // Constant bounds: 3 to 5 inclusive
        dst_char[i] = src_char[i];
        sum += dst_char[i];
    }
    
    // memcpy with constant size 2 (should inline)
    memcpy(&dst_char[6], &src_char[6], 2);
    sum += dst_char[6] + dst_char[7];
    
    return sum;
}

// Test 2: Constant bounds, larger count but small element size
NOINLINE static int test_const_small_elements(void) {
    int sum = 0;
    
    // Copy 5 chars (5 bytes total) - still small
    for (int i = 0; i <= 4; ++i) {  // Constant bounds: 0 to 4
        dst_char[i] = src_char[i] + 1;
        sum += dst_char[i];
    }
    
    return sum;
}

// Test 3: Constant bounds, larger total size (3 long longs = 24 bytes on 64-bit)
NOINLINE static int test_const_large_elements(void) {
    int sum = 0;
    
    // Copy 3 long longs - larger total size
    for (int i = 2; i <= 4; ++i) {  // Constant bounds: 2 to 4
        dst_ll[i] = src_ll[i];
        sum += (int)dst_ll[i];
    }
    
    return sum;
}

// Test 4: Non-MEM_P target (register)
NOINLINE static int test_register_target(void) {
    int sum = 0;
    
    // Copy single element to register - !MEM_P(target) should be true
    int reg1 = src_int[3];
    sum += reg1;
    
    // Copy two elements to registers
    int reg2 = src_int[4];
    int reg3 = src_int[5];
    sum += reg2 + reg3;
    
    // Store back to memory
    dst_int[3] = reg1;
    dst_int[4] = reg2;
    dst_int[5] = reg3;
    
    return sum;
}

// Test 5: Struct copies (aggregate types)
NOINLINE static int test_struct_copy(void) {
    int sum = 0;
    
    // Initialize source struct
    for (int i = 0; i < 5; i++) {
        src_struct[i].a = i;
        src_struct[i].b = i * 10;
        src_struct[i].c = i + 'A';
    }
    
    // Copy single struct - small aggregate
    dst_struct[0] = src_struct[0];
    sum += dst_struct[0].b;
    
    // Copy 2 structs - count <= 2
    dst_struct[1] = src_struct[1];
    dst_struct[2] = src_struct[2];
    sum += dst_struct[1].b + dst_struct[2].b;
    
    // Copy 3 structs with constant bounds loop
    for (int i = 3; i <= 5; ++i) {  // Note: i <= 5 but array size is 5
        if (i < 5) {
            dst_struct[i] = src_struct[i];
            sum += dst_struct[i].b;
        }
    }
    
    return sum;
}

// Test 6: Variable bounds (should NOT trigger const_bounds_p path)
NOINLINE static int test_variable_bounds(int start, int end) {
    int sum = 0;
    
    // Variable bounds - const_bounds_p should be false
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 10) {
            dst_int[i] = src_int[i] * 2;
            sum += dst_int[i];
        }
    }
    
    // memcpy with variable size
    if (end - start + 1 > 0 && end < 10) {
        memcpy(&dst_char[start], &src_char[start], end - start + 1);
        for (int i = start; i <= end; ++i) {
            sum += dst_char[i];
        }
    }
    
    return sum;
}

// Test 7: Mixed operations to cover various paths
NOINLINE static int test_mixed_operations(void) {
    int sum = 0;
    
    // Direct assignment of 2 elements
    dst_int[6] = src_int[6];
    dst_int[7] = src_int[7];
    sum += dst_int[6] + dst_int[7];
    
    // memcpy with size 4 (might inline depending on threshold)
    memcpy(&dst_int[8], &src_int[8], 4);  // 1 int = 4 bytes on most platforms
    sum += dst_int[8];
    
    // Array slice copy with constant bounds but different element types
    short src_short[5] = {10, 20, 30, 40, 50};
    short dst_short[5] = {0};
    
    for (int i = 1; i <= 3; ++i) {  // Constant bounds
        dst_short[i] = src_short[i];
        sum += dst_short[i];
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    // Initialize source arrays
    for (int i = 0; i < 10; i++) {
        src_int[i] = i;
        src_char[i] = (char)i;
        src_ll[i] = i;
    }
    
    // Run all tests
    checksum += test_const_small_memcpy();
    checksum += test_const_small_elements();
    checksum += test_const_large_elements();
    checksum += test_register_target();
    checksum += test_struct_copy();
    checksum += test_variable_bounds(1, 3);  // Variable bounds
    checksum += test_mixed_operations();
    
    // Additional test with different constant bounds
    for (int i = 0; i <= 1; ++i) {  // count = 2
        dst_char[i + 8] = src_char[i + 8];
        checksum += dst_char[i + 8];
    }
    
    return checksum & 0xFF;  // Return non-zero checksum
}
