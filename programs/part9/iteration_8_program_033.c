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
    char b;
    char c;
};

static struct SmallStruct src_struct[5] = {{1,2,3},{4,5,6},{7,8,9},{10,11,12},{13,14,15}};
static struct SmallStruct dst_struct[5] = {{0}};

// Union for testing
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion src_union[3] = {{.i=0x12345678}, {.f=3.14f}, {.c={'a','b','c','d'}}};
static union TestUnion dst_union[3] = {{0}};

// Test 1: Constant small memcpy (count <= 2 or small total size)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Copy exactly 1 element - should trigger count <= 2 path
    memcpy(&dst_char[0], &src_char[0], 1 * sizeof(char));
    checksum += dst_char[0];
    
    // Copy exactly 2 elements - should trigger count <= 2 path
    memcpy(&dst_char[1], &src_char[1], 2 * sizeof(char));
    checksum += dst_char[1] + dst_char[2];
    
    // Copy 3 chars (3 bytes total) - small total size path
    memcpy(&dst_char[3], &src_char[3], 3 * sizeof(char));
    checksum += dst_char[3] + dst_char[4] + dst_char[5];
    
    // Copy 2 ints (8 bytes on 64-bit) - count <= 2 path
    memcpy(&dst_int[0], &src_int[0], 2 * sizeof(int));
    checksum += dst_int[0] + dst_int[1];
    
    // Copy 3 chars using loop with constant bounds
    for (int i = 6; i <= 8; ++i) {
        dst_char[i] = src_char[i];
        checksum += dst_char[i];
    }
    
    return checksum;
}

// Test 2: Constant large memcpy (size exceeds threshold)
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 3 long longs (24 bytes on 64-bit) - might exceed threshold
    memcpy(&dst_ll[0], &src_ll[0], 3 * sizeof(long long));
    checksum += (int)dst_ll[0] + (int)dst_ll[1] + (int)dst_ll[2];
    
    // Copy 5 ints (20 bytes) using loop with constant bounds
    for (int i = 2; i <= 6; ++i) {
        dst_int[i] = src_int[i];
        checksum += dst_int[i];
    }
    
    return checksum;
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    // Copy single element to register (scalar variable)
    int temp1 = src_int[3];
    checksum += temp1;
    
    // Copy single element to register with different type
    char temp2 = src_char[4];
    checksum += temp2;
    
    // Copy struct element to register
    struct SmallStruct temp3 = src_struct[2];
    checksum += temp3.a + temp3.b + temp3.c;
    
    // Multiple register copies
    int temp4 = src_int[0];
    int temp5 = src_int[1];
    checksum += temp4 + temp5;
    
    return checksum;
}

// Test 4: Struct and union copies
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy entire struct (3 bytes)
    dst_struct[0] = src_struct[0];
    checksum += dst_struct[0].a + dst_struct[0].b + dst_struct[0].c;
    
    // Copy 2 structs (6 bytes)
    memcpy(&dst_struct[1], &src_struct[1], 2 * sizeof(struct SmallStruct));
    checksum += dst_struct[1].a + dst_struct[2].a;
    
    // Copy union (4 bytes)
    dst_union[0] = src_union[0];
    checksum += dst_union[0].c[0];
    
    // Copy 2 unions (8 bytes)
    memcpy(&dst_union[1], &src_union[1], 2 * sizeof(union TestUnion));
    checksum += dst_union[1].c[0] + dst_union[2].c[0];
    
    return checksum;
}

// Test 5: Variable bounds (should NOT trigger const_bounds_p)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Copy with variable bounds - const_bounds_p should be false
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 10) {
            dst_char[i] = src_char[i];
            checksum += dst_char[i];
        }
    }
    
    // Another variable bounds copy
    int count = end - start;
    if (count > 0 && count <= 10) {
        memcpy(&dst_int[start], &src_int[start], count * sizeof(int));
        for (int i = start; i < start + count; ++i) {
            if (i >= 0 && i < 10) {
                checksum += dst_int[i];
            }
        }
    }
    
    return checksum;
}

// Test 6: Mixed scenarios
NOINLINE static int test_mixed_scenarios(void) {
    int checksum = 0;
    
    // Direct assignment of array elements (should generate inline moves)
    dst_int[7] = src_int[7];
    dst_int[8] = src_int[8];
    checksum += dst_int[7] + dst_int[8];
    
    // Small memcpy with constant size 4 (might be inlined)
    memcpy(&dst_char[9], &src_char[9], 1);
    checksum += dst_char[9];
    
    // Array slice copy with constant bounds but larger size
    for (int i = 0; i <= 4; ++i) {
        dst_ll[i] = src_ll[i];
        checksum += (int)dst_ll[i];
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    // Run all tests
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_variable_bounds(1, 4);  // Variable bounds
    total_checksum += test_mixed_scenarios();
    
    // Additional edge cases
    // Copy single element to force register target
    volatile int single_copy = src_int[5];
    total_checksum += single_copy;
    
    // Very small copy that should definitely be inlined
    char tiny_src[2] = {42, 43};
    char tiny_dst[2];
    memcpy(tiny_dst, tiny_src, 2);
    total_checksum += tiny_dst[0] + tiny_dst[1];
    
    // Return checksum to prevent dead code elimination
    return total_checksum > 0 ? 0 : 1;
}
