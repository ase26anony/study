#include <stdint.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Global arrays to avoid aliasing issues
static char char_src[32] = "abcdefghijklmnopqrstuvwxyz012345";
static char char_dst[32];
static int int_src[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
static int int_dst[16];
static long long ll_src[8] = {100,200,300,400,500,600,700,800};
static long long ll_dst[8];

// Small struct with constant size
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10}, {'b', 2, 20}, {'c', 3, 30}, {'d', 4, 40}
};
static struct SmallStruct struct_dst[4];

// Union to test different type handling
union MixedUnion {
    int i;
    float f;
    char c[4];
};

static union MixedUnion union_src[4];
static union MixedUnion union_dst[4];

// Test 1: Constant small memcpy (count <= 2 or small byte size)
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    // Copy exactly 2 chars (count <= 2, small byte size)
    for (int i = 2; i <= 3; ++i) {  // Constant bounds: i=2 to i=3 (count=2)
        char_dst[i] = char_src[i];
    }
    checksum += char_dst[2] + char_dst[3];
    
    // Copy 3 chars (count > 2 but total size = 3 bytes)
    for (int i = 0; i <= 2; ++i) {  // Constant bounds: i=0 to i=2 (count=3)
        char_dst[i+10] = char_src[i+10];
    }
    checksum += char_dst[10] + char_dst[11] + char_dst[12];
    
    // Copy 1 int (count=1, !MEM_P(target) case when assigned to register)
    int temp = int_src[3];  // Register target
    checksum += temp;
    
    // Copy 2 ints using memcpy with constant size (8 bytes on 32-bit)
    memcpy(&int_dst[1], &int_src[1], 2 * sizeof(int));
    checksum += int_dst[1] + int_dst[2];
    
    return checksum;
}

// Test 2: Constant bounds but larger copy (should use library call)
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    // Copy 8 long longs (64 bytes on 64-bit, likely above inline threshold)
    for (int i = 0; i <= 7; ++i) {  // Constant bounds: i=0 to i=7 (count=8)
        ll_dst[i] = ll_src[i];
    }
    checksum += (int)ll_dst[0] + (int)ll_dst[7];
    
    // Copy 6 ints (24 bytes)
    memcpy(&int_dst[5], &int_src[5], 6 * sizeof(int));
    for (int i = 5; i <= 10; ++i) {
        checksum += int_dst[i];
    }
    
    return checksum;
}

// Test 3: Register target (!MEM_P(target))
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    // Single element to register (count=1, !MEM_P(target))
    char c1 = char_src[5];
    int i1 = int_src[5];
    long long ll1 = ll_src[2];
    
    checksum += c1 + i1 + (int)ll1;
    
    // Two elements to separate registers
    char c2 = char_src[6];
    char c3 = char_src[7];
    checksum += c2 + c3;
    
    return checksum;
}

// Test 4: Struct copies (aggregate types)
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    // Copy single struct (count=1)
    struct SmallStruct temp = struct_src[0];
    checksum += temp.b + temp.c;
    
    // Copy 2 structs (count=2)
    for (int i = 1; i <= 2; ++i) {  // Constant bounds
        struct_dst[i] = struct_src[i];
    }
    checksum += struct_dst[1].b + struct_dst[2].c;
    
    // Copy struct using memcpy with constant size
    memcpy(&struct_dst[3], &struct_src[3], sizeof(struct SmallStruct));
    checksum += struct_dst[3].b;
    
    // Union copy
    union_src[0].i = 0x12345678;
    union_dst[0] = union_src[0];  // Aggregate assignment
    checksum += union_dst[0].i & 0xFF;
    
    return checksum;
}

// Test 5: Variable bounds (should fail const_bounds_p check)
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    // Variable bounds - not compile-time constants
    for (int i = start; i < end; ++i) {
        char_dst[i] = char_src[i];
        checksum += char_dst[i];
    }
    
    // Variable size memcpy
    int size = end - start;
    if (size > 0 && size < 16) {
        memcpy(&int_dst[start], &int_src[start], size * sizeof(int));
        for (int i = start; i < end; ++i) {
            checksum += int_dst[i];
        }
    }
    
    return checksum;
}

// Test 6: Mixed scenarios to hit edge cases
NOINLINE static int test_mixed_scenarios(void) {
    int checksum = 0;
    
    // Copy 3 chars from middle of array (constant bounds, small size)
    for (int i = 4; i <= 6; ++i) {
        char_dst[i+5] = char_src[i+5];
        checksum += char_dst[i+5];
    }
    
    // Copy 1 long long to register then to memory
    long long temp = ll_src[4];
    ll_dst[4] = temp;  // Register to memory
    checksum += (int)(temp % 256);
    
    // Nested constant bounds
    {
        const int offset = 2;
        for (int i = offset; i <= offset + 1; ++i) {  // Still constant after propagation
            int_dst[i] = int_src[i];
            checksum += int_dst[i];
        }
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    // Initialize union data
    for (int i = 0; i < 4; i++) {
        union_src[i].i = 0x1000 * (i + 1);
    }
    
    // Run all tests
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_variable_bounds(2, 5);  // Variable bounds
    total_checksum += test_mixed_scenarios();
    
    // Use results to prevent dead code elimination
    volatile int result = total_checksum;
    
    return result > 0 ? 0 : 1;
}
