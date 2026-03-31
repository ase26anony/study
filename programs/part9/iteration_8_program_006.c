/* test_expr_coverage.c
 * Designed to trigger GCC's inline memory copy expansion logic
 * Compile with: gcc -O2 -fno-builtin-memcpy -fno-tree-loop-distribute-patterns -std=gnu11
 */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to maintain function boundaries for coverage */
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
    {'a', 1, 2},
    {'b', 3, 4},
    {'c', 5, 6}
};
static struct SmallStruct dst_struct[3] = {0};

/* Union for testing */
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion src_union = {.i = 0x12345678};
static union TestUnion dst_union = {0};

/* 1. Constant bounds, small count (<=2), memory target */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy exactly 2 ints - should trigger count <= 2 branch */
    for (int i = 2; i <= 3; ++i) {  /* lo=2, hi=3, count=2 */
        dst_int[i] = src_int[i];
    }
    
    /* Copy 1 char - count=1 <= 2 */
    dst_char[0] = src_char[0];
    
    /* Copy 2 chars via memcpy with constant size */
    memcpy(&dst_char[5], &src_char[5], 2 * sizeof(char));
    
    /* Copy 3 chars - total size = 3 bytes, small enough to inline */
    for (int i = 10; i <= 12; ++i) {  /* lo=10, hi=12, count=3 */
        dst_char[i] = src_char[i];
    }
}

/* 2. Constant bounds, larger element size */
NOINLINE static void test_const_large_element(void) {
    /* Copy 3 long longs - 24 bytes on 64-bit, tests size threshold */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        dst_ll[i] = src_ll[i];
    }
    
    /* Copy 2 long longs - 16 bytes, count=2 <= 2 */
    dst_ll[3] = src_ll[3];
    dst_ll[4] = src_ll[4];
}

/* 3. Non-MEM_P target (register) */
NOINLINE static int test_register_target(void) {
    /* These should trigger !MEM_P(target) path */
    int reg1 = src_int[3];      /* Single element to register */
    int reg2 = src_int[4];
    char reg3 = src_char[7];
    
    /* Also test with constant index expressions */
    int reg4 = src_int[1 + 2];  /* Index is constant 3 */
    
    return reg1 + reg2 + reg3 + reg4;
}

/* 4. Struct and union copies */
NOINLINE static void test_struct_copy(void) {
    /* Copy entire struct - constant size, small */
    dst_struct[0] = src_struct[0];
    
    /* Copy 2 structs - count=2 */
    for (int i = 1; i <= 2; ++i) {
        dst_struct[i] = src_struct[i];
    }
    
    /* Copy union */
    dst_union = src_union;
    
    /* Copy struct field by field with constant indices */
    dst_struct[0].a = src_struct[0].a;
    dst_struct[0].b = src_struct[0].b;
    dst_struct[0].c = src_struct[0].c;
}

/* 5. Variable bounds - should NOT trigger const_bounds_p */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - compiler can't determine as constant */
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 10) {
            dst_int[i] = src_int[i];
        }
    }
    
    /* Variable size memcpy */
    int size = end - start + 1;
    if (size > 0 && size <= 10) {
        memcpy(&dst_char[start], &src_char[start], size * sizeof(char));
    }
}

/* 6. Mixed constant/variable patterns */
NOINLINE static void test_mixed_patterns(void) {
    /* Constant lower bound, variable upper bound */
    int j = 3;  /* Not compile-time constant if we hide it */
    volatile int hidden = 3;
    j = hidden;
    
    for (int i = 1; i <= j; ++i) {
        dst_int[i] = src_int[i];
    }
    
    /* Constant slice in the middle */
    for (int i = 5; i <= 7; ++i) {
        dst_char[i] = src_char[i];
    }
}

/* 7. Edge cases with different element types */
NOINLINE static void test_edge_cases(void) {
    /* Copy single element of different types */
    int8_t  b = (int8_t)src_char[0];
    int16_t s = (int16_t)src_int[0];
    int32_t i = src_int[1];
    int64_t l = src_ll[0];
    
    dst_char[15] = b;
    dst_int[6] = s;
    dst_int[7] = i;
    dst_ll[2] = l;
    
    /* Copy with constant index calculation */
    dst_int[2*2] = src_int[2*2];      /* Index 4 */
    dst_int[1+2] = src_int[1+2];      /* Index 3 */
}

/* Main function that calls all tests and returns checksum */
int main(void) {
    int checksum = 0;
    
    test_const_small_memcpy();
    test_const_large_element();
    checksum += test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 4);  /* Variable bounds */
    test_mixed_patterns();
    test_edge_cases();
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 10; i++) {
        checksum += dst_int[i];
    }
    for (int i = 0; i < 20; i++) {
        checksum += dst_char[i];
    }
    for (int i = 0; i < 5; i++) {
        checksum += (int)dst_ll[i];
    }
    for (int i = 0; i < 3; i++) {
        checksum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    checksum += dst_union.i;
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
