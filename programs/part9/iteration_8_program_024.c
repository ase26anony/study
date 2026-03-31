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

// Small struct for testing aggregate types
struct SmallStruct {
    char a;
    int b;
    short c;
    char d;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 2, 'x'},
    {'b', 3, 4, 'y'},
    {'c', 5, 6, 'z'},
    {'d', 7, 8, 'w'}
};
static struct SmallStruct struct_dst[4];

// Test 1: Constant small memcpy (count <= 2 or small byte size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 2 chars (count=2, total size=2 bytes)
    // Should trigger count <= 2 branch
    for (int i = 2; i <= 3; ++i) {  // lo=2, hi=3, count=2
        char_dst[i] = char_src[i];
    }
    
    // Copy 3 chars (count=3, total size=3 bytes)
    // Should trigger size-based branch if 3 bytes is below threshold
    for (int i = 0; i <= 2; ++i) {  // lo=0, hi=2, count=3
        char_dst[i + 10] = char_src[i + 10];
    }
    
    // Use memcpy with constant size 2 (alternative path)
    memcpy(&char_dst[5], &char_src[5], 2);
}

// Test 2: Constant "large" memcpy (count > 2 and larger byte size)
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 3 long longs (count=3, total size=24 bytes on 64-bit)
    // May exceed inline threshold
    for (int i = 1; i <= 3; ++i) {  // lo=1, hi=3, count=3
        ll_dst[i] = ll_src[i];
    }
    
    // Copy 4 ints (count=4, total size=16 bytes on 32/64-bit)
    for (int i = 4; i <= 7; ++i) {  // lo=4, hi=7, count=4
        int_dst[i] = int_src[i];
    }
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // Copy single element to register (scalar variable)
    int temp1 = int_src[3];  // Should trigger !MEM_P(target) branch
    
    // Copy two elements to separate registers
    long long temp2 = ll_src[0];
    long long temp3 = ll_src[1];
    
    // Use the values to prevent dead code elimination
    int_dst[0] = temp1;
    ll_dst[7] = temp2 + temp3;
}

// Test 4: Struct copies (aggregate types)
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (count=1)
    struct_dst[0] = struct_src[0];
    
    // Copy two structs (count=2)
    for (int i = 1; i <= 2; ++i) {  // lo=1, hi=2, count=2
        struct_dst[i] = struct_src[i];
    }
    
    // Copy using memcpy with constant struct size
    memcpy(&struct_dst[3], &struct_src[3], sizeof(struct SmallStruct));
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Loop with variable bounds - const_bounds_p should be false
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 16) {
            int_dst[i] = int_src[i];
        }
    }
    
    // Variable-sized memcpy
    int size = end - start + 1;
    if (size > 0 && size <= 8) {
        memcpy(&char_dst[start], &char_src[start], size);
    }
}

// Test 6: Mixed scenarios with unions
NOINLINE static void test_union_copy(void) {
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    static union Mixed union_src = {.i = 0x12345678};
    static union Mixed union_dst[2];
    
    // Copy entire union (count=1 for aggregate type)
    union_dst[0] = union_src;
    
    // Copy two union elements
    union_dst[1] = union_dst[0];
}

// Main function that runs all tests
int main(void) {
    int checksum = 0;
    
    // Run all test functions
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 5);  // Variable bounds
    test_union_copy();
    
    // Compute checksum to prevent dead code elimination
    for (int i = 0; i < 32; i++) {
        checksum += char_dst[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += int_dst[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += (int)(ll_dst[i] & 0xFFFFFFFF);
    }
    
    return checksum & 0xFF;  // Return non-zero value
}
