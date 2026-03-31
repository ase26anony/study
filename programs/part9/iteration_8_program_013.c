/* Test program for GCC expr.cc array copy expansion logic */
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

static struct SmallStruct src_struct[5] = {
    {'a', 1, 10}, {'b', 2, 20}, {'c', 3, 30}, {'d', 4, 40}, {'e', 5, 50}
};
static struct SmallStruct dst_struct[5] = {0};

/* Union for testing different type sizes */
union MixedUnion {
    char c[8];
    int i[2];
    long long ll;
};

static union MixedUnion src_union = {.ll = 0x0123456789ABCDEFULL};
static union MixedUnion dst_union = {0};

/* Test 1: Constant small memcpy (count <= 2 or small byte size) */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy 1 element - should trigger count <= 2 path */
    dst_int[0] = src_int[0];
    
    /* Copy 2 elements - should trigger count <= 2 path */
    dst_int[1] = src_int[1];
    dst_int[2] = src_int[2];
    
    /* Copy 3 chars (3 bytes total) - small byte size path */
    for (int i = 3; i <= 5; ++i) {  /* Constant bounds: i=3 to i=5 */
        dst_char[i] = src_char[i];
    }
    
    /* memcpy with constant size 2 (fits count <= 2) */
    memcpy(&dst_char[10], &src_char[10], 2);
    
    /* memcpy with constant size 3 (small byte size for chars) */
    memcpy(&dst_char[12], &src_char[12], 3);
}

/* Test 2: Constant bounds but larger byte size */
NOINLINE static void test_const_large_memcpy(void) {
    /* Copy 3 long long elements (24 bytes on 64-bit) - larger than threshold */
    for (int i = 0; i <= 2; ++i) {  /* Constant bounds: i=0 to i=2 */
        dst_ll[i] = src_ll[i];
    }
    
    /* memcpy with 24 bytes (likely above inline threshold) */
    memcpy(&dst_ll[2], &src_ll[2], 3 * sizeof(long long));
}

/* Test 3: Register target (!MEM_P(target)) */
NOINLINE static void test_register_target(void) {
    /* Copy single element to register variable */
    int reg_temp1 = src_int[3];  /* Should trigger !MEM_P(target) path */
    dst_int[3] = reg_temp1;
    
    long long reg_temp2 = src_ll[1];
    dst_ll[1] = reg_temp2;
    
    char reg_temp3 = src_char[7];
    dst_char[7] = reg_temp3;
}

/* Test 4: Struct and union copies */
NOINLINE static void test_struct_copy(void) {
    /* Copy single struct (aggregate type) */
    dst_struct[0] = src_struct[0];  /* Should use RECORD_TYPE elttype */
    
    /* Copy 2 struct elements */
    for (int i = 1; i <= 2; ++i) {  /* Constant bounds */
        dst_struct[i] = src_struct[i];
    }
    
    /* Copy union */
    dst_union = src_union;
    
    /* Copy 3 struct elements (check byte size calculation) */
    for (int i = 2; i <= 4; ++i) {  /* Constant bounds: i=2 to i=4 */
        dst_struct[i] = src_struct[i];
    }
}

/* Test 5: Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - const_bounds_p should be false */
    for (int i = start; i <= end; ++i) {
        dst_int[i] = src_int[i];
    }
    
    /* Variable size memcpy */
    int count = end - start + 1;
    if (count > 0 && count < 10) {
        memcpy(&dst_char[start], &src_char[start], count);
    }
}

/* Test 6: Mixed scenarios with different element types */
NOINLINE static void test_mixed_scenarios(void) {
    /* Exactly 2 elements of any type */
    dst_int[5] = src_int[5];
    dst_int[6] = src_int[6];
    
    /* 3 chars (small byte size) */
    dst_char[15] = src_char[15];
    dst_char[16] = src_char[16];
    dst_char[17] = src_char[17];
    
    /* Array slice with constant bounds but different element sizes */
    short src_short[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    short dst_short[10] = {0};
    
    /* Copy 4 shorts (8 bytes on most systems) */
    for (int i = 2; i <= 5; ++i) {  /* Constant bounds */
        dst_short[i] = src_short[i];
    }
}

/* Main function that calls all tests and returns checksum */
int main(void) {
    int checksum = 0;
    
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 4);  /* Variable bounds */
    test_mixed_scenarios();
    
    /* Calculate checksum to prevent dead code elimination */
    for (int i = 0; i < 10; i++) {
        checksum += dst_int[i];
    }
    for (int i = 0; i < 20; i++) {
        checksum += dst_char[i];
    }
    for (int i = 0; i < 5; i++) {
        checksum += (int)dst_ll[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
