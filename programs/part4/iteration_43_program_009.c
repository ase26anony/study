/* test_expr_coverage.c - Test cases for GCC expr.cc constant bounds analysis */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
/* This should trigger the !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } s = {5, 123, 9};
    
    /* Extract bitfield into register (non-MEM target) */
    unsigned int extracted = s.b;
    
    /* Use the result to prevent optimization */
    return extracted * 2;
}

/* Test 2: MEM target with count = 1 */
/* Single element array initialization */
static int test_single_element_copy(void) {
    int src[3] = {10, 20, 30};
    int dst[3] = {0};
    
    /* Constant bounds: copy exactly one element */
    dst[1] = src[1];  /* lo_index = 1, hi_index = 1, count = 1 */
    
    return dst[0] + dst[1] + dst[2];
}

/* Test 3: MEM target with count = 2 */
/* Two element array initialization */
static int test_two_element_copy(void) {
    short src[5] = {1, 2, 3, 4, 5};
    short dst[5] = {0};
    
    /* Constant bounds: copy two elements */
    dst[0] = src[0];  /* Could be expanded as two scalar stores */
    dst[1] = src[1];  /* lo_index = 0, hi_index = 1, count = 2 */
    
    return dst[0] + dst[1] + dst[2] + dst[3] + dst[4];
}

/* Test 4: MEM target with count > 2 but small total size */
/* char array with 10 elements - TYPE_SIZE = 8 bits, count = 10, total = 80 bits */
static int test_small_char_array(void) {
    char src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char dst[10] = {0};
    
    /* This should trigger the TYPE_SIZE * count calculation */
    for (int i = 0; i < 10; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 5: MEM target with _Bool array */
/* _Bool has implementation-defined size but often 8 bits */
static int test_bool_array(void) {
    _Bool src[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    _Bool dst[8] = {0};
    
    /* Copy all 8 elements - small total size */
    for (int i = 0; i < 8; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 6: Structure copy with constant size */
/* Small structure copy that might use scalar expansion */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
    };
    
    struct Point p1 = {100, 200};
    struct Point p2;
    
    /* Structure copy - constant size of 4 bytes (2 shorts) */
    p2 = p1;
    
    return p2.x + p2.y;
}

/* Test 7: Array slice with constant bounds using compound literal */
static int test_array_slice(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Copy slice with constant bounds */
    int slice[3];
    slice[0] = arr[2];  /* lo_index = 2, hi_index = 4, count = 3 */
    slice[1] = arr[3];
    slice[2] = arr[4];
    
    return slice[0] + slice[1] + slice[2];
}

/* Test 8: Using enum for constant bounds */
static int test_enum_bounds(void) {
    enum { START = 3, END = 5 };
    
    long src[10] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    long dst[10] = {0};
    
    /* Constant bounds from enum */
    dst[START] = src[START];
    dst[START + 1] = src[START + 1];
    dst[END] = src[END];
    
    return (int)(dst[3] + dst[4] + dst[5]) / 100;
}

/* Test 9: Mixed types in small array */
static int test_mixed_small_copy(void) {
    /* Array of 16-bit types */
    uint16_t src[4] = {0x1234, 0x5678, 0x9ABC, 0xDEF0};
    uint16_t dst[4] = {0};
    
    /* Copy 3 elements (count > 2, but total size = 6 bytes) */
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    
    return dst[0] + dst[1] + dst[2] + dst[3];
}

/* Test 10: Nested constant bounds in loop */
static int test_nested_constant_loop(void) {
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int copy[2][3] = {{0}};
    
    /* Loop with constant bounds that might be unrolled */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            copy[i][j] = matrix[i][j];
        }
    }
    
    return copy[0][0] + copy[1][2];
}

/* Test 11: Pointer arithmetic with constant offsets */
static int test_pointer_constant_offset(void) {
    int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int *p = data;
    
    /* Constant offset operations */
    int a = *(p + 2);  /* Equivalent to data[2] */
    int b = *(p + 3);  /* Equivalent to data[3] */
    
    return a + b;
}

/* Test 12: Using sizeof for constant bounds */
static int test_sizeof_bounds(void) {
    char buffer[32] = "Hello, World!";
    char copy[32] = {0};
    
    /* Copy using sizeof for constant size */
    for (size_t i = 0; i < sizeof(buffer[0]) * 5; i++) {  /* Copy first 5 chars */
        copy[i] = buffer[i];
    }
    
    int sum = 0;
    for (size_t i = 0; i < 5; i++) {
        sum += copy[i];
    }
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Run all test cases and accumulate results */
    total += test_bitfield_extraction();      /* Test 1 */
    total += test_single_element_copy();      /* Test 2 */
    total += test_two_element_copy();         /* Test 3 */
    total += test_small_char_array();         /* Test 4 */
    total += test_bool_array();               /* Test 5 */
    total += test_struct_copy();              /* Test 6 */
    total += test_array_slice();              /* Test 7 */
    total += test_enum_bounds();              /* Test 8 */
    total += test_mixed_small_copy();         /* Test 9 */
    total += test_nested_constant_loop();     /* Test 10 */
    total += test_pointer_constant_offset();  /* Test 11 */
    total += test_sizeof_bounds();            /* Test 12 */
    
    printf("Total checksum: %d\n", total);
    
    /* Expected total (for verification):
       Test 1: 123 * 2 = 246
       Test 2: 0 + 20 + 0 = 20
       Test 3: 1 + 2 + 0 + 0 + 0 = 3
       Test 4: 1+2+3+4+5+6+7+8+9+10 = 55
       Test 5: 1+0+1+0+1+0+1+0 = 4
       Test 6: 100 + 200 = 300
       Test 7: 2 + 3 + 4 = 9
       Test 8: (400 + 500 + 600) / 100 = 15
       Test 9: 0x1234 + 0x5678 + 0x9ABC + 0 = 0x10468 = 66664
       Test 10: 1 + 6 = 7
       Test 11: 3 + 4 = 7
       Test 12: 'H'+'e'+'l'+'l'+'o' = 72+101+108+108+111 = 500
       Total: 246+20+3+55+4+300+9+15+66664+7+7+500 = 67830
    */
    
    return 0;
}
