/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
static int test_bitfield_extraction(void) {
    /* This should trigger !MEM_P(target) path */
    struct S {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } s = {0x5, 0xAB, 0x3};
    
    /* Extract constant-sized bit-field into register */
    unsigned int extracted = s.b;  /* 8-bit field, constant bounds */
    
    /* Use the result to prevent optimization */
    return extracted == 0xAB ? 1 : 0;
}

/* Test 2: MEM target with count = 1 */
static int test_single_element_copy(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Copy single element with constant bounds */
    dst[2] = src[2];  /* lo_index = 2, hi_index = 2, count = 1 */
    
    /* Verify and return result */
    return dst[2] == 3 ? 2 : 0;
}

/* Test 3: MEM target with count = 2 */
static int test_two_element_copy(void) {
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Copy two elements with constant bounds */
    dst[3] = src[3];  /* lo_index = 3, hi_index = 4, count = 2 */
    dst[4] = src[4];
    
    /* Verify */
    return (dst[3] == 3 && dst[4] == 4) ? 3 : 0;
}

/* Test 4: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[10] = "abcdefghi";
    char dst[10] = {0};
    
    /* Copy 5 chars - total size = 5 bytes, should trigger TYPE_SIZE * count path */
    for (int i = 2; i < 7; i++) {  /* lo_index = 2, hi_index = 6, count = 5 */
        dst[i] = src[i];
    }
    
    /* Verify by checksum */
    int sum = 0;
    for (int i = 2; i < 7; i++) {
        sum += dst[i];
    }
    return sum == ('c' + 'd' + 'e' + 'f' + 'g') ? 4 : 0;
}

/* Test 5: MEM target with _Bool array (1-bit elements, but usually byte-sized) */
static int test_bool_array_init(void) {
    _Bool arr[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    
    /* Initialize with constant pattern - all elements have constant bounds */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += arr[i];
    }
    
    return sum == 4 ? 5 : 0;
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        int x;
        int y;
    } p1 = {10, 20}, p2;
    
    /* Copy entire struct - constant size (2 ints = 8 bytes typically) */
    p2 = p1;
    
    return (p2.x == 10 && p2.y == 20) ? 6 : 0;
}

/* Test 7: Array slice in loop with constant iteration count */
static int test_unrolled_loop(void) {
    int src[4] = {100, 200, 300, 400};
    int dst[4] = {0};
    
    /* This loop should unroll at -O2/-O3, creating constant-bounded copies */
    for (int i = 0; i < 3; i++) {  /* Constant iteration count = 3 */
        dst[i] = src[i];
    }
    
    return (dst[0] == 100 && dst[1] == 200 && dst[2] == 300) ? 7 : 0;
}

/* Test 8: Using compile-time constants via enum */
static int test_enum_bounds(void) {
    enum { START = 1, END = 3 };
    int src[5] = {0, 10, 20, 30, 40};
    int dst[5] = {0};
    
    /* Constant bounds via enum */
    for (int i = START; i <= END; i++) {
        dst[i] = src[i];
    }
    
    return (dst[1] == 10 && dst[2] == 20 && dst[3] == 30) ? 8 : 0;
}

/* Test 9: Mixed types to test different TYPE_SIZE calculations */
static int test_mixed_types(void) {
    struct Mixed {
        char c;
        short s;
        int i;
    } m1 = {'A', 123, 456}, m2;
    
    /* Copy mixed-size structure */
    m2 = m1;
    
    return (m2.c == 'A' && m2.s == 123 && m2.i == 456) ? 9 : 0;
}

/* Test 10: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int *ptr = array;
    
    /* Access with constant offset */
    int a = ptr[2];  /* Constant index 2 */
    int b = ptr[3];  /* Constant index 3 */
    
    return (a == 2 && b == 3) ? 10 : 0;
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();      /* Should trigger !MEM_P(target) */
    total += test_single_element_copy();      /* count = 1, MEM target */
    total += test_two_element_copy();         /* count = 2, MEM target */
    total += test_small_char_array();         /* count = 5, small TYPE_SIZE */
    total += test_bool_array_init();          /* _Bool array */
    total += test_struct_copy();              /* Constant-sized struct */
    total += test_unrolled_loop();            /* Unrolled loop */
    total += test_enum_bounds();              /* Enum-based bounds */
    total += test_mixed_types();              /* Mixed type sizes */
    total += test_pointer_arithmetic();       /* Pointer with constant offset */
    
    printf("Result: %d\n", total);
    
    /* Expected result if all tests pass: 1+2+3+4+5+6+7+8+9+10 = 55 */
    return total == 55 ? 0 : 1;
}
