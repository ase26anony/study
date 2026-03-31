#include <stddef.h>
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
    char b;
    char c;
    int d;
};

static struct SmallStruct struct_src[5] = {
    {'a','b','c',1},
    {'d','e','f',2},
    {'g','h','i',3},
    {'j','k','l',4},
    {'m','n','o',5}
};
static struct SmallStruct struct_dst[5];

// Union for testing
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion union_src[5];
static union TestUnion union_dst[5];

// 1. Constant small memcpy - should trigger count <= 2 or small size branch
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 1 element (count = 1)
    char_dst[0] = char_src[0];
    
    // Copy exactly 2 elements (count = 2)
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    
    // Use memcpy for 3 chars (3 bytes total - small size)
    memcpy(&char_dst[3], &char_src[3], 3 * sizeof(char));
    
    // Copy 2 ints (count = 2)
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    
    // Use memcpy for 2 ints (count = 2)
    memcpy(&int_dst[2], &int_src[2], 2 * sizeof(int));
}

// 2. Constant large memcpy - should NOT trigger the fast path due to size
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 3 long longs (24 bytes on 64-bit - may exceed threshold)
    memcpy(&ll_dst[0], &ll_src[0], 3 * sizeof(long long));
    
    // Copy 5 ints (20 bytes - may be near threshold)
    memcpy(&int_dst[5], &int_src[5], 5 * sizeof(int));
}

// 3. Register target - should trigger !MEM_P(target) branch
NOINLINE static void test_register_target(void) {
    // Copy single element to register (scalar variable)
    int temp1 = int_src[3];
    int_dst[3] = temp1;
    
    // Copy single char to register
    char temp2 = char_src[4];
    char_dst[4] = temp2;
    
    // Copy single long long to register
    long long temp3 = ll_src[2];
    ll_dst[2] = temp3;
}

// 4. Struct copy - tests RECORD_TYPE handling
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (small constant size)
    struct_dst[0] = struct_src[0];
    
    // Copy 2 structs (count = 2)
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    // Use memcpy for 3 structs (may trigger size-based logic)
    memcpy(&struct_dst[3], &struct_src[3], 2 * sizeof(struct SmallStruct));
}

// 5. Union copy
NOINLINE static void test_union_copy(void) {
    // Initialize union data
    for (int i = 0; i < 5; i++) {
        union_src[i].i = i * 10;
    }
    
    // Copy single union
    union_dst[0] = union_src[0];
    
    // Copy 2 unions
    union_dst[1] = union_src[1];
    union_dst[2] = union_src[2];
}

// 6. Variable bounds - should NOT trigger const_bounds_p branch
NOINLINE static void test_variable_bounds(int start, int end) {
    // Loop with variable bounds - const_bounds_p should be false
    for (int i = start; i < end && i < 10; i++) {
        char_dst[i] = char_src[i];
    }
    
    // Another variable bounds copy
    int count = end - start;
    if (count > 0 && count <= 10) {
        memcpy(&int_dst[start], &int_src[start], count * sizeof(int));
    }
}

// 7. Additional tests for array slice with constant bounds
NOINLINE static void test_array_slice_const_bounds(void) {
    // Copy slice with constant bounds: indices 2 to 4 inclusive (count = 3)
    for (int i = 2; i <= 4; ++i) {
        int_dst[i] = int_src[i];
    }
    
    // Another constant bounds slice: indices 1 to 3 (count = 3)
    for (int i = 1; i <= 3; ++i) {
        char_dst[i] = char_src[i];
    }
    
    // Constant bounds with memcpy
    memcpy(&ll_dst[3], &ll_src[3], 2 * sizeof(long long));  // indices 3-4
}

// 8. Mixed types and sizes
NOINLINE static void test_mixed_sizes(void) {
    // Copy 1 element (count = 1)
    struct_dst[4] = struct_src[4];
    
    // Copy 2 chars (2 bytes)
    char_dst[8] = char_src[8];
    char_dst[9] = char_src[9];
    
    // Copy 3 ints (12 bytes - may be threshold dependent)
    int_dst[7] = int_src[7];
    int_dst[8] = int_src[8];
    int_dst[9] = int_src[9];
}

// Main function that calls all tests and returns checksum
int main(void) {
    int checksum = 0;
    
    // Initialize source arrays
    for (int i = 0; i < 10; i++) {
        char_src[i] = 'A' + i;
        int_src[i] = i * 2;
        ll_src[i] = i * 3LL;
    }
    
    // Run all test functions
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_union_copy();
    test_variable_bounds(0, 3);  // Variable bounds
    test_array_slice_const_bounds();
    test_mixed_sizes();
    
    // Calculate checksum to prevent dead code elimination
    for (int i = 0; i < 10; i++) {
        checksum += char_dst[i];
        checksum += int_dst[i];
        checksum += (int)(ll_dst[i] & 0xFFFFFFFF);
    }
    
    for (int i = 0; i < 5; i++) {
        checksum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c + struct_dst[i].d;
        checksum += union_dst[i].i;
    }
    
    return checksum & 0xFF;  // Return non-zero value
}
