/* test_expr_coverage.c - Coverage test for GCC expr.cc array copy expansion */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to maintain function boundaries for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static char char_src[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
static char char_dst[32];
static int int_src[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
static int int_dst[16];
static long long ll_src[8] = {100,200,300,400,500,600,700,800};
static long long ll_dst[8];

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10},
    {'b', 2, 20},
    {'c', 3, 30},
    {'d', 4, 40}
};
static struct SmallStruct struct_dst[4];

/* Union for testing */
union MixedUnion {
    int i;
    float f;
    char c[4];
};

static union MixedUnion union_src[4];
static union MixedUnion union_dst[4];

/* Test 1: Constant bounds, count <= 2 (triggers count <= 2 branch) */
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    /* Copy exactly 1 element - should trigger !MEM_P(target) or count <= 2 */
    int temp = int_src[3];  /* Register target */
    checksum += temp;
    
    /* Copy exactly 2 elements - count <= 2 branch */
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    checksum += int_dst[0] + int_dst[1];
    
    /* Copy 2 chars - small element size */
    char_dst[5] = char_src[5];
    char_dst[6] = char_src[6];
    checksum += char_dst[5] + char_dst[6];
    
    /* memcpy with constant size 2 (bytes) */
    memcpy(&char_dst[10], &char_src[10], 2);
    checksum += char_dst[10] + char_dst[11];
    
    return checksum;
}

/* Test 2: Constant bounds, count > 2 but small total size */
NOINLINE static int test_const_small_but_many_elements(void) {
    int checksum = 0;
    
    /* Copy 3 chars - TYPE_SIZE * count = 3 bytes (small) */
    for (int i = 2; i <= 4; ++i) {  /* Constant bounds: 2 to 4 inclusive = 3 elements */
        char_dst[i] = char_src[i];
        checksum += char_dst[i];
    }
    
    /* Copy 4 chars via memcpy - 4 bytes total */
    memcpy(&char_dst[15], &char_src[15], 4);
    for (int i = 15; i < 19; ++i) {
        checksum += char_dst[i];
    }
    
    /* Copy 3 ints - TYPE_SIZE * count = 12 bytes (may be threshold dependent) */
    int_dst[5] = int_src[5];
    int_dst[6] = int_src[6];
    int_dst[7] = int_src[7];
    checksum += int_dst[5] + int_dst[6] + int_dst[7];
    
    return checksum;
}

/* Test 3: Constant bounds, larger element size */
NOINLINE static int test_const_large_elements(void) {
    int checksum = 0;
    
    /* Copy 3 long longs - TYPE_SIZE * count = 24 bytes (64-bit) */
    ll_dst[0] = ll_src[0];
    ll_dst[1] = ll_src[1];
    ll_dst[2] = ll_src[2];
    checksum += (int)(ll_dst[0] + ll_dst[1] + ll_dst[2]);
    
    /* memcpy with constant size 24 bytes */
    memcpy(&ll_dst[3], &ll_src[3], 3 * sizeof(long long));
    checksum += (int)(ll_dst[3] + ll_dst[4] + ll_dst[5]);
    
    return checksum;
}

/* Test 4: Struct copies (aggregate types) */
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    /* Copy single struct - register target */
    struct SmallStruct temp = struct_src[1];
    checksum += temp.b + temp.c;
    
    /* Copy 2 structs - count <= 2 */
    struct_dst[0] = struct_src[0];
    struct_dst[1] = struct_src[1];
    checksum += struct_dst[0].b + struct_dst[1].b;
    
    /* Copy 3 structs - TYPE_SIZE * count may exceed threshold */
    struct_dst[2] = struct_src[2];
    checksum += struct_dst[2].b;
    
    /* memcpy with struct size */
    memcpy(&struct_dst[3], &struct_src[3], sizeof(struct SmallStruct));
    checksum += struct_dst[3].b;
    
    return checksum;
}

/* Test 5: Union copies */
NOINLINE static int test_union_copy(void) {
    int checksum = 0;
    
    /* Initialize source unions */
    for (int i = 0; i < 4; ++i) {
        union_src[i].i = 1000 + i;
    }
    
    /* Copy single union */
    union MixedUnion temp = union_src[0];
    checksum += temp.i;
    
    /* Copy 2 unions */
    union_dst[0] = union_src[0];
    union_dst[1] = union_src[1];
    checksum += union_dst[0].i + union_dst[1].i;
    
    /* Copy 3 unions */
    memcpy(&union_dst[2], &union_src[2], 3 * sizeof(union MixedUnion));
    checksum += union_dst[2].i + union_dst[3].i;
    
    return checksum;
}

/* Test 6: Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    /* Variable bounds - const_bounds_p should be false */
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 16) {
            int_dst[i] = int_src[i];
            checksum += int_dst[i];
        }
    }
    
    /* Variable size memcpy */
    int size = end - start;
    if (size > 0 && size <= 8) {
        memcpy(&char_dst[20], &char_src[20], size);
        for (int i = 20; i < 20 + size; ++i) {
            checksum += char_dst[i];
        }
    }
    
    return checksum;
}

/* Test 7: Mixed patterns in same function */
NOINLINE static int test_mixed_patterns(void) {
    int checksum = 0;
    
    /* Multiple small constant copies in sequence */
    char local_dst[8];
    char local_src[8] = "1234567";
    
    /* Individual assignments (count = 1 each) */
    local_dst[0] = local_src[0];
    local_dst[1] = local_src[1];
    checksum += local_dst[0] + local_dst[1];
    
    /* Pair assignment (count = 2) */
    local_dst[2] = local_src[2];
    local_dst[3] = local_src[3];
    checksum += local_dst[2] + local_dst[3];
    
    /* Small memcpy (count = 4, 4 bytes) */
    memcpy(&local_dst[4], &local_src[4], 4);
    for (int i = 4; i < 8; ++i) {
        checksum += local_dst[i];
    }
    
    return checksum;
}

/* Main function that runs all tests */
int main(void) {
    int total_checksum = 0;
    
    /* Initialize union array */
    for (int i = 0; i < 4; ++i) {
        union_src[i].i = 1000 + i * 100;
    }
    
    /* Run all test cases */
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_small_but_many_elements();
    total_checksum += test_const_large_elements();
    total_checksum += test_struct_copy();
    total_checksum += test_union_copy();
    total_checksum += test_variable_bounds(2, 5);  /* Variable bounds */
    total_checksum += test_mixed_patterns();
    
    /* Use results to prevent dead code elimination */
    volatile int result = total_checksum;
    
    return result > 0 ? 0 : 1;
}
