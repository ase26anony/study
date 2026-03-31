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

// Small struct for aggregate type testing
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 100}, {'b', 2, 200}, {'c', 3, 300}, {'d', 4, 400}
};
static struct SmallStruct struct_dst[4];

// Union for testing
union TestUnion {
    int i;
    float f;
    char arr[4];
};

static union TestUnion union_src[2] = {{.i = 0x12345678}, {.f = 3.14f}};
static union TestUnion union_dst[2];

// Test 1: Constant small memcpy (count <= 2 or small byte size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy 1 element - should trigger count <= 2 path
    char_dst[0] = char_src[0];
    
    // Copy 2 elements - should trigger count <= 2 path
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    
    // Copy 3 chars (3 bytes total) - small byte size path
    for (int i = 3; i <= 5; ++i) {  // Constant bounds: 3 to 5 inclusive
        char_dst[i] = char_src[i];
    }
    
    // memcpy with constant size 2 (count <= 2 for char)
    memcpy(&char_dst[6], &char_src[6], 2);
    
    // memcpy with constant size 3 (small byte size for char)
    memcpy(&char_dst[8], &char_src[8], 3);
}

// Test 2: Constant bounds but larger byte size
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 3 long longs (24 bytes on 64-bit) - larger than threshold
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: 0 to 2 inclusive
        ll_dst[i] = ll_src[i];
    }
    
    // memcpy with constant size 24 bytes
    memcpy(&ll_dst[1], &ll_src[1], 3 * sizeof(long long));
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // Copy single element to register variable
    int temp1 = int_src[0];  // Should trigger !MEM_P(target) path
    int temp2 = int_src[1];
    int temp3 = int_src[2];
    
    // Store back to memory to prevent elimination
    int_dst[0] = temp1;
    int_dst[1] = temp2;
    int_dst[2] = temp3;
    
    // Register target with struct
    struct SmallStruct reg_struct = struct_src[0];
    struct_dst[0] = reg_struct;
}

// Test 4: Struct and union copies
NOINLINE static void test_struct_copy(void) {
    // Copy entire small struct (7 bytes on most 64-bit with padding)
    struct_dst[1] = struct_src[1];
    
    // Copy 2 structs (count = 2)
    for (int i = 2; i <= 3; ++i) {  // Constant bounds: 2 to 3 inclusive
        struct_dst[i] = struct_src[i];
    }
    
    // Union copy
    union_dst[0] = union_src[0];
    union_dst[1] = union_src[1];
    
    // memcpy with struct type and constant size
    memcpy(&struct_dst[0], &struct_src[0], sizeof(struct SmallStruct));
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - should go through library call path
    for (int i = start; i < end; ++i) {
        int_dst[i] = int_src[i];
    }
    
    // Variable size memcpy
    int size = end - start;
    if (size > 0) {
        memcpy(&char_dst[start], &char_src[start], size);
    }
}

// Test 6: Mixed constant/variable patterns
NOINLINE static void test_mixed_patterns(void) {
    // Constant index with variable offset
    int offset = 1;
    int_dst[offset + 0] = int_src[offset + 0];
    int_dst[offset + 1] = int_src[offset + 1];
    
    // Nested constant bounds
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            char_dst[idx] = char_src[idx];
        }
    }
}

// Compute checksum to prevent dead code elimination
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 16; ++i) {
        sum += char_dst[i];
    }
    
    for (int i = 0; i < 8; ++i) {
        sum += int_dst[i];
    }
    
    for (int i = 0; i < 4; ++i) {
        sum += (int)(ll_dst[i] & 0xFFFFFFFF);
        sum += (int)(ll_dst[i] >> 32);
    }
    
    for (int i = 0; i < 4; ++i) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
    }
    
    for (int i = 0; i < 2; ++i) {
        sum += union_dst[i].i;
    }
    
    return sum;
}

int main(void) {
    // Initialize destination arrays
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    memset(union_dst, 0, sizeof(union_dst));
    
    // Run all tests
    test_const_small_memcpy();      // Hits count <= 2 and small byte size
    test_const_large_memcpy();      // Hits constant bounds but larger size
    test_register_target();         // Hits !MEM_P(target) path
    test_struct_copy();             // Hits aggregate types
    test_variable_bounds(2, 5);     // Should NOT hit const_bounds_p
    test_mixed_patterns();          // Additional patterns
    
    // Return checksum to prevent optimization
    return compute_checksum() & 0xFF;
}
