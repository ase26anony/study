/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } s = {0x5, 0xAB, 0x3};
    
    /* Extract constant-sized bit-field into register (non-MEM target) */
    unsigned int extracted = s.b;  /* 8-bit field extraction */
    
    /* Use in computation to prevent optimization */
    return extracted * 2;
}

/* Test 2: MEM target with count = 1 */
static int test_single_element_copy(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5];
    
    /* Copy single element with constant bounds */
    dst[2] = src[2];  /* lo_index = 2, hi_index = 2, count = 1 */
    
    return dst[2];
}

/* Test 3: MEM target with count = 2 */
static int test_two_element_copy(void) {
    short src[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    short dst[10];
    
    /* Copy two elements with constant bounds */
    dst[3] = src[3];  /* First element */
    dst[4] = src[4];  /* Second element - compiler might combine */
    
    return dst[3] + dst[4];
}

/* Test 4: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[8] = "ABCDEFG";
    char dst[8];
    
    /* Copy 7 chars (count = 7, element_size = 1, total = 7 bytes) */
    for (int i = 0; i < 7; i++) {
        dst[i] = src[i];
    }
    dst[7] = '\0';
    
    return (int)dst[0] + (int)dst[3] + (int)dst[6];
}

/* Test 5: MEM target with small struct array */
static int test_small_struct_array(void) {
    struct Point {
        short x;
        short y;
    };
    
    struct Point src[3] = {{1,2}, {3,4}, {5,6}};
    struct Point dst[3];
    
    /* Copy 3 structs (count = 3, element_size = 4, total = 12 bytes) */
    for (int i = 0; i < 3; i++) {
        dst[i] = src[i];
    }
    
    return dst[0].x + dst[1].y + dst[2].x;
}

/* Test 6: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Initialize array with constant bounds */
    int arr[4] = {10, 20, 30, 40};
    
    return arr[0] + arr[3];
}

/* Test 7: Constant loop with small iteration count */
static int test_constant_loop(void) {
    int values[5];
    int sum = 0;
    
    /* Loop with constant bounds (0 to 4 inclusive) */
    for (int i = 0; i < 5; i++) {
        values[i] = i * 10;
    }
    
    for (int i = 0; i < 5; i++) {
        sum += values[i];
    }
    
    return sum;
}

/* Test 8: Memory operation with sizeof-based bounds */
static int test_sizeof_bounds(void) {
    char buffer1[16];
    char buffer2[16];
    
    /* Use sizeof for constant bounds */
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = (char)(i + 'A');
    }
    
    /* Copy with constant size */
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        buffer2[i] = buffer1[i];
    }
    
    return (int)buffer2[0] + (int)buffer2[15];
}

/* Test 9: Mixed types with small total size */
static int test_mixed_small_types(void) {
    _Bool flags[10];  /* Small element type */
    int result = 0;
    
    /* Initialize array of _Bool (1 byte each) */
    for (int i = 0; i < 10; i++) {
        flags[i] = (i % 2 == 0);
    }
    
    for (int i = 0; i < 10; i++) {
        result += flags[i];
    }
    
    return result;
}

/* Test 10: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int array[10] = {0,1,2,3,4,5,6,7,8,9};
    int *p1 = &array[2];
    int *p2 = &array[5];
    
    /* Copy with constant offset difference */
    int temp[3];
    for (int i = 0; i < 3; i++) {
        temp[i] = p1[i];
    }
    
    return temp[0] + temp[2];
}

int main(void) {
    int total = 0;
    
    /* Run all test cases and accumulate results */
    total += test_bitfield_extraction();      /* Should trigger !MEM_P(target) path */
    total += test_single_element_copy();      /* MEM target, count = 1 */
    total += test_two_element_copy();         /* MEM target, count = 2 */
    total += test_small_char_array();         /* MEM target, count=7, size=7 */
    total += test_small_struct_array();       /* MEM target, count=3, size=12 */
    total += test_compound_literal();         /* Array initialization */
    total += test_constant_loop();            /* Constant-bounded loop */
    total += test_sizeof_bounds();            /* sizeof-based bounds */
    total += test_mixed_small_types();        /* _Bool array */
    total += test_pointer_arithmetic();       /* Pointer arithmetic */
    
    printf("Result: %d\n", total);
    return 0;
}
