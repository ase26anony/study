#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char char_src[10] = "abcdefghi";
static char char_dst[10];
static int int_src[10] = {0,1,2,3,4,5,6,7,8,9};
static int int_dst[10];
static long long ll_src[10] = {0,1,2,3,4,5,6,7,8,9};
static long long ll_dst[10];

// Small struct for testing
struct SmallStruct {
    char a;
    int b;
    char c;
};

static struct SmallStruct struct_src[5] = {
    {'a', 1, 'x'},
    {'b', 2, 'y'},
    {'c', 3, 'z'},
    {'d', 4, 'w'},
    {'e', 5, 'v'}
};
static struct SmallStruct struct_dst[5];

// Test 1: Small constant memcpy (count <= 2 or small total size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy 1 element - should trigger count <= 2 path
    memcpy(&char_dst[2], &char_src[2], 1);
    
    // Copy 2 elements - should trigger count <= 2 path
    memcpy(&char_dst[3], &char_src[3], 2);
    
    // Copy 3 chars = 3 bytes - small total size
    memcpy(&char_dst[0], &char_src[0], 3);
    
    // Copy 2 ints - count <= 2
    memcpy(&int_dst[1], &int_src[1], 2 * sizeof(int));
    
    // Copy 3 ints - larger total size (12 bytes on 32-bit, 12 bytes on 64-bit)
    memcpy(&int_dst[2], &int_src[2], 3 * sizeof(int));
}

// Test 2: Constant bounds but larger copy (should use library call)
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 10 ints = 40/80 bytes - larger than inline threshold
    memcpy(int_dst, int_src, 10 * sizeof(int));
    
    // Copy 5 long longs = 40/80 bytes
    memcpy(ll_dst, ll_src, 5 * sizeof(long long));
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // These should create register targets
    int temp1 = int_src[3];        // Single element to register
    char temp2 = char_src[4];      // Single char to register
    long long temp3 = ll_src[2];   // Single long long to register
    
    // Use the values to prevent optimization
    char_dst[0] = (char)temp1;
    char_dst[1] = temp2;
    int_dst[0] = (int)temp3;
}

// Test 4: Struct copies with constant sizes
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (small aggregate)
    struct_dst[0] = struct_src[0];
    
    // Copy 2 structs
    memcpy(&struct_dst[1], &struct_src[1], 2 * sizeof(struct SmallStruct));
    
    // Copy 3 structs - test size threshold
    memcpy(&struct_dst[2], &struct_src[2], 3 * sizeof(struct SmallStruct));
}

// Test 5: Array slices with constant loop bounds
NOINLINE static void test_const_loop_bounds(void) {
    // Loop with compile-time constant bounds
    for (int i = 2; i <= 4; ++i) {
        int_dst[i] = int_src[i];  // Copy 3 elements
    }
    
    // Another constant loop
    for (int i = 0; i < 2; ++i) {
        char_dst[i] = char_src[i];  // Copy 2 elements
    }
    
    // Loop with constant bounds but larger stride
    for (int i = 1; i < 5; i += 2) {
        ll_dst[i] = ll_src[i];  // Copy 2 elements (indices 1 and 3)
    }
}

// Test 6: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - should use library call path
    for (int i = start; i < end; ++i) {
        int_dst[i] = int_src[i];
    }
    
    // Variable-sized memcpy
    int size = end - start;
    if (size > 0) {
        memcpy(&char_dst[start], &char_src[start], size);
    }
}

// Test 7: Mixed types and sizes
NOINLINE static void test_mixed_types(void) {
    // Copy exactly 1 element of various types
    int_dst[9] = int_src[9];
    ll_dst[9] = ll_src[9];
    char_dst[9] = char_src[9];
    
    // Copy exactly 2 elements
    int_dst[7] = int_src[7];
    int_dst[8] = int_src[8];
    
    // Copy 3 chars (3 bytes total) - small size
    char temp[3];
    memcpy(temp, &char_src[5], 3);
    memcpy(&char_dst[5], temp, 3);
    
    // Copy 3 long longs (24 bytes on 64-bit) - test size threshold
    memcpy(&ll_dst[3], &ll_src[3], 3 * sizeof(long long));
}

// Compute checksum to prevent dead code elimination
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += char_dst[i];
        sum += int_dst[i];
        sum += (int)(ll_dst[i] & 0xFFFFFFFF);
    }
    for (int i = 0; i < 5; i++) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
    }
    return sum;
}

int main(void) {
    // Clear destination arrays
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    // Run all tests
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_const_loop_bounds();
    test_variable_bounds(0, 3);  // Variable bounds
    test_mixed_types();
    
    // Use results to prevent optimization
    return compute_checksum() == 0 ? 1 : 0;
}
