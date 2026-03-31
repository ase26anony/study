#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Small struct to test aggregate types
struct SmallStruct {
    char a;
    int b;
    short c;
};

// Union to test different type sizes
union MixedUnion {
    char bytes[8];
    int ints[2];
    long long ll;
};

// Global arrays to avoid aliasing issues
static int src_int[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int dst_int[10] = {0};
static char src_char[20] = "0123456789abcdef";
static char dst_char[20] = {0};
static struct SmallStruct src_struct[5] = {{'a', 1, 2}, {'b', 3, 4}, {'c', 5, 6}};
static struct SmallStruct dst_struct[5] = {{0}};
static union MixedUnion src_union = {.ll = 0x123456789ABCDEF0LL};
static union MixedUnion dst_union = {0};

// 1. Constant bounds, small count (<=2), memory target
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 2 elements - should trigger count <= 2 branch
    for (int i = 2; i <= 3; ++i) {  // constant bounds: 2 to 3 inclusive = 2 elements
        dst_int[i] = src_int[i];
    }
    
    // Copy 1 element with memcpy - constant size
    memcpy(&dst_char[0], &src_char[0], 1 * sizeof(char));
    
    // Copy 2 elements with memcpy - constant size
    memcpy(&dst_char[2], &src_char[2], 2 * sizeof(char));
}

// 2. Constant bounds, small total byte size
NOINLINE static void test_const_small_byte_size(void) {
    // Copy 3 chars = 3 bytes total - small byte size
    for (int i = 1; i <= 3; ++i) {  // constant bounds: 1 to 3 inclusive = 3 elements
        dst_char[i] = src_char[i];
    }
    
    // Copy 3 ints = 12 bytes (assuming 32-bit) - might be above threshold
    for (int i = 0; i <= 2; ++i) {  // constant bounds: 0 to 2 inclusive = 3 elements
        dst_int[i + 5] = src_int[i + 5];
    }
}

// 3. Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // Copy single element to register variable
    int temp1 = src_int[3];  // Should trigger !MEM_P(target) path
    
    // Copy single element from struct to register
    char temp2 = src_struct[1].a;
    int temp3 = src_struct[1].b;
    
    // Use the values to prevent optimization
    dst_int[0] = temp1;
    dst_struct[0].a = temp2;
    dst_struct[0].b = temp3;
}

// 4. Struct and union copies
NOINLINE static void test_struct_copy(void) {
    // Copy entire small struct - constant size
    dst_struct[1] = src_struct[1];
    
    // Copy struct array slice - constant bounds
    for (int i = 0; i <= 1; ++i) {  // 2 elements
        dst_struct[i + 2] = src_struct[i + 2];
    }
    
    // Copy union - constant size
    dst_union = src_union;
    
    // Copy part of union array
    memcpy(dst_union.bytes, src_union.bytes, 4);  // 4 bytes
}

// 5. Larger constant copy that might exceed inline threshold
NOINLINE static void test_const_larger_copy(void) {
    // Copy 8 ints = 32 bytes (assuming 32-bit) - might use library call
    for (int i = 0; i <= 7; ++i) {  // constant bounds: 0 to 7 inclusive = 8 elements
        dst_int[i] = src_int[i];
    }
    
    // Copy 16 chars = 16 bytes
    memcpy(dst_char, src_char, 16);
}

// 6. Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - should go through library call path
    for (int i = start; i <= end; ++i) {
        dst_int[i] = src_int[i];
    }
    
    // Variable size memcpy
    int size = end - start + 1;
    if (size > 0 && size < 10) {
        memcpy(&dst_char[start], &src_char[start], size * sizeof(char));
    }
}

// 7. Mixed element sizes
NOINLINE static void test_mixed_sizes(void) {
    // Copy 2 long longs = 16 bytes (assuming 64-bit)
    long long src_ll[3] = {1LL, 2LL, 3LL};
    long long dst_ll[3] = {0};
    
    for (int i = 0; i <= 1; ++i) {  // 2 elements
        dst_ll[i] = src_ll[i];
    }
    
    // Copy 1 long long to register
    long long temp = src_ll[2];
    dst_ll[2] = temp;
}

// Compute checksum to prevent dead code elimination
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        sum += dst_int[i];
    }
    
    for (int i = 0; i < 20; i++) {
        sum += dst_char[i];
    }
    
    for (int i = 0; i < 5; i++) {
        sum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    
    for (int i = 0; i < 8; i++) {
        sum += dst_union.bytes[i];
    }
    
    return sum;
}

int main(void) {
    // Clear destination arrays
    memset(dst_int, 0, sizeof(dst_int));
    memset(dst_char, 0, sizeof(dst_char));
    memset(dst_struct, 0, sizeof(dst_struct));
    memset(&dst_union, 0, sizeof(dst_union));
    
    // Execute all test cases
    test_const_small_memcpy();        // Lines 7691-7700: const_bounds_p, count <= 2
    test_const_small_byte_size();     // Lines 7691-7700: const_bounds_p, small byte size
    test_register_target();           // Lines 7691-7700: !MEM_P(target) path
    test_struct_copy();               // Lines 7691-7700: struct types
    test_const_larger_copy();         // Might exceed threshold
    test_variable_bounds(1, 3);       // Variable bounds - alternative path
    test_mixed_sizes();               // Different element sizes
    
    // Return checksum to prevent optimization
    return compute_checksum() > 0 ? 0 : 1;
}
