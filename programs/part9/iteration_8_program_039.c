#include <stddef.h>
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
    short c;
};

static struct SmallStruct src_struct[5] = {{0}};
static struct SmallStruct dst_struct[5] = {{0}};

// Test 1: Small constant memcpy (count <= 2 or small total size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy 1 element - should trigger count <= 2 path
    memcpy(&dst_char[0], &src_char[0], 1 * sizeof(char));
    
    // Copy 2 elements - should trigger count <= 2 path
    memcpy(&dst_char[2], &src_char[2], 2 * sizeof(char));
    
    // Copy 3 chars = 3 bytes total (small size)
    memcpy(&dst_char[5], &src_char[5], 3 * sizeof(char));
    
    // Copy 2 ints - count <= 2
    memcpy(&dst_int[0], &src_int[0], 2 * sizeof(int));
    
    // Copy 3 ints - larger total size (12 bytes on 32-bit, 24 on 64-bit)
    memcpy(&dst_int[3], &src_int[3], 3 * sizeof(int));
}

// Test 2: Constant bounds with loop (non-MEM_P target)
NOINLINE static void test_register_target(void) {
    // Copy single element to register (!MEM_P target)
    int reg1 = src_int[3];  // Should trigger !MEM_P(target) path
    
    // Copy two elements to registers
    int reg2 = src_int[4];
    int reg3 = src_int[5];
    
    // Store back to memory
    dst_int[3] = reg1;
    dst_int[4] = reg2;
    dst_int[5] = reg3;
    
    // Constant loop bounds with register target
    for (int i = 2; i <= 4; ++i) {  // i=2,3,4 (count=3)
        int temp = src_int[i];      // Register target
        dst_int[i] = temp;          // Store back
    }
}

// Test 3: Struct copies (aggregate types)
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (small constant size)
    dst_struct[0] = src_struct[0];
    
    // Copy two structs
    dst_struct[1] = src_struct[1];
    dst_struct[2] = src_struct[2];
    
    // Copy three structs - larger total size
    // Using memcpy with constant bounds
    memcpy(&dst_struct[2], &src_struct[2], 3 * sizeof(struct SmallStruct));
}

// Test 4: Constant bounds with array slice copy
NOINLINE static void test_const_array_slice(void) {
    // Copy slice with constant bounds: indices 1 through 3 (count=3)
    for (int i = 1; i <= 3; ++i) {
        dst_int[i] = src_int[i];
    }
    
    // Different element type: long long
    // Copy 2 elements (count <= 2)
    for (int i = 0; i <= 1; ++i) {
        dst_ll[i] = src_ll[i];
    }
    
    // Copy 3 long longs - large total size (24 bytes on 64-bit)
    for (int i = 2; i <= 4; ++i) {
        dst_ll[i] = src_ll[i];
    }
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - const_bounds_p should be false
    for (int i = start; i <= end; ++i) {
        dst_char[i] = src_char[i];
    }
    
    // Another variable bounds example
    int count = end - start + 1;
    if (count > 0) {
        memcpy(&dst_int[start], &src_int[start], count * sizeof(int));
    }
}

// Test 6: Mixed scenarios
NOINLINE static void test_mixed_scenarios(void) {
    // Direct assignment of 2 elements (count <= 2)
    dst_int[0] = src_int[0];
    dst_int[1] = src_int[1];
    
    // 3-element copy with char type (small total size: 3 bytes)
    char temp[3];
    temp[0] = src_char[0];
    temp[1] = src_char[1];
    temp[2] = src_char[2];
    memcpy(&dst_char[0], temp, 3);
    
    // Union test
    union TestUnion {
        int i;
        float f;
        char c[4];
    } u1, u2;
    
    u1.i = 42;
    u2 = u1;  // Copy entire union
}

// Main function that calls all tests
int main(void) {
    int checksum = 0;
    
    // Initialize source arrays
    for (int i = 0; i < 10; i++) {
        src_char[i] = i;
        src_int[i] = i;
        src_ll[i] = i;
    }
    
    for (int i = 0; i < 5; i++) {
        src_struct[i].a = i;
        src_struct[i].b = i * 10;
        src_struct[i].c = i * 100;
    }
    
    // Run all test functions
    test_const_small_memcpy();
    test_register_target();
    test_struct_copy();
    test_const_array_slice();
    test_variable_bounds(1, 3);  // Variable bounds
    test_mixed_scenarios();
    
    // Calculate checksum to prevent dead code elimination
    for (int i = 0; i < 10; i++) {
        checksum += dst_char[i];
        checksum += dst_int[i];
        checksum += (int)dst_ll[i];
    }
    
    for (int i = 0; i < 5; i++) {
        checksum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    
    return checksum > 0 ? 0 : 1;
}
