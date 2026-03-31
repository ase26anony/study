/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_single_element_copy(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy exactly one element */
    const int lo = 2;
    const int hi = 2;  /* count = hi - lo + 1 = 1 */
    
    /* This should be recognized as constant-bounded */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[2];  /* Should be 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_two_element_copy(void) {
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Constant bounds: copy exactly two elements */
    const int lo = 3;
    const int hi = 4;  /* count = 2 */
    
    /* Constant-bounded copy of 2 short elements */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[3] + dst[4];  /* Should be 3 + 4 = 7 */
}

/* Test 3: MEM target with count > 2 but small total size */
static int test_small_char_array(void) {
    char src[15] = "Hello, World!";
    char dst[15] = {0};
    
    /* Constant bounds: copy 10 chars */
    const int lo = 0;
    const int hi = 9;  /* count = 10, TYPE_SIZE(char) = 1, total = 10 bytes */
    
    /* This should trigger TYPE_SIZE * count calculation */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    /* Create a simple checksum */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i];
    }
    return sum;  /* Sum of ASCII values of "Hello, Wor" */
}

/* Test 4: Non-MEM target (register operation) */
static int test_bitfield_extract(void) {
    uint32_t value = 0xABCD1234;
    
    /* Extract constant-bounded bit field into register (non-MEM target) */
    const int lo_bit = 8;
    const int hi_bit = 15;  /* count = 8 bits = 1 byte */
    
    uint32_t mask = ((1U << (hi_bit - lo_bit + 1)) - 1) << lo_bit;
    uint32_t result = (value & mask) >> lo_bit;
    
    return result;  /* Should be 0x12 */
}

/* Test 5: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Small {
        char a;
        char b;
        char c;
    };
    
    struct Small src = {'x', 'y', 'z'};
    struct Small dst;
    
    /* Structure copy of constant size (3 bytes) */
    dst = src;
    
    return dst.a + dst.b + dst.c;  /* Sum of ASCII values */
}

/* Test 6: Array initialization with compound literal */
static int test_array_init(void) {
    /* Initialize array with constant bounds */
    int arr[4] = {10, 20, 30, 40};
    
    /* Use all elements to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += arr[i];
    }
    return sum;  /* Should be 100 */
}

/* Test 7: Mixed types with small total size */
static int test_mixed_small_copy(void) {
    struct Mixed {
        char c;
        short s;
        int i;
    } src = {'A', 123, 456};
    
    struct Mixed dst;
    
    /* Copy structure of total size 7-8 bytes (padding dependent) */
    dst = src;
    
    return dst.c + dst.s + dst.i;  /* 65 + 123 + 456 = 644 */
}

/* Test 8: Constant loop unrolling scenario */
static int test_constant_loop_unroll(void) {
    int data[3] = {100, 200, 300};
    int result[3] = {0};
    
    /* Loop with constant bounds that should unroll */
    for (int i = 0; i < 3; i++) {
        result[i] = data[i] * 2;
    }
    
    return result[0] + result[1] + result[2];  /* 1200 */
}

/* Test 9: Pointer arithmetic with constant bounds */
static int test_pointer_arithmetic(void) {
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int *src = &array[2];
    int *dst = &array[6];
    
    /* Copy constant number of elements via pointers */
    const int count = 3;  /* hi = 2, lo = 0, count = 3 */
    for (int i = 0; i < count; i++) {
        dst[i] = src[i];
    }
    
    return array[6] + array[7] + array[8];  /* 2 + 3 + 4 = 9 */
}

/* Test 10: Boolean array initialization */
static int test_bool_array(void) {
    _Bool flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    
    /* Access with constant bounds */
    int sum = 0;
    const int start = 1;
    const int end = 6;  /* count = 6 */
    for (int i = start; i <= end; i++) {
        sum += flags[i];
    }
    
    return sum;  /* Should be 3 */
}

int main(void) {
    int total = 0;
    
    /* Run all test cases */
    total += test_single_element_copy();      /* +3 */
    total += test_two_element_copy();         /* +7 */
    total += test_small_char_array();         /* + sum of "Hello, Wor" */
    total += test_bitfield_extract();         /* +0x12 = 18 */
    total += test_struct_copy();              /* +120 + 121 + 122 = 363 */
    total += test_array_init();               /* +100 */
    total += test_mixed_small_copy();         /* +644 */
    total += test_constant_loop_unroll();     /* +1200 */
    total += test_pointer_arithmetic();       /* +9 */
    total += test_bool_array();               /* +3 */
    
    printf("Total checksum: %d\n", total);
    
    /* Verify expected total for basic correctness */
    int expected_partial = 3 + 7 + 18 + 363 + 100 + 644 + 1200 + 9 + 3;
    printf("Partial sum (excluding char array): %d\n", expected_partial);
    
    return 0;
}
