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
struct small_struct {
    char a;
    int b;
    char c;
};

static struct small_struct struct_src[5] = {
    {'a', 1, 'x'}, {'b', 2, 'y'}, {'c', 3, 'z'}, {'d', 4, 'w'}, {'e', 5, 'v'}
};
static struct small_struct struct_dst[5];

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Copy exactly 2 chars (count <= 2, small size)
    memcpy(&char_dst[0], &char_src[0], 2);
    checksum += char_dst[0] + char_dst[1];
    
    // Copy 3 chars (count > 2 but total size = 3 bytes, should be small)
    memcpy(&char_dst[2], &char_src[2], 3);
    checksum += char_dst[2] + char_dst[3] + char_dst[4];
    
    // Copy 2 ints (count <= 2)
    memcpy(&int_dst[0], &int_src[0], 2 * sizeof(int));
    checksum += int_dst[0] + int_dst[1];
    
    // Copy 3 ints (count > 2, total size = 12 bytes on 32-bit, 12/16 bytes)
    memcpy(&int_dst[2], &int_src[2], 3 * sizeof(int));
    checksum += int_dst[2] + int_dst[3] + int_dst[4];
    
    return checksum;
}

// Test 2: Constant large memcpy (exceeds inline threshold)
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 10 ints (40 bytes on 32-bit, 80 bytes on 64-bit)
    memcpy(int_dst, int_src, 10 * sizeof(int));
    for (int i = 0; i < 10; i++) {
        checksum += int_dst[i];
    }
    
    // Copy 10 long longs (80 bytes on 64-bit)
    memcpy(ll_dst, ll_src, 10 * sizeof(long long));
    for (int i = 0; i < 10; i++) {
        checksum += (int)ll_dst[i];
    }
    
    return checksum;
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    // Copy single element to register (scalar variable)
    int reg1 = int_src[3];  // Should trigger !MEM_P(target)
    checksum += reg1;
    
    char reg2 = char_src[5];
    checksum += reg2;
    
    long long reg3 = ll_src[2];
    checksum += (int)reg3;
    
    // Copy two elements to separate registers
    int reg4 = int_src[0];
    int reg5 = int_src[1];
    checksum += reg4 + reg5;
    
    return checksum;
}

// Test 4: Struct copies (aggregate types)
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy single struct (small aggregate)
    struct small_struct s1 = struct_src[0];
    checksum += s1.a + s1.b + s1.c;
    
    // Copy two structs (count <= 2)
    struct small_struct s2 = struct_src[1];
    struct small_struct s3 = struct_src[2];
    checksum += s2.a + s2.b + s2.c;
    checksum += s3.a + s3.b + s3.c;
    
    // Copy struct array slice with constant bounds
    for (int i = 1; i <= 3; i++) {  // Constant bounds: 1 to 3
        struct_dst[i] = struct_src[i];
        checksum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
    }
    
    // Use memcpy with struct
    memcpy(&struct_dst[4], &struct_src[4], sizeof(struct small_struct));
    checksum += struct_dst[4].a + struct_dst[4].b + struct_dst[4].c;
    
    return checksum;
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Variable bounds - compiler can't know these at compile time
    for (int i = start; i < end && i < 10; i++) {
        int_dst[i] = int_src[i];
        checksum += int_dst[i];
    }
    
    // Another variable bounds example
    int n = 3;
    memcpy(char_dst, char_src, n);  // Size not known at compile time
    for (int i = 0; i < n; i++) {
        checksum += char_dst[i];
    }
    
    return checksum;
}

// Test 6: Array slice with constant indices (explicit loops)
NOINLINE static int test_array_slice_constant(void) {
    int checksum = 0;
    
    // Copy slice [2..4] (3 elements) - constant bounds
    for (int i = 2; i <= 4; i++) {
        int_dst[i] = int_src[i];
        checksum += int_dst[i];
    }
    
    // Copy slice [0..1] (2 elements) - count <= 2
    for (int i = 0; i <= 1; i++) {
        char_dst[i] = char_src[i];
        checksum += char_dst[i];
    }
    
    // Copy slice [5..7] of long long (3 elements)
    for (int i = 5; i <= 7; i++) {
        ll_dst[i] = ll_src[i];
        checksum += (int)ll_dst[i];
    }
    
    return checksum;
}

// Test 7: Mixed operations to hit various conditions
NOINLINE static int test_mixed_operations(void) {
    int checksum = 0;
    
    // Direct assignment of 2 elements (count <= 2)
    int_dst[8] = int_src[8];
    int_dst[9] = int_src[9];
    checksum += int_dst[8] + int_dst[9];
    
    // Union test
    union {
        int i;
        float f;
        char c[4];
    } u1, u2;
    
    u1.i = 42;
    u2 = u1;  // Copy entire union
    checksum += u2.i;
    
    // Small array within struct
    struct {
        char data[4];
    } small1 = {{1,2,3,4}}, small2;
    
    small2 = small1;  // Copy struct with small array
    for (int i = 0; i < 4; i++) {
        checksum += small2.data[i];
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_variable_bounds(0, 3);  // Variable bounds
    total_checksum += test_array_slice_constant();
    total_checksum += test_mixed_operations();
    
    // Use the result to prevent dead code elimination
    return total_checksum == 0 ? 1 : 0;
}
