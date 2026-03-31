#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char char_src[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
static char char_dst[32];
static int int_src[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
static int int_dst[16];
static long long ll_src[8] = {100,200,300,400,500,600,700,800};
static long long ll_dst[8];

// Small struct for aggregate type testing
struct SmallStruct {
    char a;
    int b;
    short c;
    char d;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10, 'x'},
    {'b', 2, 20, 'y'},
    {'c', 3, 30, 'z'},
    {'d', 4, 40, 'w'}
};
static struct SmallStruct struct_dst[4];

// Test 1: Constant bounds, small count (<=2), memory target
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 2 chars - should trigger count <= 2 branch
    for (int i = 2; i <= 3; ++i) {  // lo=2, hi=3, count=2
        char_dst[i] = char_src[i];
    }
    
    // Copy exactly 1 int - should trigger count <= 2 branch
    int_dst[0] = int_src[0];
    
    // Copy exactly 2 ints - should trigger count <= 2 branch
    int_dst[1] = int_src[1];
    int_dst[2] = int_src[2];
    
    // Use memcpy with constant size 2 (chars) - count <= 2
    memcpy(&char_dst[4], &char_src[4], 2);
    
    // Use memcpy with constant size 8 (2 ints on 32-bit) - count <= 2
    memcpy(&int_dst[3], &int_src[3], 2 * sizeof(int));
}

// Test 2: Constant bounds, count > 2 but small total size
NOINLINE static void test_const_small_total_size(void) {
    // Copy 3 chars - total size = 3 bytes, small enough to inline
    for (int i = 5; i <= 7; ++i) {  // lo=5, hi=7, count=3
        char_dst[i] = char_src[i];
    }
    
    // Copy 4 chars via memcpy - total size = 4 bytes
    memcpy(&char_dst[8], &char_src[8], 4);
    
    // Copy 3 shorts - total size = 6 bytes, likely small
    short short_src[5] = {10, 20, 30, 40, 50};
    short short_dst[5];
    for (int i = 0; i <= 2; ++i) {  // count = 3
        short_dst[i] = short_src[i];
    }
}

// Test 3: Constant bounds, count > 2 and larger total size
NOINLINE static void test_const_large_total_size(void) {
    // Copy 3 long longs - total size = 24 bytes (on 64-bit)
    for (int i = 0; i <= 2; ++i) {  // lo=0, hi=2, count=3
        ll_dst[i] = ll_src[i];
    }
    
    // Copy 4 ints via memcpy - total size = 16 bytes
    memcpy(&int_dst[8], &int_src[8], 4 * sizeof(int));
}

// Test 4: Non-MEM_P target (register)
NOINLINE static void test_register_target(void) {
    // Copy single element to register - !MEM_P(target) should be true
    int temp1 = int_src[5];  // Register target
    
    // Copy single struct element to register
    struct SmallStruct temp2 = struct_src[1];  // Register target
    
    // Multiple register copies
    char reg1 = char_src[10];
    int reg2 = int_src[10];
    long long reg3 = ll_src[3];
    
    // Use the values to prevent optimization
    char_dst[10] = reg1;
    int_dst[10] = reg2;
    ll_dst[3] = reg3;
}

// Test 5: Struct copies (aggregate types)
NOINLINE static void test_struct_copy(void) {
    // Copy entire struct - constant size, count=1
    struct_dst[0] = struct_src[0];
    
    // Copy 2 structs - count=2
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    // Copy struct slice (3 elements) - count=3
    for (int i = 0; i <= 2; ++i) {
        struct_dst[i] = struct_src[i];
    }
    
    // Use memcpy with struct
    memcpy(&struct_dst[3], &struct_src[3], sizeof(struct SmallStruct));
}

// Test 6: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - const_bounds_p should be false
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 16) {
            int_dst[i] = int_src[i];
        }
    }
    
    // Variable size memcpy
    int size = end - start + 1;
    if (size > 0 && size <= 16) {
        memcpy(&char_dst[16], &char_src[16], size);
    }
}

// Test 7: Union copies
NOINLINE static void test_union_copy(void) {
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed u_src = { .i = 0x12345678 };
    union Mixed u_dst;
    
    // Copy entire union
    u_dst = u_src;
    
    // Copy union array element
    union Mixed u_arr_src[2] = {{.i = 1}, {.f = 2.0}};
    union Mixed u_arr_dst[2];
    u_arr_dst[0] = u_arr_src[0];
    u_arr_dst[1] = u_arr_src[1];
}

// Test 8: Edge cases with different element types
NOINLINE static void test_mixed_types(void) {
    // Mixed constant-bound copies
    float f_src[4] = {1.1, 2.2, 3.3, 4.4};
    float f_dst[4];
    
    // Copy 2 floats
    f_dst[0] = f_src[0];
    f_dst[1] = f_src[1];
    
    // Copy 3 floats via loop
    for (int i = 0; i <= 2; ++i) {
        f_dst[i] = f_src[i];
    }
    
    // Pointer copies
    void* ptr_src[3] = {&char_src, &int_src, &ll_src};
    void* ptr_dst[3];
    for (int i = 0; i <= 1; ++i) {  // count=2
        ptr_dst[i] = ptr_src[i];
    }
}

// Compute checksum to prevent dead code elimination
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        sum += char_dst[i];
    }
    
    for (int i = 0; i < 16; i++) {
        sum += int_dst[i];
    }
    
    for (int i = 0; i < 8; i++) {
        sum += (int)(ll_dst[i] & 0xFF);
    }
    
    for (int i = 0; i < 4; i++) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c + struct_dst[i].d;
    }
    
    return sum;
}

int main(void) {
    // Initialize destination arrays
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    // Run all tests
    test_const_small_memcpy();
    test_const_small_total_size();
    test_const_large_total_size();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 5);  // Variable bounds
    test_union_copy();
    test_mixed_types();
    
    // Compute and return checksum
    return compute_checksum() & 0xFF;
}
