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

static union MixedUnion union_src[4] = {
    {.i = 42},
    {.f = 3.14f},
    {.c = {'x','y','z','\0'}},
    {.i = 99}
};
static union MixedUnion union_dst[4];

// 1. Constant small memcpy (count <= 2 or small byte size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 1 element - should trigger count <= 2 path
    char_dst[0] = char_src[0];
    
    // Copy exactly 2 elements - should trigger count <= 2 path
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    
    // Copy 3 chars (3 bytes total) - small byte size path
    for (int i = 3; i <= 5; ++i) {  // Constant bounds: 3 to 5 inclusive
        char_dst[i] = char_src[i];
    }
    
    // memcpy with constant size 2
    memcpy(&char_dst[6], &char_src[6], 2);
    
    // memcpy with constant size 3 (small byte size)
    memcpy(&char_dst[8], &char_src[8], 3);
}

// 2. Constant large memcpy (count > 2 and large byte size)
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 3 long long elements (24 bytes on 64-bit) - large byte size
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: 0 to 2 inclusive
        ll_dst[i] = ll_src[i];
    }
    
    // Copy 4 int elements (16 bytes on 32-bit, 16 bytes on 64-bit)
    for (int i = 1; i <= 4; ++i) {  // Constant bounds: 1 to 4 inclusive
        int_dst[i] = int_src[i];
    }
    
    // memcpy with constant size 16 bytes
    memcpy(&int_dst[0], &int_src[0], 4 * sizeof(int));
}

// 3. Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // Copy single element to register (scalar variable)
    int temp1 = int_src[3];  // Register target
    
    // Copy multiple elements to different registers
    char temp2 = char_src[5];
    long long temp3 = ll_src[2];
    
    // Use the values to prevent optimization
    char_dst[10] = temp2;
    int_dst[5] = temp1;
    ll_dst[3] = temp3;
}

// 4. Struct and union copies
NOINLINE static void test_struct_copy(void) {
    // Copy single struct element (aggregate type)
    struct_dst[0] = struct_src[0];
    
    // Copy 2 struct elements
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    // Copy union elements
    union_dst[0] = union_src[0];
    union_dst[1] = union_src[1];
    
    // Copy struct array slice with constant bounds
    for (int i = 0; i <= 1; ++i) {  // Constant bounds: 0 to 1 inclusive
        struct_dst[i + 2] = struct_src[i + 2];
    }
}

// 5. Variable bounds (const_bounds_p should be false)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Loop with variable bounds - should NOT trigger inline expansion
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 8) {
            int_dst[i] = int_src[i];
        }
    }
    
    // memcpy with variable size
    size_t size = (end - start) * sizeof(int);
    if (size > 0 && size <= sizeof(int_dst)) {
        memcpy(&int_dst[start], &int_src[start], size);
    }
}

// 6. Mixed scenarios
NOINLINE static void test_mixed_scenarios(void) {
    // Copy 2 elements of different types
    char_dst[12] = char_src[12];
    char_dst[13] = char_src[13];
    
    // Copy with pointer arithmetic but constant offset
    int *p = &int_src[2];
    int_dst[2] = p[0];  // Single element
    int_dst[3] = p[1];  // Another single element
    
    // Nested constant bounds
    const int offset = 2;
    for (int i = offset; i <= offset + 1; ++i) {  // Still constant bounds
        char_dst[i + 10] = char_src[i + 10];
    }
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
        sum += (int)ll_dst[i];
    }
    
    for (int i = 0; i < 4; i++) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
    }
    
    for (int i = 0; i < 4; i++) {
        sum += union_dst[i].i;
    }
    
    return sum;
}

int main(void) {
    // Clear destination arrays
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    memset(union_dst, 0, sizeof(union_dst));
    
    // Execute all test functions
    test_const_small_memcpy();      // Hits count <= 2 and small byte size paths
    test_const_large_memcpy();      // Hits large byte size path
    test_register_target();         // Hits !MEM_P(target) path
    test_struct_copy();             // Hits aggregate type paths
    test_variable_bounds(1, 3);     // Should NOT hit inline expansion
    test_mixed_scenarios();         // Additional mixed cases
    
    // Return checksum to prevent optimization
    return compute_checksum() % 256;
}
