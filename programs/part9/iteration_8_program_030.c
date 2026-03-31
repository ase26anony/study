/* test_expr_coverage.c - Target GCC expr.cc lines 7691-7700 */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static char char_src[16] = "0123456789ABCDEF";
static char char_dst[16];
static int int_src[8] = {10, 20, 30, 40, 50, 60, 70, 80};
static int int_dst[8];
static long long ll_src[4] = {100LL, 200LL, 300LL, 400LL};
static long long ll_dst[4];

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 100},
    {'b', 2, 200},
    {'c', 3, 300},
    {'d', 4, 400}
};
static struct SmallStruct struct_dst[4];

/* Test 1: Small memcpy with constant size (1-3 bytes) */
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    /* Copy 1 byte - should trigger count <= 2 path */
    memcpy(&char_dst[2], &char_src[2], 1);
    checksum += char_dst[2];
    
    /* Copy 2 bytes - should trigger count <= 2 path */
    memcpy(&char_dst[4], &char_src[4], 2);
    checksum += char_dst[4] + char_dst[5];
    
    /* Copy 3 bytes of char - total size small (3 bytes) */
    memcpy(&char_dst[8], &char_src[8], 3);
    checksum += char_dst[8] + char_dst[9] + char_dst[10];
    
    return checksum;
}

/* Test 2: Constant bounds with loop (non-MEM_P target) */
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    /* Copy single element to register (!MEM_P target) */
    int temp1 = int_src[3];  /* Should be expanded as move to register */
    checksum += temp1;
    
    /* Copy two elements to registers */
    int temp2 = int_src[0];
    int temp3 = int_src[1];
    checksum += temp2 + temp3;
    
    /* Copy with constant bounds loop - target is memory */
    for (int i = 2; i <= 4; ++i) {  /* count = 3 */
        int_dst[i] = int_src[i];
        checksum += int_dst[i];
    }
    
    return checksum;
}

/* Test 3: Struct copies (aggregate types) */
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    /* Copy single struct (small constant size) */
    struct SmallStruct s1 = struct_src[0];
    checksum += s1.b + s1.c;
    
    /* Copy two structs */
    struct SmallStruct s2 = struct_src[1];
    struct SmallStruct s3 = struct_src[2];
    checksum += s2.b + s3.b;
    
    /* Copy struct array slice with constant bounds */
    for (int i = 1; i <= 2; ++i) {  /* count = 2 */
        struct_dst[i] = struct_src[i];
        checksum += struct_dst[i].b;
    }
    
    return checksum;
}

/* Test 4: Varying element sizes and counts */
NOINLINE static int test_varying_sizes(void) {
    int checksum = 0;
    
    /* Copy 3 chars (3 bytes total - small) */
    for (int i = 0; i < 3; ++i) {
        char_dst[i] = char_src[i + 10];
        checksum += char_dst[i];
    }
    
    /* Copy 3 long longs (24 bytes on 64-bit - may exceed threshold) */
    for (int i = 0; i < 3; ++i) {  /* count = 3, but large total size */
        ll_dst[i] = ll_src[i];
        checksum += (int)(ll_dst[i] % 256);
    }
    
    /* Copy exactly 2 ints (8 bytes - should use count <= 2 path) */
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    checksum += int_dst[0] + int_dst[1];
    
    return checksum;
}

/* Test 5: Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    /* Loop with variable bounds - const_bounds_p should be false */
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 8) {
            int_dst[i] = int_src[i];
            checksum += int_dst[i];
        }
    }
    
    /* memcpy with variable size */
    int size = end - start;
    if (size > 0 && size <= 16) {
        memcpy(&char_dst[start], &char_src[start], size);
        for (int i = 0; i < size; ++i) {
            checksum += char_dst[start + i];
        }
    }
    
    return checksum;
}

/* Test 6: Union copies */
NOINLINE static int test_union_copy(void) {
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed u_src = { .i = 0x41424344 };
    union Mixed u_dst;
    int checksum = 0;
    
    /* Copy entire union (constant size) */
    u_dst = u_src;
    checksum += u_dst.c[0] + u_dst.c[1];
    
    /* Copy union array slice */
    union Mixed u_arr_src[3] = {{.i=1}, {.i=2}, {.i=3}};
    union Mixed u_arr_dst[3];
    
    for (int i = 0; i < 2; ++i) {  /* count = 2 */
        u_arr_dst[i] = u_arr_src[i];
        checksum += u_arr_dst[i].i;
    }
    
    return checksum;
}

/* Test 7: Mixed operations to hit different branches */
NOINLINE static int test_mixed_operations(void) {
    int checksum = 0;
    
    /* Direct assignment of array elements (count = 1) */
    char_dst[15] = char_src[15];
    checksum += char_dst[15];
    
    /* Two assignments (count = 2) */
    int_dst[6] = int_src[6];
    int_dst[7] = int_src[7];
    checksum += int_dst[6] + int_dst[7];
    
    /* Three assignments of small type (char) */
    char_dst[12] = char_src[12];
    char_dst[13] = char_src[13];
    char_dst[14] = char_src[14];
    checksum += char_dst[12] + char_dst[13] + char_dst[14];
    
    return checksum;
}

/* Main function that runs all tests */
int main(void) {
    int total_checksum = 0;
    
    /* Run all test functions */
    total_checksum += test_const_small_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_varying_sizes();
    total_checksum += test_variable_bounds(1, 4);  /* Variable bounds */
    total_checksum += test_union_copy();
    total_checksum += test_mixed_operations();
    
    /* Use the results to prevent dead code elimination */
    return total_checksum & 0xFF;  /* Return lower byte as exit code */
}
