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

// Small struct for testing
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct src_struct[5] = {{0}};
static struct SmallStruct dst_struct[5] = {{0}};

// Test 1: Small constant memcpy (count <= 2 or small total size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 2 chars (2 bytes total) - should trigger count <= 2
    memcpy(&dst_char[0], &src_char[0], 2 * sizeof(char));
    
    // Copy 3 chars (3 bytes total) - small total size
    memcpy(&dst_char[3], &src_char[3], 3 * sizeof(char));
    
    // Copy exactly 2 ints - count <= 2
    memcpy(&dst_int[0], &src_int[0], 2 * sizeof(int));
    
    // Copy 3 ints - larger total size (12 bytes on 32-bit, 24 on 64-bit)
    memcpy(&dst_int[3], &src_int[3], 3 * sizeof(int));
}

// Test 2: Constant bounds with array indices (non-memcpy form)
NOINLINE static void test_const_array_copy(void) {
    // Copy with constant loop bounds - count = 3
    for (int i = 2; i <= 4; ++i) {
        dst_int[i] = src_int[i];
    }
    
    // Copy exactly 2 elements with constant indices
    dst_int[0] = src_int[0];
    dst_int[1] = src_int[1];
    
    // Copy 3 long long elements - large total size
    for (int i = 1; i <= 3; ++i) {
        dst_ll[i] = src_ll[i];
    }
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // Copy single element to register (scalar variable)
    int temp1 = src_int[3];  // Should be !MEM_P(target)
    
    // Copy multiple elements to different registers
    char temp2 = src_char[2];
    long long temp3 = src_ll[4];
    
    // Use the values to prevent optimization
    dst_int[0] = temp1;
    dst_char[0] = temp2;
    dst_ll[0] = temp3;
}

// Test 4: Struct copies
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (small constant size)
    dst_struct[0] = src_struct[0];
    
    // Copy exactly 2 structs
    dst_struct[1] = src_struct[1];
    dst_struct[2] = src_struct[2];
    
    // Copy 3 structs with memcpy
    memcpy(&dst_struct[2], &src_struct[2], 3 * sizeof(struct SmallStruct));
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Loop with variable bounds - const_bounds_p should be false
    for (int i = start; i < end && i < 10; ++i) {
        dst_char[i] = src_char[i];
    }
}

// Test 6: Mixed operations to hit various conditions
NOINLINE static void test_mixed_operations(void) {
    // 1. Single element copy to memory (MEM_P target, count=1)
    dst_int[5] = src_int[5];
    
    // 2. Two element copy (count=2)
    dst_int[6] = src_int[6];
    dst_int[7] = src_int[7];
    
    // 3. Three element copy of small type (char)
    for (int i = 7; i <= 9; ++i) {
        dst_char[i] = src_char[i];
    }
    
    // 4. memcpy with size 4 (edge case)
    memcpy(&dst_int[8], &src_int[8], 4 * sizeof(int));
}

// Test 7: Union copies
NOINLINE static void test_union_copy(void) {
    union TestUnion {
        int i;
        float f;
        char c[4];
    };
    
    static union TestUnion u1 = {.i = 42};
    static union TestUnion u2 = {.i = 0};
    
    // Copy entire union
    u2 = u1;
    
    // Copy via memcpy
    memcpy(&u2, &u1, sizeof(union TestUnion));
}

// Compute checksum to prevent dead code elimination
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst_char[i];
        sum += dst_int[i];
        sum += (int)dst_ll[i];
    }
    for (int i = 0; i < 5; i++) {
        sum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    return sum;
}

int main(void) {
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
    
    // Run all tests
    test_const_small_memcpy();
    test_const_array_copy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(1, 4);  // Variable bounds
    test_mixed_operations();
    test_union_copy();
    
    // Return checksum to prevent optimization
    return compute_checksum() > 0 ? 0 : 1;
}
