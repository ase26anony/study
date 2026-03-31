#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char char_src[32] = "abcdefghijklmnopqrstuvwxyz012345";
static char char_dst[32];
static int int_src[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
static int int_dst[16];
static long long ll_src[8] = {100,200,300,400,500,600,700,800};
static long long ll_dst[8];

// Small struct for testing
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10}, {'b', 2, 20}, {'c', 3, 30}, {'d', 4, 40}
};
static struct SmallStruct struct_dst[4];

// Test 1: Small constant memcpy (count <= 2 or small total size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy 1 char (count=1, small size) - should trigger !MEM_P(target) path?
    char_dst[0] = char_src[0];
    
    // Copy 2 chars (count=2) - hits count <= 2 branch
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    
    // Copy 3 chars with constant bounds loop - total size = 3 bytes
    for (int i = 3; i <= 5; ++i) {
        char_dst[i] = char_src[i];
    }
    
    // Copy 2 ints (count=2) - hits count <= 2 branch
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    
    // Copy 3 ints with memcpy - constant size = 12 bytes (assuming 4-byte int)
    memcpy(&int_dst[2], &int_src[2], 3 * sizeof(int));
}

// Test 2: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // Copy single element to register variable
    int temp1 = int_src[3];  // Should trigger !MEM_P(target) path
    
    // Copy two elements to separate registers
    long long temp2 = ll_src[0];
    long long temp3 = ll_src[1];
    
    // Store back to verify no dead code elimination
    int_dst[3] = temp1;
    ll_dst[0] = temp2;
    ll_dst[1] = temp3;
}

// Test 3: Struct copies (aggregate types)
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (count=1)
    struct_dst[0] = struct_src[0];
    
    // Copy two structs (count=2)
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    // Copy 3 structs with constant bounds loop
    for (int i = 1; i <= 3; ++i) {
        struct_dst[i] = struct_src[i];
    }
}

// Test 4: Variable bounds (should NOT trigger const_bounds_p path)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Loop with variable bounds - should go to library call path
    for (int i = start; i < end && i < 16; ++i) {
        int_dst[i] = int_src[i];
    }
}

// Test 5: Mixed element sizes and counts
NOINLINE static void test_mixed_sizes(void) {
    // Copy 3 chars (3 bytes total) - small size
    for (int i = 10; i <= 12; ++i) {
        char_dst[i] = char_src[i];
    }
    
    // Copy 3 long longs (24 bytes on 64-bit) - tests size threshold
    for (int i = 2; i <= 4; ++i) {
        ll_dst[i] = ll_src[i];
    }
    
    // Direct assignment of 2 elements (count <= 2)
    ll_dst[5] = ll_src[5];
    ll_dst[6] = ll_src[6];
}

// Test 6: Union copy
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion union_src = {.i = 0x12345678};
static union TestUnion union_dst;

NOINLINE static void test_union_copy(void) {
    // Copy entire union (constant size, count=1)
    union_dst = union_src;
    
    // Copy union array element
    union TestUnion union_array[2] = {{.i = 1}, {.i = 2}};
    union TestUnion union_array_dst[2];
    
    for (int i = 0; i <= 1; ++i) {
        union_array_dst[i] = union_array[i];
    }
}

// Main function that calls all tests
int main(void) {
    // Initialize
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    // Run all tests
    test_const_small_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(5, 8);  // Variable bounds
    test_mixed_sizes();
    test_union_copy();
    
    // Additional test with exactly count=2
    int_dst[10] = int_src[10];
    int_dst[11] = int_src[11];
    
    // Additional test with count=1 but large element
    long long single_ll = ll_src[7];
    ll_dst[7] = single_ll;
    
    // Create checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < 32; i++) checksum += char_dst[i];
    for (int i = 0; i < 16; i++) checksum += int_dst[i];
    for (int i = 0; i < 8; i++) checksum += (int)ll_dst[i];
    
    return checksum == 0 ? 0 : 1;
}
