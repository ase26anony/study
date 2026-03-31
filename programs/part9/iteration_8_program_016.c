/* test_expr_coverage.c - Target GCC expr.cc lines 7691-7700 */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our test cases */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global arrays to avoid aliasing issues */
static int src_int[10] = {0,1,2,3,4,5,6,7,8,9};
static int dst_int[10] = {0};
static char src_char[20] = "0123456789ABCDEFGHI";
static char dst_char[20] = {0};
static long long src_ll[5] = {100,200,300,400,500};
static long long dst_ll[5] = {0};

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct src_struct[5] = {
    {'a', 1, 10}, {'b', 2, 20}, {'c', 3, 30},
    {'d', 4, 40}, {'e', 5, 50}
};
static struct SmallStruct dst_struct[5] = {0};

/* 1. Constant bounds, count <= 2, MEM_P(target) */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy 1 element - should trigger count <= 2 branch */
    dst_int[0] = src_int[0];
    
    /* Copy 2 elements - should trigger count <= 2 branch */
    dst_int[1] = src_int[1];
    dst_int[2] = src_int[2];
    
    /* Copy 2 chars (2 bytes) - small size */
    dst_char[0] = src_char[0];
    dst_char[1] = src_char[1];
    
    /* memcpy with constant size 2 */
    memcpy(&dst_char[2], &src_char[2], 2);
}

/* 2. Constant bounds, count > 2, small total size */
NOINLINE static void test_const_small_aggregate(void) {
    /* Copy 3 chars (3 bytes) - TYPE_SIZE * count is small */
    for (int i = 3; i <= 5; ++i) {  /* constant bounds: i=3 to i=5 */
        dst_char[i] = src_char[i];
    }
    
    /* memcpy 3 bytes */
    memcpy(&dst_char[6], &src_char[6], 3);
    
    /* Copy 4 chars (4 bytes) */
    memcpy(&dst_char[9], &src_char[9], 4);
}

/* 3. Constant bounds, count > 2, larger total size */
NOINLINE static void test_const_large_elements(void) {
    /* Copy 3 long longs (24 bytes on 64-bit) - larger than threshold */
    for (int i = 0; i <= 2; ++i) {  /* constant bounds: i=0 to i=2 */
        dst_ll[i] = src_ll[i];
    }
    
    /* memcpy 24 bytes */
    memcpy(&dst_ll[2], &src_ll[2], 3 * sizeof(long long));
}

/* 4. Non-MEM_P target (register) */
NOINLINE static void test_register_target(void) {
    /* Single element copy to register */
    int reg1 = src_int[3];  /* !MEM_P(target) */
    
    /* Two elements to separate registers */
    int reg2 = src_int[4];
    int reg3 = src_int[5];
    
    /* Use the registers to prevent elimination */
    dst_int[3] = reg1;
    dst_int[4] = reg2;
    dst_int[5] = reg3;
    
    /* Struct element to register */
    char reg_char = src_struct[0].a;
    dst_struct[0].a = reg_char;
}

/* 5. Struct copies - aggregate types */
NOINLINE static void test_struct_copy(void) {
    /* Copy entire struct (constant size) */
    dst_struct[0] = src_struct[0];
    
    /* Copy 2 structs */
    dst_struct[1] = src_struct[1];
    dst_struct[2] = src_struct[2];
    
    /* Copy struct array slice (3 elements) */
    for (int i = 2; i <= 4; ++i) {  /* constant bounds */
        dst_struct[i] = src_struct[i];
    }
    
    /* memcpy with struct */
    memcpy(&dst_struct[3], &src_struct[3], 2 * sizeof(struct SmallStruct));
}

/* 6. Union for additional coverage */
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion src_union = {.i = 0x12345678};
static union TestUnion dst_union;

NOINLINE static void test_union_copy(void) {
    /* Union copy - constant size */
    dst_union = src_union;
    
    /* Copy union array */
    union TestUnion unions[3];
    unions[0] = src_union;
    unions[1] = src_union;
    unions[2] = src_union;
    
    /* Use unions */
    dst_union.i = unions[0].i;
}

/* 7. Variable bounds - should NOT trigger the uncovered lines */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - const_bounds_p should be false */
    for (int i = start; i <= end; ++i) {
        dst_int[i] = src_int[i];
    }
    
    /* Variable size memcpy */
    int count = end - start + 1;
    if (count > 0 && count < 10) {
        memcpy(&dst_char[10], &src_char[10], count);
    }
}

/* 8. Mixed constant/variable cases */
NOINLINE static void test_mixed_bounds(int n) {
    /* Mixed: constant lower bound, variable upper bound */
    for (int i = 0; i <= n; ++i) {
        dst_int[i] = src_int[i];
    }
    
    /* Mixed: variable lower bound, constant upper bound */
    for (int i = n; i <= 4; ++i) {
        dst_ll[i] = src_ll[i];
    }
}

/* Checksum to prevent dead code elimination */
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        sum += dst_int[i];
        if (i < 5) {
            sum += dst_ll[i];
            sum += dst_struct[i].b;
        }
        if (i < 20) {
            sum += dst_char[i];
        }
    }
    
    sum += dst_union.i;
    return sum;
}

/* Main function that calls all test cases */
int main(void) {
    /* Clear destination arrays */
    memset(dst_int, 0, sizeof(dst_int));
    memset(dst_char, 0, sizeof(dst_char));
    memset(dst_ll, 0, sizeof(dst_ll));
    memset(dst_struct, 0, sizeof(dst_struct));
    
    /* Execute all test cases */
    test_const_small_memcpy();        /* Lines 7691-7700: count <= 2 */
    test_const_small_aggregate();     /* Lines 7691-7700: small TYPE_SIZE * count */
    test_const_large_elements();      /* Lines 7691-7700: larger TYPE_SIZE * count */
    test_register_target();           /* Lines 7691-7700: !MEM_P(target) */
    test_struct_copy();               /* Lines 7691-7700: aggregate types */
    test_union_copy();                /* Additional coverage */
    
    /* These should NOT hit the uncovered lines */
    test_variable_bounds(1, 3);       /* Variable bounds */
    test_mixed_bounds(2);             /* Mixed bounds */
    
    /* Compute and return checksum */
    return compute_checksum() % 256;
}
