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
static long long ll_src[4] = {0x1122334455667788LL, 0x2233445566778899LL,
                              0x33445566778899AALL, 0x445566778899AABBLL};
static long long ll_dst[4];

// Small struct for testing
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 100}, {'b', 2, 200}, {'c', 3, 300}, {'d', 4, 400}
};
static struct SmallStruct struct_dst[4];

// Test 1: Small constant memcpy (count <= 2 or small total size)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Copy exactly 1 element - should trigger count <= 2 path
    char_dst[0] = char_src[0];
    checksum += char_dst[0];
    
    // Copy exactly 2 elements - should trigger count <= 2 path
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    checksum += char_dst[1] + char_dst[2];
    
    // Copy 3 chars (3 bytes total) - small total size
    for (int i = 3; i <= 5; ++i) {  // Constant bounds: i=3 to i=5 (3 elements)
        char_dst[i] = char_src[i];
        checksum += char_dst[i];
    }
    
    // Copy 2 ints (8 bytes on 32-bit, 8 bytes on 64-bit) - count <= 2
    for (int i = 0; i <= 1; ++i) {  // Constant bounds: i=0 to i=1 (2 elements)
        int_dst[i] = int_src[i];
        checksum += int_dst[i];
    }
    
    return checksum;
}

// Test 2: Larger constant memcpy (total size may exceed threshold)
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 3 long longs (24 bytes on 64-bit) - may exceed inline threshold
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: i=0 to i=2 (3 elements)
        ll_dst[i] = ll_src[i];
        checksum += (int)(ll_dst[i] & 0xFF);
    }
    
    // Copy 4 ints (16 bytes on 32-bit) - may be near threshold
    for (int i = 2; i <= 5; ++i) {  // Constant bounds: i=2 to i=5 (4 elements)
        int_dst[i] = int_src[i];
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
    
    // Copy single element to register from struct array
    char temp2 = struct_src[1].a;
    checksum += temp2;
    
    // Copy single element to register with constant index
    long long temp3 = ll_src[2];
    checksum += (int)(temp3 & 0xFF);
    
    return checksum;
}

// Test 4: Struct copies (aggregate types)
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy entire struct (constant size, small)
    struct_dst[0] = struct_src[0];
    checksum += struct_dst[0].b;
    
    // Copy 2 struct elements - count <= 2
    for (int i = 1; i <= 2; ++i) {  // Constant bounds: i=1 to i=2 (2 elements)
        struct_dst[i] = struct_src[i];
        checksum += struct_dst[i].b;
    }
    
    // Copy struct field to register
    int temp = struct_src[3].b;
    checksum += temp;
    
    return checksum;
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p path)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Loop with variable bounds - const_bounds_p should be false
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 8) {
            int_dst[i] = int_src[i];
            checksum += int_dst[i];
        }
    }
    
    // Variable memcpy size
    size_t size = (end - start + 1) * sizeof(int);
    if (size > 0 && size <= sizeof(int_dst)) {
        memcpy(&int_dst[start], &int_src[start], size);
        for (int i = start; i <= end; ++i) {
            if (i >= 0 && i < 8) {
                checksum += int_dst[i];
            }
        }
    }
    
    return checksum;
}

// Test 6: Mixed operations with different element sizes
NOINLINE static int test_mixed_operations(void) {
    int checksum = 0;
    
    // Copy 1 byte (char) - smallest possible
    char_dst[10] = char_src[10];
    checksum += char_dst[10];
    
    // Copy 2 bytes (short) - count=1 for short type
    short short_src[4] = {100, 200, 300, 400};
    short short_dst[4];
    short_dst[0] = short_src[0];
    checksum += short_dst[0];
    
    // Copy 4 bytes (int) - count=1
    int_dst[7] = int_src[7];
    checksum += int_dst[7];
    
    // Copy 8 bytes (long long) - count=1
    ll_dst[3] = ll_src[3];
    checksum += (int)(ll_dst[3] & 0xFF);
    
    return checksum;
}

// Test 7: Union copies
NOINLINE static int test_union_copy(void) {
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    static union Mixed union_src = {.i = 0x12345678};
    union Mixed union_dst;
    int checksum = 0;
    
    // Copy entire union (constant size)
    union_dst = union_src;
    checksum += union_dst.c[0];
    
    // Copy union field to register
    int temp = union_src.i;
    checksum += temp;
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    // Run all tests
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_variable_bounds(1, 3);  // Variable bounds
    total_checksum += test_mixed_operations();
    total_checksum += test_union_copy();
    
    // Use results to prevent dead code elimination
    return total_checksum & 0xFF;
}
