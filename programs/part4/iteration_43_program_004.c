/* test_expr_coverage.c - Test cases for expr.cc lines 7691-7700 */

#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } s = {5, 123, 9};
    
    /* This should trigger !MEM_P(target) path */
    unsigned int extracted = s.b;  /* Extract 8-bit field into register */
    return extracted;
}

/* Test 2: MEM target with count = 1 */
static int test_single_element_copy(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Copy single element with constant bounds */
    dst[2] = src[2];  /* lo_index = 2, hi_index = 2, count = 1 */
    
    return dst[2];
}

/* Test 3: MEM target with count = 2 */
static int test_two_element_copy(void) {
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Copy two elements with constant bounds */
    dst[3] = src[3];  /* First element */
    dst[4] = src[4];  /* Second element - count = 2 */
    
    return dst[3] + dst[4];
}

/* Test 4: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[8] = "ABCDEFG";
    char dst[8] = {0};
    
    /* Copy 7 chars - count = 7, TYPE_SIZE = 8 bits, total = 56 bits */
    for (int i = 0; i < 7; i++) {
        dst[i] = src[i];
    }
    
    return (int)dst[3];  /* Return 'D' = 68 */
}

/* Test 5: MEM target with count > 2, using _Bool for small element size */
static int test_bool_array_init(void) {
    _Bool flags[12];
    
    /* Initialize with constant pattern - count = 12, TYPE_SIZE = 8 bits typically */
    for (int i = 0; i < 12; i++) {
        flags[i] = (i % 3 == 0);
    }
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += flags[i];
    }
    return sum;
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
    } p1 = {100, 200}, p2;
    
    /* Structure copy - fixed size, constant bounds */
    p2 = p1;  /* count = 2 elements? Depends on how it's expanded */
    
    return p2.x + p2.y;
}

/* Test 7: Array slice with compile-time constant indices */
static int test_array_slice(void) {
    int arr[20];
    
    /* Initialize with constant indices */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * 2;
    }
    
    /* Access slice with constant bounds */
    int sum = 0;
    const int start = 5;
    const int end = 8;
    for (int i = start; i <= end; i++) {  /* count = 4 */
        sum += arr[i];
    }
    
    return sum;
}

/* Test 8: Using enum for constant bounds */
static int test_enum_bounds(void) {
    enum { FIRST = 3, LAST = 6 };
    long values[10] = {0};
    
    /* Initialize slice using enum constants */
    for (int i = FIRST; i <= LAST; i++) {  /* count = 4 */
        values[i] = i * 10L;
    }
    
    return (int)(values[LAST] - values[FIRST]);
}

/* Test 9: Mixed types with sizeof for bounds */
static int test_mixed_sizes(void) {
    unsigned char data[32];
    
    /* Initialize first 10 bytes - count = 10, TYPE_SIZE = 8 bits */
    for (int i = 0; i < 10; i++) {
        data[i] = (unsigned char)(i + 'A');
    }
    
    /* Initialize next 4 shorts - count = 4, TYPE_SIZE = 16 bits */
    short *shorts = (short *)&data[10];
    for (int i = 0; i < 4; i++) {
        shorts[i] = (short)(100 + i);
    }
    
    return data[5] + shorts[2];
}

/* Test 10: Nested constant loops that might unroll */
static int test_nested_constant_loops(void) {
    int matrix[3][3] = {{0}};
    
    /* Constant loops that compiler might analyze */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * 3 + j;
        }
    }
    
    return matrix[2][2];
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();      /* Test 1 */
    total += test_single_element_copy();      /* Test 2 */
    total += test_two_element_copy();         /* Test 3 */
    total += test_small_char_array();         /* Test 4 */
    total += test_bool_array_init();          /* Test 5 */
    total += test_struct_copy();              /* Test 6 */
    total += test_array_slice();              /* Test 7 */
    total += test_enum_bounds();              /* Test 8 */
    total += test_mixed_sizes();              /* Test 9 */
    total += test_nested_constant_loops();    /* Test 10 */
    
    printf("Total checksum: %d\n", total);
    return 0;
}
