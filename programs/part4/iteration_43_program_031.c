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
    const int hi = 2;  /* count = hi - lo + 1 = 1 */
    
    /* This should trigger the MEM target with count <= 2 path */
    dst[lo] = src[hi];
    
    return dst[2];  /* Should be 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_two_element_copy(void) {
    short src[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    short dst[10] = {0};
    
    /* Constant bounds: copy two elements */
    const int lo = 3;
    const int hi = 4;  /* count = 2 */
    
    /* Should use scalar expansion for 2 elements */
    dst[lo] = src[lo];
    dst[hi] = src[hi];
    
    return dst[3] + dst[4];  /* Should be 40 + 50 = 90 */
}

/* Test 3: MEM target with count > 2 but small total size */
static int test_small_char_array(void) {
    char src[15] = "Hello, World!";
    char dst[15] = {0};
    
    /* Constant bounds for entire array */
    const int lo = 0;
    const int hi = 13;  /* count = 14, but char size = 1, total = 14 bytes */
    
    /* This should trigger TYPE_SIZE * count calculation */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    /* Simple checksum */
    int sum = 0;
    for (int i = 0; i <= hi; i++) {
        sum += dst[i];
    }
    return sum;  /* Sum of ASCII values */
}

/* Test 4: Non-MEM target (register operation) */
static int test_bitfield_extract(void) {
    /* Create a constant value with known bits */
    const uint32_t value = 0x12345678;
    
    /* Extract constant-sized bitfield into register (non-MEM target) */
    const int start_bit = 8;
    const int end_bit = 15;  /* 8 bits total */
    
    /* This should trigger !MEM_P(target) path */
    uint32_t mask = ((1U << (end_bit - start_bit + 1)) - 1) << start_bit;
    uint32_t result = (value & mask) >> start_bit;
    
    return result;  /* Should be 0x56 */
}

/* Test 5: Structure copy with constant size */
static int test_small_struct_copy(void) {
    struct SmallStruct {
        char a;
        char b;
        char c;
    };
    
    struct SmallStruct src = {'X', 'Y', 'Z'};
    struct SmallStruct dst;
    
    /* Constant-sized structure copy - total size = 3 bytes */
    dst = src;
    
    return dst.a + dst.b + dst.c;  /* Sum of ASCII: 'X' + 'Y' + 'Z' */
}

/* Test 6: Array initialization with compound literal */
static int test_array_init_compound(void) {
    /* Initialize array with constant bounds */
    int arr[4] = { [0] = 100, [2] = 200, [3] = 300 };
    
    /* Force computation with constant indices */
    const int idx1 = 0;
    const int idx2 = 2;
    const int idx3 = 3;
    
    return arr[idx1] + arr[idx2] + arr[idx3];  /* 100 + 200 + 300 = 600 */
}

/* Test 7: Loop with constant iteration count (should unroll) */
static int test_constant_loop(void) {
    int values[3];
    
    /* Loop with compile-time constant bounds */
    const int start = 0;
    const int end = 2;  /* 3 iterations */
    
    for (int i = start; i <= end; i++) {
        values[i] = (i + 1) * 10;
    }
    
    return values[0] + values[1] + values[2];  /* 10 + 20 + 30 = 60 */
}

/* Test 8: Mixed types with small total size */
static int test_mixed_small_types(void) {
    struct Mixed {
        char c;
        short s;
        int i;
    } m1 = {'A', 123, 456}, m2;
    
    /* Constant-sized copy of mixed structure */
    m2 = m1;
    
    return m2.c + m2.s + m2.i;  /* 65 + 123 + 456 = 644 */
}

int main(void) {
    int total = 0;
    
    /* Run all test cases */
    total += test_single_element_copy();      /* + 3 */
    total += test_two_element_copy();         /* + 90 */
    total += test_small_char_array();         /* + sum of "Hello, World!" ASCII */
    total += test_bitfield_extract();         /* + 0x56 = 86 */
    total += test_small_struct_copy();        /* + 231 (ASCII X+Y+Z) */
    total += test_array_init_compound();      /* + 600 */
    total += test_constant_loop();            /* + 60 */
    total += test_mixed_small_types();        /* + 644 */
    
    printf("Total checksum: %d\n", total);
    
    /* Expected total calculation:
       3 + 90 + 1116 + 86 + 231 + 600 + 60 + 644 = 2830
       Note: "Hello, World!" ASCII sum = 1116
    */
    
    return 0;
}
