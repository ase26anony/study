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
    char src[15] = "Hello, World!";
    char dst[15] = {0};
    
    /* Constant bounds: copy 13 characters (count = 13, but char size = 1) */
    const int lo = 0;
    const int hi = 12;  /* count = 13, total size = 13 bytes */
    
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return (int)dst[7];  /* Should be 'W' = 87 */
}

/* Test 4: MEM target with small struct array */
static int test_small_struct_array(void) {
    struct small {
        char a;
        char b;
    } src[4] = {{1,2}, {3,4}, {5,6}, {7,8}};
    struct small dst[4] = {{0}};
    
    /* Constant bounds: copy 3 elements, each 2 bytes */
    const int lo = 0;
    const int hi = 2;  /* count = 3, total size = 6 bytes */
    
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[1].a + dst[2].b;  /* Should be 3 + 6 = 9 */
}

/* Test 5: Non-MEM target (bit-field extraction into register) */
static int test_bitfield_extract(void) {
    struct bitfields {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } bits = {.a = 5, .b = 10, .c = 3, .d = 7};
    
    /* Extract constant-sized bit-field into register (non-MEM target) */
    unsigned int result = 0;
    
    /* Constant bounds extraction */
    result = bits.b;  /* Extracting 4-bit field into register */
    
    return result;  /* Should be 10 */
}

/* Test 6: MEM target with bool array (small element size) */
static int test_bool_array(void) {
    _Bool src[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    _Bool dst[8] = {0};
    
    /* Constant bounds: copy 6 elements */
    const int lo = 1;
    const int hi = 6;  /* count = 6, total size small (likely 6 bytes) */
    
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[2] + dst[4] + dst[6];  /* Should be 1 + 1 + 1 = 3 */
}

/* Test 7: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Direct initialization with constant bounds */
    int arr[3] = {10, 20, 30};
    
    return arr[0] + arr[2];  /* Should be 40 */
}

/* Test 8: Memory copy with constant offset using pointer arithmetic */
static int test_pointer_arithmetic(void) {
    int buffer[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int temp[3];
    
    /* Constant bounds through pointer arithmetic */
    const int start = 2;
    const int end = 4;  /* count = 3 */
    
    for (int i = 0; i <= (end - start); i++) {
        temp[i] = buffer[start + i];
    }
    
    return temp[0] + temp[2];  /* Should be 2 + 4 = 6 */
}

/* Test 9: Mixed types to test TYPE_SIZE calculation */
static int test_mixed_types(void) {
    short src[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    short dst[8] = {0};
    
    /* Constant bounds with short (2 bytes) */
    enum { LO_IDX = 1, HI_IDX = 3 };  /* count = 3, total = 6 bytes */
    
    for (int i = LO_IDX; i <= HI_IDX; i++) {
        dst[i] = src[i] + 1;
    }
    
    return dst[1] + dst[3];  /* Should be 201 + 401 = 602 */
}

/* Test 10: Nested constant loop for potential unrolling */
static int test_nested_constant_loop(void) {
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int copy[2][3] = {{0}};
    
    /* Nested loops with constant bounds */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            copy[i][j] = matrix[i][j];
        }
    }
    
    return copy[0][0] + copy[1][2];  /* Should be 1 + 6 = 7 */
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_single_element_copy();      /* +3 */
    total += test_two_element_copy();         /* +7 */
    total += test_small_char_array();         /* +87 */
    total += test_small_struct_array();       /* +9 */
    total += test_bitfield_extract();         /* +10 */
    total += test_bool_array();               /* +3 */
    total += test_compound_literal();         /* +40 */
    total += test_pointer_arithmetic();       /* +6 */
    total += test_mixed_types();              /* +602 */
    total += test_nested_constant_loop();     /* +7 */
    
    /* Expected total: 3+7+87+9+10+3+40+6+602+7 = 774 */
    printf("Result: %d\n", total);
    
    /* Verify expected result */
    if (total == 774) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Unexpected result!\n");
        return 1;
    }
}
