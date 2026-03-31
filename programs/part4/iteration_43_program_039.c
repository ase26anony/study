/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_single_element_copy(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element at compile-time known index */
    const int lo = 2;
    const int hi = 2;  /* count = 1 */
    dst[lo] = src[hi];
    
    return dst[2];  /* Should be 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_two_element_copy(void) {
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Constant bounds: copy two elements */
    const int lo = 3;
    const int hi = 4;  /* count = 2 */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[3] + dst[4];  /* Should be 3 + 4 = 7 */
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[20];
    char dst[20];
    
    /* Initialize source with pattern */
    for (int i = 0; i < 20; i++) {
        src[i] = (char)(i + 'A');
    }
    
    /* Constant bounds: copy 10 chars (10 bytes total) */
    const int lo = 5;
    const int hi = 14;  /* count = 10 */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = lo; i <= hi; i++) {
        sum += dst[i];
    }
    return sum;  /* Sum of ASCII values F-O */
}

/* Test 4: MEM target with bool array (small element size) */
static int test_bool_array_init(void) {
    _Bool arr1[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    _Bool arr2[8] = {0};
    
    /* Constant bounds: copy 6 bools */
    const int start = 1;
    const int end = 6;  /* count = 6 */
    for (int i = start; i <= end; i++) {
        arr2[i] = arr1[i];
    }
    
    int count = 0;
    for (int i = 0; i < 8; i++) {
        count += arr2[i];
    }
    return count;  /* Should be 3 */
}

/* Test 5: Non-MEM target (register operation) - bitfield extraction */
static int test_bitfield_extract(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } src = {.a = 5, .b = 10, .c = 15, .d = 0};
    
    /* Extract constant-sized bitfield into register (!MEM_P target) */
    unsigned int value = 0;
    
    /* This should trigger !MEM_P(target) path when extracting b and c */
    const int field1 = 1;  /* b field */
    const int field2 = 2;  /* c field */
    
    /* Simulate extraction of two adjacent 4-bit fields */
    if (field1 == 1) value |= src.b;
    if (field2 == 2) value |= (src.c << 4);
    
    return value;  /* Should be 10 + (15 << 4) = 250 */
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {'X', 'Y', 'Z'};
    struct small s2;
    
    /* Constant bounds copy of entire struct (count = 3 chars) */
    const int lo = 0;
    const int hi = sizeof(struct small) - 1;  /* count = 3 */
    
    char *p1 = (char*)&s1;
    char *p2 = (char*)&s2;
    for (int i = lo; i <= hi; i++) {
        p2[i] = p1[i];
    }
    
    return s2.a + s2.b + s2.c;  /* Sum of 'X' + 'Y' + 'Z' */
}

/* Test 7: Array slice in loop with constant iteration count */
static int test_constant_loop_unroll(void) {
    int data[100];
    int result[100] = {0};
    
    /* Initialize source */
    for (int i = 0; i < 100; i++) {
        data[i] = i * 2;
    }
    
    /* Constant-bounded operation that might be unrolled */
    const int iterations = 3;  /* Small constant loop */
    for (int j = 0; j < iterations; j++) {
        const int idx = j * 10;
        const int lo = idx;
        const int hi = idx + 2;  /* count = 3 each iteration */
        
        for (int i = lo; i <= hi; i++) {
            result[i] = data[i];
        }
    }
    
    return result[0] + result[12] + result[22];  /* 0 + 24 + 44 = 68 */
}

/* Test 8: Mixed types with constant indices */
static int test_mixed_constant_access(void) {
    typedef struct {
        int x;
        short y;
        char z;
    } mixed_t;
    
    mixed_t arr[4] = {
        {1, 100, 'A'},
        {2, 200, 'B'},
        {3, 300, 'C'},
        {4, 400, 'D'}
    };
    
    /* Access constant indices with different element types */
    int sum = 0;
    const int idx1 = 0;
    const int idx2 = 2;
    
    /* These should create constant-bounded memory operations */
    sum += arr[idx1].x;
    sum += arr[idx2].y;
    sum += arr[1].z;  /* Another constant index */
    
    return sum;  /* 1 + 300 + 'B' = 1 + 300 + 66 = 367 */
}

int main(void) {
    int total = 0;
    
    /* Run all test cases */
    total += test_single_element_copy();      /* +3 */
    total += test_two_element_copy();         /* +7 */
    total += test_small_char_array();         /* + ASCII sum F-O */
    total += test_bool_array_init();          /* +3 */
    total += test_bitfield_extract();         /* +250 */
    total += test_struct_copy();              /* + ASCII sum X+Y+Z */
    total += test_constant_loop_unroll();     /* +68 */
    total += test_mixed_constant_access();    /* +367 */
    
    /* Calculate expected total for verification */
    /* test_small_char_array: F-O = 70+71+72+73+74+75+76+77+78+79 = 745 */
    /* test_struct_copy: 'X'+'Y'+'Z' = 88+89+90 = 267 */
    /* Expected: 3+7+745+3+250+267+68+367 = 1710 */
    
    printf("Total: %d\n", total);
    
    /* Quick verification */
    if (total == 1710) {
        printf("All tests passed!\n");
    } else {
        printf("Unexpected result: %d\n", total);
    }
    
    return 0;
}
