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

// Union for testing
union MixedUnion {
    int i;
    float f;
    char c[4];
};

static union MixedUnion union_src[4];
static union MixedUnion union_dst[4];

// Test 1: Small constant memcpy (should trigger count <= 2 or small size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 1 element (count <= 2 branch)
    memcpy(&char_dst[0], &char_src[0], 1);
    
    // Copy exactly 2 elements (count <= 2 branch)
    memcpy(&char_dst[1], &char_src[1], 2);
    
    // Copy 3 chars = 3 bytes (small size branch)
    memcpy(&char_dst[3], &char_src[3], 3);
    
    // Copy 2 ints = 8 bytes on 64-bit (count <= 2 branch)
    memcpy(&int_dst[0], &int_src[0], 2 * sizeof(int));
    
    // Copy 3 ints = 12 bytes (size-based decision)
    memcpy(&int_dst[2], &int_src[2], 3 * sizeof(int));
}

// Test 2: Constant bounds but larger size
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 4 long longs = 32 bytes (likely above threshold)
    memcpy(&ll_dst[0], &ll_src[0], 4 * sizeof(long long));
    
    // Copy 6 ints = 24 bytes
    memcpy(&int_dst[0], &int_src[0], 6 * sizeof(int));
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // These should create register targets
    int reg1 = int_src[0];          // Single element to register
    int reg2 = int_src[1];
    char reg3 = char_src[2];
    long long reg4 = ll_src[0];
    
    // Store back to memory to prevent elimination
    int_dst[0] = reg1;
    int_dst[1] = reg2;
    char_dst[2] = reg3;
    ll_dst[0] = reg4;
}

// Test 4: Struct and union copies
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (count = 1, size = sizeof(struct SmallStruct))
    struct_dst[0] = struct_src[0];
    
    // Copy 2 structs (count <= 2)
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    // Initialize union
    union_src[0].i = 0x12345678;
    union_src[1].f = 3.14f;
    
    // Copy unions
    union_dst[0] = union_src[0];
    union_dst[1] = union_src[1];
    
    // Copy via memcpy with struct type
    memcpy(&struct_dst[3], &struct_src[3], sizeof(struct SmallStruct));
}

// Test 5: Array slices with constant bounds (explicit loops)
NOINLINE static void test_const_bounded_loops(void) {
    // Loop with constant bounds: i = 2 to 4 inclusive (count = 3)
    for (int i = 2; i <= 4; ++i) {
        int_dst[i] = int_src[i];
    }
    
    // Loop with constant bounds: i = 0 to 1 (count = 2)
    for (int i = 0; i <= 1; ++i) {
        char_dst[i] = char_src[i];
    }
    
    // Loop with constant bounds: i = 1 to 3 (count = 3)
    for (int i = 1; i <= 3; ++i) {
        ll_dst[i] = ll_src[i];
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
    
    // Variable size memcpy
    size_t size = end - start;
    if (size > 0 && size <= 8) {
        memcpy(&char_dst[start], &char_src[start], size);
    }
}

// Test 7: Mixed operations to hit various conditions
NOINLINE static void test_mixed_operations(void) {
    // Direct assignment of 2 elements (count <= 2)
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    
    // 3-element copy with different types
    char temp[3];
    temp[0] = char_src[0];
    temp[1] = char_src[1];
    temp[2] = char_src[2];
    memcpy(&char_dst[5], temp, 3);
    
    // Struct array element copy
    struct SmallStruct local = struct_src[0];
    struct_dst[0] = local;
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
        sum += (int)(ll_dst[i] & 0xFFFFFFFF);
        sum += (int)((ll_dst[i] >> 32) & 0xFFFFFFFF);
    }
    
    for (int i = 0; i < 4; i++) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
        sum += union_dst[i].i;
    }
    
    return sum;
}

int main(void) {
    // Initialize source arrays
    for (int i = 0; i < 16; i++) {
        char_src[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 8; i++) {
        int_src[i] = i * 10;
    }
    
    for (int i = 0; i < 4; i++) {
        ll_src[i] = i * 100LL;
    }
    
    // Run all tests
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_const_bounded_loops();
    test_variable_bounds(1, 4);  // Variable bounds
    test_mixed_operations();
    
    // Compute and return checksum
    return compute_checksum() & 0xFF;
}
