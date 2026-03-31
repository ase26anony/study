#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char src_char[10] = {0,1,2,3,4,5,6,7,8,9};
static char dst_char[10] = {0};
static int src_int[10] = {0,10,20,30,40,50,60,70,80,90};
static int dst_int[10] = {0};
static long long src_ll[10] = {0,100,200,300,400,500,600,700,800,900};
static long long dst_ll[10] = {0};

// Small struct for aggregate type testing
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct src_struct[5] = {
    {1, 100, 1000},
    {2, 200, 2000},
    {3, 300, 3000},
    {4, 400, 4000},
    {5, 500, 5000}
};

static struct SmallStruct dst_struct[5] = {0};

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 2 chars (count=2, total size=2 bytes)
    // Should trigger count <= 2 branch
    for (int i = 2; i <= 3; ++i) {  // Constant bounds: i=2, i<=3
        dst_char[i] = src_char[i];
    }
    
    // Copy exactly 1 int (count=1)
    dst_int[0] = src_int[0];
    
    // Copy 2 ints (count=2)
    dst_int[1] = src_int[1];
    dst_int[2] = src_int[2];
    
    // Copy 3 chars (count=3, total size=3 bytes - small)
    // Should trigger size-based branch
    for (int i = 5; i <= 7; ++i) {  // Constant bounds: i=5, i<=7
        dst_char[i] = src_char[i];
    }
}

// Test 2: Constant large memcpy (total size > threshold)
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 3 long longs (count=3, total size=24 bytes on 64-bit)
    // May exceed inline threshold
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: i=0, i<=2
        dst_ll[i] = src_ll[i];
    }
    
    // Alternative using memcpy with constant size
    memcpy(&dst_int[3], &src_int[3], 3 * sizeof(int));  // 12 bytes
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // Copy single element to register variable
    int temp1 = src_int[3];  // Register target
    
    // Copy two elements to separate registers
    char temp2 = src_char[4];
    char temp3 = src_char[5];
    
    // Use the values to prevent dead code elimination
    dst_char[0] = (char)temp1;
    dst_char[1] = temp2 + temp3;
}

// Test 4: Struct copy (aggregate type)
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (count=1)
    dst_struct[0] = src_struct[0];
    
    // Copy two structs (count=2)
    dst_struct[1] = src_struct[1];
    dst_struct[2] = src_struct[2];
    
    // Copy struct slice with constant bounds
    for (int i = 2; i <= 4; ++i) {  // Constant bounds: i=2, i<=4
        dst_struct[i] = src_struct[i];
    }
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Loop bounds are function parameters (not compile-time constants)
    for (int i = start; i < end; ++i) {
        dst_int[i] = src_int[i];
    }
}

// Test 6: Mixed operations with unions
NOINLINE static void test_union_copy(void) {
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed src_union = {.i = 0x12345678};
    union Mixed dst_union;
    
    // Copy entire union (constant size, count=1)
    dst_union = src_union;
    
    // Copy union array elements with constant bounds
    union Mixed src_unions[3] = {{.i=1}, {.i=2}, {.i=3}};
    union Mixed dst_unions[3];
    
    for (int i = 0; i <= 2; ++i) {  // Constant bounds
        dst_unions[i] = src_unions[i];
    }
}

// Test 7: Edge cases with different element sizes
NOINLINE static void test_mixed_sizes(void) {
    // Copy 4 bytes as 4 chars (count=4, small)
    for (int i = 0; i <= 3; ++i) {
        dst_char[i+8] = src_char[i+8];
    }
    
    // Copy 4 bytes as 1 int (count=1)
    dst_int[9] = src_int[9];
}

// Compute checksum to prevent dead code elimination
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += dst_char[i];
        sum += dst_int[i];
        sum += (int)dst_ll[i];
    }
    for (int i = 0; i < 5; ++i) {
        sum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    return sum;
}

int main(void) {
    // Run all test cases
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(4, 7);  // Variable bounds
    test_union_copy();
    test_mixed_sizes();
    
    // Return checksum to prevent optimization
    return compute_checksum() > 0 ? 0 : 1;
}
