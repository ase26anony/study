/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int sink;

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_mem_count_1(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element */
    dst[2] = src[2];  /* lo_index = 2, hi_index = 2, count = 1 */
    
    return dst[2];
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_mem_count_2(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};
    
    /* Constant bounds: copy two elements */
    dst[0] = src[0];  /* First element */
    dst[1] = src[1];  /* Second element - compiler may combine as block */
    
    return dst[0] + dst[1];
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_mem_small_total_size(void) {
    char src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char dst[10] = {0};
    
    /* Constant bounds: copy all 10 chars */
    for (int i = 0; i < 10; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 4: MEM target with short array (small element size) */
static int test_mem_short_array(void) {
    short src[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    short dst[8] = {0};
    
    /* Constant bounds through loop unrolling */
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
    
    return dst[0] + dst[7];
}

/* Test 5: Non-MEM target (register operation) - bitfield extraction */
static int test_non_mem_target(void) {
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } bits = {.a = 5, .b = 10, .c = 15, .d = 0};
    
    /* Extracting bitfields into registers (non-MEM target) */
    unsigned int extracted = bits.a + bits.b + bits.c;
    
    return extracted;
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {'x', 'y', 'z'}, s2;
    
    /* Constant-sized structure copy */
    s2 = s1;
    
    return s2.a + s2.b + s2.c;
}

/* Test 7: Array slice with constant bounds */
static int test_array_slice(void) {
    int arr[20];
    
    /* Initialize array */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * 2;
    }
    
    /* Constant slice: elements 5 through 8 (count = 4) */
    int slice[4];
    slice[0] = arr[5];
    slice[1] = arr[6];
    slice[2] = arr[7];
    slice[3] = arr[8];
    
    return slice[0] + slice[3];
}

/* Test 8: Boolean array (very small element size) */
static int test_bool_array(void) {
    _Bool flags[16] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
    _Bool copy[16];
    
    /* Copy all elements - total size is small */
    for (int i = 0; i < 16; i++) {
        copy[i] = flags[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += copy[i];
    }
    return sum;
}

/* Test 9: Mixed operations to trigger different paths */
static int test_mixed(void) {
    /* Create various constant-bounded operations */
    int result = 0;
    
    /* Single element copy */
    int a[3] = {100, 200, 300};
    int b[3];
    b[1] = a[1];  /* count = 1 */
    result += b[1];
    
    /* Two element copy */
    char c[4] = "abcd";
    char d[4];
    d[0] = c[0];
    d[1] = c[1];  /* count = 2 when considered together */
    result += d[0] + d[1];
    
    return result;
}

/* Test 10: Using compile-time constants via enums */
static int test_enum_bounds(void) {
    enum { START = 3, END = 7, COUNT = END - START + 1 };
    
    int values[10] = {0,1,2,3,4,5,6,7,8,9};
    int subset[COUNT];
    
    /* Constant bounds from enum */
    for (int i = START; i <= END; i++) {
        subset[i - START] = values[i];
    }
    
    return subset[0] + subset[COUNT-1];
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_mem_count_1();        /* Should trigger count=1 path */
    total += test_mem_count_2();        /* Should trigger count=2 path */
    total += test_mem_small_total_size(); /* char array with count=10 */
    total += test_mem_short_array();    /* short array with count=8 */
    total += test_non_mem_target();     /* Non-MEM target (register) */
    total += test_struct_copy();        /* Structure copy */
    total += test_array_slice();        /* Array slice */
    total += test_bool_array();         /* Boolean array */
    total += test_mixed();              /* Mixed operations */
    total += test_enum_bounds();        /* Enum-based bounds */
    
    /* Use sink to prevent dead code elimination */
    sink = total;
    
    printf("Result: %d\n", total);
    return 0;
}
