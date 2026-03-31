/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_single_element(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element */
    dst[2] = src[2];  /* lo_index = 2, hi_index = 2, count = 1 */
    
    return dst[2];  /* Should return 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_two_elements(void) {
    short src[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    short dst[10] = {0};
    
    /* Constant bounds: copy two elements */
    dst[3] = src[3];  /* First element */
    dst[4] = src[4];  /* Second element */
    /* Compiler might combine these into a small block operation */
    
    return dst[3] + dst[4];  /* Should return 90 */
}

/* Test 3: MEM target with count > 2 but small total size */
static int test_small_chars(void) {
    char src[15] = "Hello, World!";
    char dst[15] = {0};
    
    /* Constant bounds: copy 13 chars (including null terminator) */
    /* lo_index = 0, hi_index = 12, count = 13 */
    /* TYPE_SIZE(char) = 8 bits, total = 13 * 8 = 104 bits = 13 bytes */
    for (int i = 0; i < 13; i++) {
        dst[i] = src[i];
    }
    
    return (int)dst[7];  /* Should return 'W' = 87 */
}

/* Test 4: Non-MEM target (register operation) */
static int test_bitfield_extract(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } src = {.a = 1, .b = 2, .c = 3, .d = 4};
    
    /* Extract constant-sized bitfield to register (!MEM_P target) */
    unsigned int extracted = src.b;  /* 4-bit field extraction */
    
    return extracted;  /* Should return 2 */
}

/* Test 5: Array initialization with constant small bounds */
static int test_small_array_init(void) {
    /* Initialize first 3 elements of array - count = 3 */
    int arr[10] = {[0] = 100, [1] = 200, [2] = 300};
    
    return arr[0] + arr[1] + arr[2];  /* Should return 600 */
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {'x', 'y', 'z'};
    struct small s2;
    
    /* Constant-sized structure copy (3 bytes) */
    s2 = s1;
    
    return s2.a + s2.b + s2.c;  /* Should return 'x' + 'y' + 'z' = 363 */
}

/* Test 7: Using enum for constant bounds */
static int test_enum_bounds(void) {
    enum { START = 2, END = 5 };
    int src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int dst[10] = {0};
    
    /* Constant bounds from enum */
    for (int i = START; i <= END; i++) {
        dst[i] = src[i];
    }
    
    return dst[2] + dst[5];  /* Should return 7 */
}

/* Test 8: Multi-dimensional array slice */
static int test_2d_slice(void) {
    int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    int row[3] = {0};
    
    /* Copy middle row - constant bounds */
    for (int i = 0; i < 3; i++) {
        row[i] = matrix[1][i];
    }
    
    return row[0] + row[1] + row[2];  /* Should return 15 */
}

/* Test 9: Pointer arithmetic with constant offsets */
static int test_pointer_const_offset(void) {
    int data[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int *p = data;
    
    /* Constant offset operations */
    int a = *(p + 3);  /* offset 3 */
    int b = *(p + 4);  /* offset 4 */
    
    return a + b;  /* Should return 70 */
}

/* Test 10: Mixed types with small total size */
static int test_mixed_small(void) {
    struct tiny {
        _Bool flag;
        char code;
        short value;
    } t1 = {1, 'A', 123};
    struct tiny t2;
    
    /* Total size: 1 + 1 + 2 = 4 bytes (on most systems) */
    t2 = t1;
    
    return t2.flag + t2.code + t2.value;  /* Should return 1 + 65 + 123 = 189 */
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_single_element();      /* +3 */
    total += test_two_elements();        /* +90 */
    total += test_small_chars();         /* +87 */
    total += test_bitfield_extract();    /* +2 */
    total += test_small_array_init();    /* +600 */
    total += test_struct_copy();         /* +363 */
    total += test_enum_bounds();         /* +7 */
    total += test_2d_slice();            /* +15 */
    total += test_pointer_const_offset(); /* +70 */
    total += test_mixed_small();         /* +189 */
    
    /* Expected total: 3+90+87+2+600+363+7+15+70+189 = 1426 */
    printf("Result: %d\n", total);
    
    return 0;
}
