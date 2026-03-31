/* test_expr_coverage.c
 * Designed to trigger GCC's inline memory copy expansion logic
 * Compile with: gcc -std=gnu11 -O2 -fno-builtin-memcpy -fno-tree-loop-distribute-patterns -o test test_expr_coverage.c
 */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static int src_int[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int dst_int[10] = {0};
static char src_char[20] = "0123456789ABCDEFGHI";
static char dst_char[20] = {0};
static long long src_ll[5] = {100, 200, 300, 400, 500};
static long long dst_ll[5] = {0};

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct src_struct[3] = {
    {'a', 1, 10},
    {'b', 2, 20},
    {'c', 3, 30}
};
static struct SmallStruct dst_struct[3] = {0};

/* Test 1: Constant small memcpy (count <= 2 or small total size) */
NOINLINE static void test_const_small_memcpy(void) {
    /* Case 1: Copy exactly 2 ints (count <= 2 branch) */
    for (int i = 2; i <= 3; ++i) {  /* i=2,3 -> count=2 */
        dst_int[i] = src_int[i];
    }
    
    /* Case 2: Copy 3 chars (small total size: 3 bytes) */
    for (int i = 1; i <= 3; ++i) {  /* i=1,2,3 -> count=3 */
        dst_char[i] = src_char[i];
    }
    
    /* Case 3: Direct memcpy with constant size 2 */
    memcpy(&dst_int[5], &src_int[5], 2 * sizeof(int));
    
    /* Case 4: Copy 1 element (count=1) */
    dst_int[0] = src_int[0];
}

/* Test 2: Constant "large" copy (more than 2 elements, larger total size) */
NOINLINE static void test_const_large_memcpy(void) {
    /* Copy 3 long longs (24 bytes on 64-bit) - should be above threshold */
    for (int i = 0; i <= 2; ++i) {  /* i=0,1,2 -> count=3 */
        dst_ll[i] = src_ll[i];
    }
    
    /* Copy 4 ints (16 bytes) */
    for (int i = 4; i <= 7; ++i) {  /* i=4,5,6,7 -> count=4 */
        dst_int[i] = src_int[i];
    }
}

/* Test 3: Register target (!MEM_P(target)) */
NOINLINE static void test_register_target(void) {
    /* Copy single element to register variable */
    int reg_temp1 = src_int[3];  /* Should trigger !MEM_P(target) */
    char reg_temp2 = src_char[5];
    long long reg_temp3 = src_ll[2];
    
    /* Use the values to prevent elimination */
    dst_int[0] += reg_temp1;
    dst_char[0] += reg_temp2;
    dst_ll[0] += reg_temp3;
}

/* Test 4: Struct copies (aggregate types) */
NOINLINE static void test_struct_copy(void) {
    /* Copy entire struct (constant size) */
    dst_struct[0] = src_struct[0];
    
    /* Copy 2 structs */
    for (int i = 0; i <= 1; ++i) {  /* count=2 */
        dst_struct[i] = src_struct[i];
    }
    
    /* Copy struct field by field (might trigger different expansion) */
    dst_struct[2].a = src_struct[2].a;
    dst_struct[2].b = src_struct[2].b;
    dst_struct[2].c = src_struct[2].c;
}

/* Test 5: Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Loop with variable bounds - should go to library call path */
    for (int i = start; i <= end; ++i) {
        dst_int[i] = src_int[i];
    }
    
    /* Variable-sized memcpy */
    int count = end - start + 1;
    if (count > 0 && count < 10) {
        memcpy(&dst_char[start], &src_char[start], count * sizeof(char));
    }
}

/* Test 6: Mixed types and edge cases */
NOINLINE static void test_mixed_cases(void) {
    /* Union test */
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed src_union = {.i = 0x12345678};
    union Mixed dst_union;
    
    /* Copy entire union */
    dst_union = src_union;
    
    /* Copy array slice with constant bounds but different element sizes */
    short src_short[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    short dst_short[8] = {0};
    
    /* Copy 4 shorts (8 bytes) */
    for (int i = 1; i <= 4; ++i) {  /* count=4 */
        dst_short[i] = src_short[i];
    }
    
    /* Copy with pointer arithmetic (still constant bounds) */
    int *src_ptr = &src_int[2];
    int *dst_ptr = &dst_int[2];
    for (int i = 0; i < 2; ++i) {  /* count=2 */
        dst_ptr[i] = src_ptr[i];
    }
}

/* Main function that calls all tests and returns checksum */
int main(void) {
    int checksum = 0;
    
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(1, 3);  /* Variable bounds */
    test_mixed_cases();
    
    /* Calculate checksum to prevent dead code elimination */
    for (int i = 0; i < 10; ++i) {
        checksum += dst_int[i];
    }
    
    for (int i = 0; i < 20; ++i) {
        checksum += dst_char[i];
    }
    
    for (int i = 0; i < 5; ++i) {
        checksum += (int)(dst_ll[i] % 256);
    }
    
    for (int i = 0; i < 3; ++i) {
        checksum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    
    return checksum & 0xFF;  /* Return non-zero, non-constant value */
}
