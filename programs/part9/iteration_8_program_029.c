/* test_expr_coverage.c - Target GCC expr.cc lines 7691-7700 */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our test cases */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global arrays to avoid aliasing issues */
static char char_src[16] = "0123456789ABCDEF";
static char char_dst[16];
static int int_src[8] = {10, 20, 30, 40, 50, 60, 70, 80};
static int int_dst[8];
static long long ll_src[4] = {100, 200, 300, 400};
static long long ll_dst[4];

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10}, {'b', 2, 20}, {'c', 3, 30}, {'d', 4, 40}
};
static struct SmallStruct struct_dst[4];

/* Test 1: Constant small memcpy (count <= 2) */
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    /* Case 1: Copy exactly 2 chars (count <= 2, small size) */
    memcpy(&char_dst[2], &char_src[2], 2 * sizeof(char));
    checksum += char_dst[2] + char_dst[3];
    
    /* Case 2: Copy exactly 2 ints (count <= 2, larger element size) */
    memcpy(&int_dst[1], &int_src[1], 2 * sizeof(int));
    checksum += int_dst[1] + int_dst[2];
    
    /* Case 3: Copy 3 chars (count > 2 but total size small) */
    memcpy(&char_dst[5], &char_src[5], 3 * sizeof(char));
    checksum += char_dst[5] + char_dst[6] + char_dst[7];
    
    return checksum;
}

/* Test 2: Constant large memcpy (count > 2 and total size > threshold) */
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    /* Copy 3 long longs (24 bytes on 64-bit, likely above threshold) */
    memcpy(&ll_dst[0], &ll_src[0], 3 * sizeof(long long));
    checksum += (int)(ll_dst[0] + ll_dst[1] + ll_dst[2]);
    
    /* Copy 4 ints (16 bytes) */
    memcpy(&int_dst[4], &int_src[4], 4 * sizeof(int));
    checksum += int_dst[4] + int_dst[5] + int_dst[6] + int_dst[7];
    
    return checksum;
}

/* Test 3: Register target (!MEM_P(target)) */
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    /* Single element copy to register variable */
    int temp1 = int_src[3];
    checksum += temp1;
    
    /* Two element copy via separate register assignments */
    char temp2 = char_src[0];
    char temp3 = char_src[1];
    checksum += temp2 + temp3;
    
    /* Struct element to register */
    struct SmallStruct temp4 = struct_src[2];
    checksum += temp4.b + temp4.c;
    
    return checksum;
}

/* Test 4: Struct and union copies */
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    /* Direct struct assignment (should use inline copy) */
    struct SmallStruct s1 = struct_src[0];
    struct SmallStruct s2;
    s2 = s1;  /* This should trigger the logic */
    checksum += s2.b;
    
    /* Copy between struct array elements with constant bounds */
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    checksum += struct_dst[1].b + struct_dst[2].b;
    
    /* Copy 2 structs via memcpy (count <= 2) */
    memcpy(&struct_dst[0], &struct_src[0], 2 * sizeof(struct SmallStruct));
    checksum += struct_dst[0].c + struct_dst[1].c;
    
    return checksum;
}

/* Test 5: Variable bounds (should NOT hit the uncovered lines) */
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    /* Loop with variable bounds - const_bounds_p should be false */
    for (int i = start; i < end && i < 8; i++) {
        int_dst[i] = int_src[i];
        checksum += int_dst[i];
    }
    
    /* memcpy with variable size */
    int size = (end - start) * sizeof(int);
    if (size > 0 && size < 32) {
        memcpy(&char_dst[start], &char_src[start], size);
        for (int i = start; i < end && i < 16; i++) {
            checksum += char_dst[i];
        }
    }
    
    return checksum;
}

/* Test 6: Explicit constant-bound loops (alternative to memcpy) */
NOINLINE static int test_constant_loop(void) {
    int checksum = 0;
    
    /* Loop with compile-time constant bounds */
    for (int i = 2; i <= 4; ++i) {  /* count = 3 */
        int_dst[i] = int_src[i];
        checksum += int_dst[i];
    }
    
    /* Another with different constant bounds */
    for (int i = 0; i < 2; ++i) {  /* count = 2 */
        ll_dst[i] = ll_src[i];
        checksum += (int)ll_dst[i];
    }
    
    return checksum;
}

/* Test 7: Mixed element sizes and counts */
NOINLINE static int test_mixed_sizes(void) {
    int checksum = 0;
    
    /* Copy 1 element (count = 1) */
    int_dst[0] = int_src[0];
    checksum += int_dst[0];
    
    /* Copy 2 elements (count = 2) */
    int_dst[1] = int_src[1];
    int_dst[2] = int_src[2];
    checksum += int_dst[1] + int_dst[2];
    
    /* Copy 4 chars (count = 4, small total size) */
    char temp[4];
    for (int i = 0; i < 4; i++) {
        temp[i] = char_src[i + 8];
        checksum += temp[i];
    }
    
    return checksum;
}

/* Main function that runs all tests */
int main(void) {
    int total_checksum = 0;
    
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_variable_bounds(1, 3);  /* Variable bounds */
    total_checksum += test_constant_loop();
    total_checksum += test_mixed_sizes();
    
    /* Use the results to prevent dead code elimination */
    volatile int result = total_checksum;
    
    return result != 0 ? 0 : 1;
}
