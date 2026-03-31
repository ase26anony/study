/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int sink;

/* Test 1: Non-MEM target - bit-field extraction into register */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } s = {2, 7, 42};
    
    /* Extract constant-sized bit-field - should trigger !MEM_P(target) path */
    unsigned int extracted = s.b;  /* 5-bit field, constant bounds */
    
    /* Use the result */
    return extracted * 10;
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
    dst[0] = src[0];  /* First element */
    dst[1] = src[1];  /* Second element - count = 2 */
    
    return dst[0] + dst[1];
}

/* Test 4: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[8] = "ABCDEFG";
    char dst[8] = {0};
    
    /* Copy 7 chars - count = 7, TYPE_SIZE = 8 bits, total = 56 bits */
    for (int i = 0; i < 7; i++) {
        dst[i] = src[i];
    }
    
    return dst[3];  /* Return 'D' = 68 */
}

/* Test 5: MEM target with count > 2, using const indices */
static int test_const_bounded_copy(void) {
    const int LO = 1;
    const int HI = 3;
    int src[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int dst[10] = {0};
    
    /* Copy elements 1 through 3 inclusive: count = 3 */
    for (int i = LO; i <= HI; i++) {
        dst[i] = src[i];
    }
    
    return dst[1] + dst[2] + dst[3];  /* 20 + 30 + 40 = 90 */
}

/* Test 6: Structure copy with small size */
static int test_small_struct_copy(void) {
    struct Point {
        char x;
        char y;
        char z;  /* 3 bytes total */
    } p1 = {1, 2, 3}, p2;
    
    /* Structure copy - constant size, count = 3 elements of char */
    p2 = p1;
    
    return p2.x + p2.y + p2.z;  /* 1 + 2 + 3 = 6 */
}

/* Test 7: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Initialize array with constant bounds */
    int arr[4] = { [0] = 100, [2] = 200, [3] = 300 };
    /* Elements: arr[0]=100, arr[1]=0, arr[2]=200, arr[3]=300 */
    
    return arr[0] + arr[2] + arr[3];  /* 100 + 200 + 300 = 600 */
}

/* Test 8: Memory operation with sizeof-based bounds */
static int test_sizeof_bounds(void) {
    unsigned char buffer[16];
    unsigned char pattern[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    
    /* Copy pattern using sizeof - constant bounds */
    for (size_t i = 0; i < sizeof(pattern); i++) {
        buffer[i] = pattern[i];  /* count = 4, TYPE_SIZE = 8 bits */
    }
    
    return buffer[0] + buffer[3];  /* 0xAA + 0xDD = 0x187 = 391 */
}

/* Test 9: Enum-based constant bounds */
static int test_enum_bounds(void) {
    enum { START = 2, END = 4 };
    float values[10] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.0f};
    float copy[10] = {0};
    
    /* Copy using enum constants: count = 3 */
    for (int i = START; i <= END; i++) {
        copy[i] = values[i];
    }
    
    /* Convert to int for checksum */
    return (int)(copy[2] + copy[3] + copy[4]);  /* 3.3 + 4.4 + 5.5 = 13.2 */
}

/* Test 10: Mixed types with small total size */
static int test_mixed_small_types(void) {
    struct Small {
        _Bool flag;
        unsigned char byte;
        signed char sbyte;
    } s1 = {1, 0xFF, -1}, s2;
    
    /* Structure copy: 3 elements, total size small */
    s2 = s1;
    
    return s2.flag + s2.byte + (int)s2.sbyte;  /* 1 + 255 + (-1) = 255 */
}

int main(void) {
    int total = 0;
    
    /* Run all test cases */
    total += test_bitfield_extraction();      /* Expected: 7 * 10 = 70 */
    total += test_single_element_copy();      /* Expected: 3 */
    total += test_two_element_copy();         /* Expected: 0 + 1 = 1 */
    total += test_small_char_array();         /* Expected: 'D' = 68 */
    total += test_const_bounded_copy();       /* Expected: 20+30+40 = 90 */
    total += test_small_struct_copy();        /* Expected: 1+2+3 = 6 */
    total += test_compound_literal();         /* Expected: 100+200+300 = 600 */
    total += test_sizeof_bounds();            /* Expected: 0xAA + 0xDD = 391 */
    total += test_enum_bounds();              /* Expected: 13 (truncated from 13.2) */
    total += test_mixed_small_types();        /* Expected: 1+255-1 = 255 */
    
    /* Add sink to prevent dead code elimination */
    sink = total;
    
    printf("Result: %d\n", total);
    
    /* Expected total: 70+3+1+68+90+6+600+391+13+255 = 1497 */
    return 0;
}
