#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Small struct for testing aggregate types
struct SmallStruct {
    char a;
    int b;
    short c;
};

// Union for testing different type sizes
union MixedUnion {
    int i;
    float f;
    char arr[8];
};

// Global arrays to avoid aliasing issues
static int src_int[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int dst_int[10] = {0};
static char src_char[20] = "0123456789abcdef";
static char dst_char[20] = {0};
static struct SmallStruct src_struct[5] = {{'a', 1, 2}, {'b', 3, 4}, {'c', 5, 6}};
static struct SmallStruct dst_struct[5] = {{0}};
static union MixedUnion src_union[4] = {{0}};
static union MixedUnion dst_union[4] = {{0}};

// 1. Constant small memcpy - should trigger count <= 2 branch
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 2 elements - triggers count <= 2
    for (int i = 2; i <= 3; ++i) {  // Constant bounds: i=2, i<=3
        dst_int[i] = src_int[i];
    }
    
    // Copy 1 element - triggers count <= 2
    dst_int[4] = src_int[4];
    
    // Copy 3 chars (3 bytes) - tests TYPE_SIZE * count calculation
    for (int i = 1; i <= 3; ++i) {  // Constant bounds: i=1, i<=3
        dst_char[i] = src_char[i];
    }
    
    // Alternative using memcpy with constant size
    memcpy(&dst_char[5], &src_char[5], 2 * sizeof(char));  // 2 bytes
}

// 2. Constant "large" memcpy - tests TYPE_SIZE * count threshold
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 3 long longs (24 bytes on 64-bit) - may exceed inline threshold
    long long src_ll[5] = {100, 200, 300, 400, 500};
    long long dst_ll[5] = {0};
    
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: i=0, i<=2
        dst_ll[i] = src_ll[i];
    }
    
    // Copy 4 ints (16 bytes) - may be near threshold
    for (int i = 1; i <= 4; ++i) {  // Constant bounds: i=1, i<=4
        dst_int[i + 2] = src_int[i + 2];
    }
}

// 3. Register target - triggers !MEM_P(target) path
NOINLINE static void test_register_target(void) {
    // Copy single element to register (scalar variable)
    int temp1 = src_int[3];  // Register target
    
    // Copy multiple elements to different registers
    int temp2 = src_int[4];
    int temp3 = src_int[5];
    
    // Use the values to prevent optimization
    dst_int[0] = temp1 + temp2 + temp3;
    
    // Copy struct element to register
    struct SmallStruct temp_struct = src_struct[1];
    dst_struct[0] = temp_struct;
}

// 4. Struct copy - tests aggregate types
NOINLINE static void test_struct_copy(void) {
    // Copy entire struct - constant size
    dst_struct[1] = src_struct[1];
    
    // Copy 2 structs - count <= 2 with aggregate type
    for (int i = 0; i <= 1; ++i) {  // Constant bounds: i=0, i<=1
        dst_struct[i + 2] = src_struct[i + 2];
    }
    
    // Copy union
    dst_union[0] = src_union[0];
    dst_union[1] = src_union[1];
}

// 5. Variable bounds - should NOT trigger const_bounds_p
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - const_bounds_p should be false
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 10) {
            dst_int[i] = src_int[i];
        }
    }
    
    // Another variable bounds example
    int n = 3;
    for (int i = 0; i < n; ++i) {
        dst_char[i] = src_char[i];
    }
}

// 6. Additional edge cases
NOINLINE static void test_edge_cases(void) {
    // Copy between different array types with constant bounds
    uint16_t src_short[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint16_t dst_short[8] = {0};
    
    // Copy 3 shorts (6 bytes) - tests size calculation
    for (int i = 1; i <= 3; ++i) {  // Constant bounds
        dst_short[i] = src_short[i];
    }
    
    // Copy using pointer arithmetic with constant offsets
    int *src_ptr = src_int + 2;
    int *dst_ptr = dst_int + 2;
    for (int i = 0; i < 2; ++i) {  // Constant bounds
        dst_ptr[i] = src_ptr[i];
    }
    
    // Nested struct with constant bounds
    struct Nested {
        struct SmallStruct s;
        int extra;
    } src_nested[3], dst_nested[3];
    
    for (int i = 0; i <= 1; ++i) {  // Constant bounds
        dst_nested[i] = src_nested[i];
    }
}

// Main function that calls all tests
int main(void) {
    int checksum = 0;
    
    // Initialize source data
    for (int i = 0; i < 10; i++) {
        src_int[i] = i * 10;
    }
    
    for (int i = 0; i < 20; i++) {
        src_char[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 5; i++) {
        src_struct[i].a = 'a' + i;
        src_struct[i].b = i * 100;
        src_struct[i].c = i * 10;
    }
    
    src_union[0].i = 0x12345678;
    src_union[1].f = 3.14159f;
    
    // Run all test cases
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 5);  // Variable bounds
    test_edge_cases();
    
    // Calculate checksum to prevent dead code elimination
    for (int i = 0; i < 10; i++) {
        checksum += dst_int[i];
    }
    
    for (int i = 0; i < 20; i++) {
        checksum += dst_char[i];
    }
    
    return checksum & 0xFF;  // Return non-zero value
}
