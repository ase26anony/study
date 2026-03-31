/* test_expr_coverage.c - Target GCC expr.cc lines 7691-7700 */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our test cases */
static volatile int sink;

/* Test 1: Small constant memcpy operations */
__attribute__((noinline))
static void test_const_small_memcpy(void) {
    /* char arrays - small element size */
    char src1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    char dst1[8] = {0};
    
    /* Copy 1 element (count = 1) */
    memcpy(&dst1[2], &src1[2], 1 * sizeof(char));
    
    /* Copy 2 elements (count = 2) - hits count <= 2 branch */
    memcpy(&dst1[0], &src1[0], 2 * sizeof(char));
    
    /* Copy 3 chars (3 bytes total) - small total size */
    memcpy(&dst1[3], &src1[3], 3 * sizeof(char));
    
    /* int arrays - larger element size */
    int src2[5] = {10, 20, 30, 40, 50};
    int dst2[5] = {0};
    
    /* Copy 2 ints (count = 2, but larger byte size) */
    memcpy(&dst2[1], &src2[1], 2 * sizeof(int));
    
    sink = dst1[0] + dst2[0];
}

/* Test 2: Register target (non-MEM_P) */
__attribute__((noinline))
static void test_register_target(void) {
    int array[4] = {100, 200, 300, 400};
    
    /* Copy single element to register variable */
    int reg1 = array[2];  /* !MEM_P(target) path */
    
    /* Copy two elements to separate registers */
    int reg2 = array[0];
    int reg3 = array[1];
    
    /* Manual copy of 2 elements - constant bounds */
    int temp[2];
    for (int i = 0; i <= 1; ++i) {  /* lo=0, hi=1, count=2 */
        temp[i] = array[i + 2];
    }
    
    sink = reg1 + reg2 + reg3 + temp[0];
}

/* Test 3: Struct copies with constant sizes */
__attribute__((noinline))
static void test_struct_copy(void) {
    struct SmallStruct {
        char a;
        int b;
        short c;
    };
    
    struct SmallStruct s1 = {65, 1000, 2000};
    struct SmallStruct s2 = {0};
    
    /* Direct struct assignment - should use inline copy */
    s2 = s1;
    
    /* Array of structs with constant bounds copy */
    struct SmallStruct arr1[3] = {{1,2,3}, {4,5,6}, {7,8,9}};
    struct SmallStruct arr2[3] = {0};
    
    /* Copy 2 structs (count = 2) */
    for (int i = 0; i <= 1; ++i) {
        arr2[i] = arr1[i];
    }
    
    /* Copy 1 struct (count = 1) */
    arr2[2] = arr1[2];
    
    sink = s2.b + arr2[0].b;
}

/* Test 4: Union copies */
__attribute__((noinline))
static void test_union_copy(void) {
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed u1 = {.i = 0x12345678};
    union Mixed u2;
    
    /* Union assignment */
    u2 = u1;
    
    /* Array of unions with constant bounds */
    union Mixed uarr1[4] = {{.i=1}, {.i=2}, {.i=3}, {.i=4}};
    union Mixed uarr2[4];
    
    /* Copy 3 unions (count = 3) */
    for (int i = 1; i <= 3; ++i) {  /* lo=1, hi=3, count=3 */
        uarr2[i] = uarr1[i];
    }
    
    sink = u2.i + uarr2[1].i;
}

/* Test 5: Mixed element sizes and counts */
__attribute__((noinline))
static void test_mixed_sizes(void) {
    /* Test with long long - larger element size */
    long long big_src[4] = {1000LL, 2000LL, 3000LL, 4000LL};
    long long big_dst[4] = {0};
    
    /* Copy 3 long longs (24 bytes on 64-bit) - tests size threshold */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        big_dst[i] = big_src[i];
    }
    
    /* Copy 1 long long (count = 1) */
    big_dst[3] = big_src[3];
    
    /* Test with short - smaller element size */
    short small_src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    short small_dst[8] = {0};
    
    /* Copy 4 shorts (8 bytes) */
    for (int i = 2; i <= 5; ++i) {  /* lo=2, hi=5, count=4 */
        small_dst[i] = small_src[i];
    }
    
    sink = (int)big_dst[0] + small_dst[2];
}

/* Test 6: Variable bounds (should NOT hit the uncovered lines) */
__attribute__((noinline))
static void test_variable_bounds(int start, int end) {
    int src[10] = {0,1,2,3,4,5,6,7,8,9};
    int dst[10] = {0};
    
    /* Variable bounds - const_bounds_p should be false */
    for (int i = start; i <= end; ++i) {
        dst[i] = src[i];
    }
    
    sink = dst[0];
}

/* Test 7: Constant bounds with pointer arithmetic */
__attribute__((noinline))
static void test_pointer_arithmetic(void) {
    double data[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double copy[8] = {0};
    
    /* Constant bounds with pointer offset */
    double *src_ptr = &data[2];
    double *dst_ptr = &copy[2];
    
    /* Copy 3 doubles (count = 3) */
    for (int i = 0; i <= 2; ++i) {
        dst_ptr[i] = src_ptr[i];
    }
    
    sink = (int)copy[2];
}

/* Test 8: Nested array copies */
__attribute__((noinline))
static void test_nested_arrays(void) {
    int matrix1[3][3] = {{1,2,3}, {4,5,6}, {7,8,9}};
    int matrix2[3][3] = {0};
    
    /* Copy entire row (3 elements) with constant bounds */
    for (int j = 0; j <= 2; ++j) {  /* lo=0, hi=2, count=3 */
        matrix2[1][j] = matrix1[1][j];
    }
    
    /* Copy 2 elements from a row */
    matrix2[0][0] = matrix1[0][0];
    matrix2[0][1] = matrix1[0][1];  /* Two separate assignments */
    
    sink = matrix2[1][0];
}

int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    test_const_small_memcpy();
    test_register_target();
    test_struct_copy();
    test_union_copy();
    test_mixed_sizes();
    test_variable_bounds(0, 3);  /* Variable bounds */
    test_pointer_arithmetic();
    test_nested_arrays();
    
    /* Use sink to prevent dead code elimination */
    checksum = sink;
    
    return checksum != 0 ? 0 : 1;
}
