#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char char_src[16] = "0123456789ABCDEF";
static char char_dst[16];
static int int_src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
static int int_dst[8];
static long long ll_src[4] = {100, 200, 300, 400};
static long long ll_dst[4];

// Small struct for testing
struct SmallStruct {
    char a;
    int b;
    char c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 'x'},
    {'b', 2, 'y'},
    {'c', 3, 'z'},
    {'d', 4, 'w'}
};
static struct SmallStruct struct_dst[4];

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy 1 element - should trigger count <= 2 path
    memcpy(&char_dst[0], &char_src[0], 1);
    
    // Copy 2 elements - should trigger count <= 2 path
    memcpy(&int_dst[0], &int_src[0], 2 * sizeof(int));
    
    // Copy 3 chars = 3 bytes - small total size
    memcpy(&char_dst[3], &char_src[3], 3);
    
    // Copy 3 ints - larger total size (12 bytes on 32-bit, 24 on 64-bit)
    memcpy(&int_dst[2], &int_src[2], 3 * sizeof(int));
}

// Test 2: Constant large memcpy (exceeds inline threshold)
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 8 ints = 32/64 bytes - likely exceeds threshold
    memcpy(&int_dst[0], &int_src[0], 8 * sizeof(int));
    
    // Copy 4 long longs = 32/64 bytes
    memcpy(&ll_dst[0], &ll_src[0], 4 * sizeof(long long));
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // These should create register targets
    int temp1 = int_src[3];           // Single element to register
    long long temp2 = ll_src[2];      // Single element to register
    char temp3 = char_src[5];         // Single element to register
    
    // Use the values to prevent optimization
    char_dst[0] = (char)temp1;
    int_dst[0] = (int)temp2;
    ll_dst[0] = (long long)temp3;
}

// Test 4: Struct copies
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (small aggregate)
    struct_dst[0] = struct_src[0];
    
    // Copy 2 structs - count <= 2
    memcpy(&struct_dst[1], &struct_src[1], 2 * sizeof(struct SmallStruct));
    
    // Copy 3 structs - test size calculation
    memcpy(&struct_dst[0], &struct_src[0], 3 * sizeof(struct SmallStruct));
}

// Test 5: Array slices with constant bounds (explicit loops)
NOINLINE static void test_const_bounds_loops(void) {
    // Loop with constant bounds - should be unrolled
    for (int i = 2; i <= 4; ++i) {
        int_dst[i] = int_src[i];
    }
    
    // Another constant-bounded loop
    for (int i = 0; i < 2; ++i) {
        ll_dst[i] = ll_src[i];
    }
    
    // Char array with constant bounds
    for (int i = 1; i <= 3; ++i) {
        char_dst[i] = char_src[i];
    }
}

// Test 6: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - should go through library call path
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 8) {
            int_dst[i] = int_src[i];
        }
    }
    
    // Variable memcpy size
    size_t size = end - start;
    if (size > 0 && size <= 8) {
        memcpy(&char_dst[start], &char_src[start], size);
    }
}

// Test 7: Mixed operations to hit various conditions
NOINLINE static void test_mixed_operations(void) {
    // Direct assignment (count = 1)
    int_dst[7] = int_src[7];
    
    // Two assignments (count = 2)
    ll_dst[3] = ll_src[3];
    ll_dst[2] = ll_src[2];
    
    // Three chars via memcpy (small total size)
    memcpy(&char_dst[10], &char_src[10], 3);
    
    // Single struct member copy (register target)
    int temp = struct_src[2].b;
    struct_dst[2].b = temp;
}

// Compute checksum to prevent dead code elimination
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        sum += char_dst[i];
    }
    
    for (int i = 0; i < 8; i++) {
        sum += int_dst[i];
    }
    
    for (int i = 0; i < 4; i++) {
        sum += (int)ll_dst[i];
    }
    
    for (int i = 0; i < 4; i++) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
    }
    
    return sum;
}

int main(void) {
    // Run all test functions
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_const_bounds_loops();
    test_variable_bounds(1, 4);  // Variable bounds
    test_mixed_operations();
    
    // Return checksum to prevent optimization
    return compute_checksum() > 0 ? 0 : 1;
}
