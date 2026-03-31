#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char src_char[10] = "abcdefghi";
static char dst_char[10];
static int src_int[10] = {0,1,2,3,4,5,6,7,8,9};
static int dst_int[10];
static long long src_ll[10] = {0,1,2,3,4,5,6,7,8,9};
static long long dst_ll[10];

// Small struct for testing
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct src_struct[5] = {{'a',1,2},{'b',3,4},{'c',5,6},{'d',7,8},{'e',9,10}};
static struct SmallStruct dst_struct[5];

// Union for testing
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion src_union[5];
static union TestUnion dst_union[5];

// 1. Constant small memcpy - should trigger count <= 2 or small size branch
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 2 chars (count <= 2, small size)
    for (int i = 2; i <= 3; ++i) {  // Constant bounds: i=2 to i=3 (2 elements)
        dst_char[i] = src_char[i];
    }
    
    // Copy 3 chars - total size = 3 bytes (small)
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: i=0 to i=2 (3 elements)
        dst_char[i + 5] = src_char[i + 5];
    }
    
    // Use memcpy with constant size 2 (should inline)
    memcpy(&dst_char[7], &src_char[7], 2 * sizeof(char));
}

// 2. Constant "large" copy - should test size threshold logic
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 3 long longs - total size = 24 bytes (may be above threshold)
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: i=0 to i=2 (3 elements)
        dst_ll[i] = src_ll[i];
    }
    
    // Copy 2 ints - total size = 8 bytes (may be inline threshold)
    for (int i = 3; i <= 4; ++i) {  // Constant bounds: i=3 to i=4 (2 elements)
        dst_int[i] = src_int[i];
    }
}

// 3. Register target - should trigger !MEM_P(target) branch
NOINLINE static void test_register_target(void) {
    // Copy single element to register (scalar variable)
    int temp1 = src_int[0];  // Register target
    int temp2 = src_int[1];  // Register target
    dst_int[0] = temp1;
    dst_int[1] = temp2;
    
    // Copy struct element to register
    struct SmallStruct temp_struct = src_struct[0];  // Register target
    dst_struct[0] = temp_struct;
}

// 4. Struct and union copies
NOINLINE static void test_struct_copy(void) {
    // Copy entire small struct (constant size, count=1)
    dst_struct[1] = src_struct[1];
    
    // Copy 2 struct elements
    for (int i = 2; i <= 3; ++i) {  // Constant bounds: i=2 to i=3 (2 elements)
        dst_struct[i] = src_struct[i];
    }
    
    // Copy union
    dst_union[0] = src_union[0];
    
    // Copy 2 union elements
    for (int i = 1; i <= 2; ++i) {  // Constant bounds: i=1 to i=2 (2 elements)
        dst_union[i] = src_union[i];
    }
}

// 5. Variable bounds - should NOT trigger const_bounds_p path
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - compiler can't determine as constant
    for (int i = start; i <= end; ++i) {
        dst_int[i] = src_int[i];
    }
}

// 6. Additional tests for edge cases
NOINLINE static void test_edge_cases(void) {
    // Single element copy (count = 1)
    dst_int[5] = src_int[5];
    
    // Two element copy with pointer arithmetic
    int *src_ptr = &src_int[6];
    int *dst_ptr = &dst_int[6];
    dst_ptr[0] = src_ptr[0];
    dst_ptr[1] = src_ptr[1];
    
    // Mixed types in same array section
    char mixed_src[20];
    char mixed_dst[20];
    for (int i = 0; i < 4; ++i) {  // Constant bounds
        mixed_dst[i] = mixed_src[i];
    }
}

// Main function that calls all tests
int main(void) {
    // Initialize source arrays
    for (int i = 0; i < 10; i++) {
        src_int[i] = i * 10;
        src_ll[i] = i * 100LL;
    }
    
    for (int i = 0; i < 5; i++) {
        src_struct[i].a = 'A' + i;
        src_struct[i].b = i * 100;
        src_struct[i].c = i * 10;
        
        src_union[i].i = i * 1000;
    }
    
    // Run all test functions
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 4);  // Variable bounds
    test_edge_cases();
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += dst_char[i];
        checksum += dst_int[i];
        checksum += (int)dst_ll[i];
    }
    
    for (int i = 0; i < 5; i++) {
        checksum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
        checksum += dst_union[i].i;
    }
    
    return checksum == 0 ? 0 : 1;
}
