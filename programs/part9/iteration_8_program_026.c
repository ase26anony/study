#include <stdint.h>
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
    int b;
    char c;
};

static struct SmallStruct struct_src[5] = {{'a',1,'x'},{'b',2,'y'},{'c',3,'z'}};
static struct SmallStruct struct_dst[5];

// Union for testing
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion union_src[5];
static union TestUnion union_dst[5];

// Test 1: Small memcpy with constant size (should trigger count <= 2 branch)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Copy exactly 2 chars (count <= 2, small element size)
    memcpy(&char_dst[2], &char_src[2], 2 * sizeof(char));
    checksum += char_dst[2] + char_dst[3];
    
    // Copy 1 int (count <= 2, larger element but still small)
    memcpy(&int_dst[3], &int_src[3], 1 * sizeof(int));
    checksum += int_dst[3];
    
    // Copy 2 ints (count <= 2)
    memcpy(&int_dst[0], &int_src[0], 2 * sizeof(int));
    checksum += int_dst[0] + int_dst[1];
    
    // Copy 3 chars (count > 2 but total size = 3 bytes, small)
    memcpy(&char_dst[5], &char_src[5], 3 * sizeof(char));
    checksum += char_dst[5] + char_dst[6] + char_dst[7];
    
    return checksum;
}

// Test 2: Larger memcpy with constant size but still constant bounds
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 3 long longs (count > 2 and total size = 24 bytes on 64-bit)
    // This should test the size threshold logic
    memcpy(&ll_dst[1], &ll_src[1], 3 * sizeof(long long));
    checksum += (int)(ll_dst[1] + ll_dst[2] + ll_dst[3]);
    
    // Copy 5 ints (count > 2, total size = 20 bytes)
    memcpy(&int_dst[2], &int_src[2], 5 * sizeof(int));
    for (int i = 2; i < 7; i++) checksum += int_dst[i];
    
    return checksum;
}

// Test 3: Register target (!MEM_P(target) branch)
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    // Copy single element to register (scalar variable)
    int temp1 = int_src[3];  // Should trigger !MEM_P(target)
    checksum += temp1;
    
    char temp2 = char_src[4];  // Another register target
    checksum += temp2;
    
    long long temp3 = ll_src[2];  // Register target with larger type
    checksum += (int)temp3;
    
    // Copy two elements - might still use registers
    int temp4 = int_src[0];
    int temp5 = int_src[1];
    checksum += temp4 + temp5;
    
    return checksum;
}

// Test 4: Struct and union copies
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy single struct (count = 1, aggregate type)
    struct_dst[0] = struct_src[0];
    checksum += struct_dst[0].b + struct_dst[0].a;
    
    // Copy two structs (count = 2)
    memcpy(&struct_dst[1], &struct_src[1], 2 * sizeof(struct SmallStruct));
    checksum += struct_dst[1].b + struct_dst[2].b;
    
    // Copy union
    union_src[0].i = 42;
    union_dst[0] = union_src[0];  // Struct assignment
    checksum += union_dst[0].i;
    
    // Copy 3 unions via memcpy (count > 2)
    for (int i = 0; i < 3; i++) union_src[i].i = i * 10;
    memcpy(&union_dst[0], &union_src[0], 3 * sizeof(union TestUnion));
    for (int i = 0; i < 3; i++) checksum += union_dst[i].i;
    
    return checksum;
}

// Test 5: Array copies with constant bounds using loops
NOINLINE static int test_const_bounds_loop(void) {
    int checksum = 0;
    
    // Loop with constant bounds (i=2 to i=4 inclusive, count=3)
    for (int i = 2; i <= 4; ++i) {
        int_dst[i] = int_src[i];
        checksum += int_dst[i];
    }
    
    // Another loop with different constant bounds
    for (int i = 0; i < 2; ++i) {  // count=2
        char_dst[i] = char_src[i];
        checksum += char_dst[i];
    }
    
    // Loop copying structs with constant bounds
    for (int i = 1; i <= 3; ++i) {  // count=3
        struct_dst[i] = struct_src[i];
        checksum += struct_dst[i].b;
    }
    
    return checksum;
}

// Test 6: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Loop with variable bounds (from function parameters)
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 10) {
            int_dst[i] = int_src[i];
            checksum += int_dst[i];
        }
    }
    
    // Another variable bounds copy
    int n = 3;
    for (int i = 0; i < n; ++i) {
        char_dst[i] = char_src[i];
        checksum += char_dst[i];
    }
    
    return checksum;
}

// Test 7: Mixed patterns to hit various conditions
NOINLINE static int test_mixed_patterns(void) {
    int checksum = 0;
    
    // Direct assignment of array elements (might generate individual loads/stores)
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];  // Two elements, count=2
    checksum += int_dst[0] + int_dst[1];
    
    // Three element direct assignment
    char_dst[0] = char_src[0];
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];  // Three chars, count=3, total 3 bytes
    checksum += char_dst[0] + char_dst[1] + char_dst[2];
    
    // Copy between different array sections with constant indices
    int temp[3];
    temp[0] = int_src[5];
    temp[1] = int_src[6];
    temp[2] = int_src[7];  // Three elements to non-MEM target? (temp is array)
    checksum += temp[0] + temp[1] + temp[2];
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    // Initialize source arrays
    for (int i = 0; i < 10; i++) {
        char_src[i] = 'A' + i;
        int_src[i] = i * 10;
        ll_src[i] = i * 100;
    }
    
    // Run all tests
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_const_bounds_loop();
    total_checksum += test_variable_bounds(1, 4);  // Variable bounds
    total_checksum += test_mixed_patterns();
    
    // Use the results to prevent dead code elimination
    return total_checksum % 256;
}
