#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char src_char[10] = {0,1,2,3,4,5,6,7,8,9};
static char dst_char[10] = {0};
static int src_int[10] = {0,1,2,3,4,5,6,7,8,9};
static int dst_int[10] = {0};
static long long src_ll[10] = {0,1,2,3,4,5,6,7,8,9};
static long long dst_ll[10] = {0};

// Small struct for testing
struct SmallStruct {
    char a;
    int b;
    char c;
};

static struct SmallStruct src_struct[5] = {{0}};
static struct SmallStruct dst_struct[5] = {{0}};

// Union for testing
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion src_union[5] = {{0}};
static union TestUnion dst_union[5] = {{0}};

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static void test_const_small_memcpy(void) {
    // Copy exactly 1 element - should trigger count <= 2 path
    memcpy(&dst_char[0], &src_char[0], 1 * sizeof(char));
    
    // Copy exactly 2 elements - should trigger count <= 2 path
    memcpy(&dst_char[1], &src_char[1], 2 * sizeof(char));
    
    // Copy 3 chars = 3 bytes total (small size)
    memcpy(&dst_char[3], &src_char[3], 3 * sizeof(char));
    
    // Copy 2 ints = 8 bytes on 64-bit (count <= 2)
    memcpy(&dst_int[0], &src_int[0], 2 * sizeof(int));
    
    // Copy 3 ints = 12 bytes (might be above threshold depending on target)
    memcpy(&dst_int[3], &src_int[3], 3 * sizeof(int));
}

// Test 2: Constant large memcpy (total size > threshold)
NOINLINE static void test_const_large_memcpy(void) {
    // Copy 10 long longs = 80 bytes on 64-bit (likely above inline threshold)
    memcpy(&dst_ll[0], &src_ll[0], 10 * sizeof(long long));
    
    // Copy 10 ints = 40 bytes
    memcpy(&dst_int[0], &src_int[0], 10 * sizeof(int));
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static void test_register_target(void) {
    // These should create register targets
    int temp1 = src_int[0];          // Single element to register
    int temp2 = src_int[1];          // Another single element
    long long temp3 = src_ll[2];     // Larger type to register
    
    // Store back to memory to prevent elimination
    dst_int[0] = temp1;
    dst_int[1] = temp2;
    dst_ll[0] = temp3;
}

// Test 4: Struct and union copies
NOINLINE static void test_struct_copy(void) {
    // Copy single struct (aggregate type)
    dst_struct[0] = src_struct[0];
    
    // Copy two structs
    dst_struct[1] = src_struct[1];
    dst_struct[2] = src_struct[2];
    
    // Copy union
    dst_union[0] = src_union[0];
    
    // Copy array of structs with constant bounds
    for (int i = 0; i < 2; ++i) {  // Constant bounds
        dst_struct[i+3] = src_struct[i+3];
    }
}

// Test 5: Array copies with constant bounds using loops
NOINLINE static void test_const_bounds_loop(void) {
    // Loop with compile-time constant bounds
    for (int i = 2; i <= 4; ++i) {  // count = 3
        dst_int[i] = src_int[i];
    }
    
    // Another constant bounds loop
    for (int i = 0; i < 3; ++i) {  // count = 3
        dst_char[i] = src_char[i];
    }
    
    // Loop with step
    for (int i = 1; i < 10; i += 2) {  // Not simple range, but bounds are constants
        dst_ll[i] = src_ll[i];
    }
}

// Test 6: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static void test_variable_bounds(int start, int end) {
    // Variable bounds - should go through library call path
    for (int i = start; i < end; ++i) {
        dst_int[i] = src_int[i];
    }
    
    // Another variable bounds example
    int count = end - start;
    if (count > 0) {
        memcpy(&dst_char[start], &src_char[start], count * sizeof(char));
    }
}

// Test 7: Mixed assignments to trigger different paths
NOINLINE static void test_mixed_assignments(void) {
    // Direct assignments (register targets)
    dst_int[0] = src_int[0];
    dst_int[1] = src_int[1];
    
    // Two-element copy
    dst_int[2] = src_int[2];
    dst_int[3] = src_int[3];
    
    // Three-element copy (might inline or call memcpy)
    dst_int[4] = src_int[4];
    dst_int[5] = src_int[5];
    dst_int[6] = src_int[6];
}

// Test 8: Edge cases with different element sizes
NOINLINE static void test_edge_cases(void) {
    // Copy 3 chars = 3 bytes (small)
    char small_src[3] = {1, 2, 3};
    char small_dst[3];
    memcpy(small_dst, small_src, 3);
    
    // Copy 3 long longs = 24 bytes (might be threshold boundary)
    long long large_src[3] = {1, 2, 3};
    long long large_dst[3];
    memcpy(large_dst, large_src, 3 * sizeof(long long));
    
    // Single element of large type
    large_dst[0] = large_src[0];
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
        sum += dst_union[i].i;
    }
    return sum;
}

int main(void) {
    // Initialize source arrays
    for (int i = 0; i < 10; ++i) {
        src_char[i] = i;
        src_int[i] = i;
        src_ll[i] = i;
    }
    for (int i = 0; i < 5; ++i) {
        src_struct[i].a = i;
        src_struct[i].b = i * 10;
        src_struct[i].c = i * 20;
        src_union[i].i = i * 30;
    }
    
    // Run all tests
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_const_bounds_loop();
    test_variable_bounds(1, 4);  // Variable bounds
    test_mixed_assignments();
    test_edge_cases();
    
    // Compute and return checksum
    return compute_checksum() & 0xFF;  // Return lower byte
}
