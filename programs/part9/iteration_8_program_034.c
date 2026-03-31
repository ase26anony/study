#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char char_src[16] = "0123456789ABCDEF";
static char char_dst[16];
static int int_src[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static int int_dst[8];
static long long ll_src[4] = {100, 200, 300, 400};
static long long ll_dst[4];

// Small struct for testing
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10},
    {'b', 2, 20},
    {'c', 3, 30},
    {'d', 4, 40}
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

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Copy exactly 1 element - should trigger count <= 2 path
    char_dst[0] = char_src[0];
    checksum += char_dst[0];
    
    // Copy exactly 2 elements - should trigger count <= 2 path
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    checksum += int_dst[0] + int_dst[1];
    
    // Copy 3 chars (3 bytes total) - small total size path
    for (int i = 2; i <= 4; ++i) {  // Constant bounds: i=2 to i=4 inclusive
        char_dst[i] = char_src[i];
        checksum += char_dst[i];
    }
    
    // memcpy with constant size 2 (fits in count <= 2)
    memcpy(&char_dst[5], &char_src[5], 2);
    checksum += char_dst[5] + char_dst[6];
    
    return checksum;
}

// Test 2: Constant bounds but larger copy (should use library call)
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 3 long long elements (24 bytes on 64-bit) - likely exceeds threshold
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: i=0 to i=2 inclusive
        ll_dst[i] = ll_src[i];
        checksum += (int)(ll_dst[i] % 256);
    }
    
    // memcpy with constant size 16 bytes
    memcpy(&int_dst[2], &int_src[2], 4 * sizeof(int));
    for (int i = 2; i <= 5; ++i) {
        checksum += int_dst[i];
    }
    
    return checksum;
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    // Copy single element to register (scalar variable)
    int temp1 = int_src[3];
    checksum += temp1;
    
    // Copy single element to register from struct
    char temp2 = struct_src[1].a;
    checksum += temp2;
    
    // Copy single element to register from union
    int temp3 = union_src[0].i;
    checksum += temp3;
    
    // Multiple register copies
    long long temp4 = ll_src[2];
    int temp5 = int_src[4];
    checksum += (int)(temp4 % 256) + temp5;
    
    return checksum;
}

// Test 4: Struct and union copies
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy entire small struct (constant size)
    struct_dst[0] = struct_src[0];
    checksum += struct_dst[0].b;
    
    // Copy 2 struct elements (count <= 2)
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    checksum += struct_dst[1].b + struct_dst[2].b;
    
    // Copy union
    union_dst[0] = union_src[0];
    checksum += union_dst[0].i;
    
    // Copy struct array slice with constant bounds
    for (int i = 1; i <= 2; ++i) {  // Constant bounds
        struct_dst[i].a = struct_src[i].a;
        struct_dst[i].b = struct_src[i].b;
        struct_dst[i].c = struct_src[i].c;
        checksum += struct_dst[i].b;
    }
    
    return checksum;
}

// Test 5: Variable bounds (should not trigger const_bounds_p)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Copy with variable bounds - const_bounds_p should be false
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 8) {
            int_dst[i] = int_src[i];
            checksum += int_dst[i];
        }
    }
    
    // memcpy with variable size
    int size = end - start;
    if (size > 0 && size <= 8) {
        memcpy(&char_dst[start], &char_src[start], size);
        for (int i = start; i < end; ++i) {
            checksum += char_dst[i];
        }
    }
    
    return checksum;
}

// Test 6: Mixed scenarios
NOINLINE static int test_mixed_scenarios(void) {
    int checksum = 0;
    
    // Direct assignment of 2 elements (count <= 2)
    ll_dst[0] = ll_src[0];
    ll_dst[1] = ll_src[1];
    checksum += (int)(ll_dst[0] % 256) + (int)(ll_dst[1] % 256);
    
    // Copy 4 chars (4 bytes) - small total size
    for (int i = 0; i <= 3; ++i) {  // Constant bounds
        char_dst[i + 8] = char_src[i + 8];
        checksum += char_dst[i + 8];
    }
    
    // Register copy from middle of array
    int middle = int_src[4];
    checksum += middle;
    
    return checksum;
}

// Initialize test data
NOINLINE static void init_data(void) {
    // Initialize union data
    for (int i = 0; i < 4; ++i) {
        union_src[i].i = 1000 + i;
    }
}

int main(void) {
    init_data();
    
    int checksum = 0;
    
    // Run all tests
    checksum += test_const_small_memcpy();
    checksum += test_const_large_memcpy();
    checksum += test_register_target();
    checksum += test_struct_copy();
    checksum += test_variable_bounds(1, 3);  // Variable bounds
    checksum += test_mixed_scenarios();
    
    // Additional variable bounds test
    checksum += test_variable_bounds(0, 2);
    
    // Prevent dead code elimination
    return checksum & 0xFF;
}
