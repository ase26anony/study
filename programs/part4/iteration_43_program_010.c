/* test_expr_coverage.c - Test cases for GCC expr.cc constant bounds checking */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_single_element_copy(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element at index 2 */
    const int lo = 2;
    const int hi = 2;
    int count = hi - lo + 1;  /* count = 1 */
    
    /* This should trigger the MEM target with count <= 2 path */
    dst[lo] = src[lo];
    
    return dst[2];  /* Should be 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_two_element_copy(void) {
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Constant bounds: copy two elements starting at index 3 */
    const int lo = 3;
    const int hi = 4;
    int count = hi - lo + 1;  /* count = 2 */
    
    /* This should trigger the MEM target with count <= 2 path */
    dst[lo] = src[lo];
    dst[hi] = src[hi];
    
    return dst[3] + dst[4];  /* Should be 3 + 4 = 7 */
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[20] = "Hello, World!";
    char dst[20] = {0};
    
    /* Constant bounds: copy 10 characters starting at index 0 */
    const int lo = 0;
    const int hi = 9;
    int count = hi - lo + 1;  /* count = 10 */
    /* TYPE_SIZE(char) = 8 bits, total = 10 * 8 = 80 bits = 10 bytes */
    
    /* This should trigger the TYPE_SIZE * count calculation */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i];
    }
    return sum;  /* Sum of ASCII values */
}

/* Test 4: Non-MEM target (register operation) */
static int test_bitfield_extract(void) {
    /* Create a 64-bit value with specific bit pattern */
    uint64_t value = 0x123456789ABCDEF0ULL;
    
    /* Extract constant-sized bitfield into register (non-MEM target) */
    const int start_bit = 8;
    const int end_bit = 15;
    const int bit_count = end_bit - start_bit + 1;  /* count = 8 */
    
    /* This should trigger the !MEM_P(target) path */
    uint64_t mask = ((1ULL << bit_count) - 1) << start_bit;
    uint64_t result = (value & mask) >> start_bit;
    
    return (int)result;  /* Should be 0xEF */
}

/* Test 5: Structure copy with constant size */
static int test_struct_copy(void) {
    struct SmallStruct {
        char a;
        char b;
        char c;
    };
    
    struct SmallStruct src = {'X', 'Y', 'Z'};
    struct SmallStruct dst;
    
    /* Constant bounds through structure copy */
    const int lo = 0;
    const int hi = sizeof(struct SmallStruct) - 1;
    int count = hi - lo + 1;  /* count = 3, TYPE_SIZE = 8 bits */
    
    /* This may trigger MEM target with small total size */
    dst = src;
    
    return dst.a + dst.b + dst.c;  /* Sum of ASCII values */
}

/* Test 6: Array initialization with compound literal */
static int test_array_init(void) {
    /* Initialize array with constant bounds */
    int arr[4] = {10, 20, 30, 40};
    
    /* Access with constant indices */
    const int lo = 1;
    const int hi = 2;
    int count = hi - lo + 1;  /* count = 2 */
    
    int sum = 0;
    for (int i = lo; i <= hi; i++) {
        sum += arr[i];
    }
    
    return sum;  /* 20 + 30 = 50 */
}

/* Test 7: Mixed types with small total size */
static int test_mixed_small_types(void) {
    /* Array of 16-bit types with count = 4 */
    int16_t src[4] = {100, 200, 300, 400};
    int16_t dst[4] = {0};
    
    const int lo = 0;
    const int hi = 3;
    int count = hi - lo + 1;  /* count = 4 */
    /* TYPE_SIZE(int16_t) = 16 bits, total = 4 * 16 = 64 bits = 8 bytes */
    
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[0] + dst[1] + dst[2] + dst[3];  /* 100 + 200 + 300 + 400 = 1000 */
}

/* Test 8: Boolean array (very small element size) */
static int test_bool_array(void) {
    _Bool src[32] = {0};  /* 32 elements */
    _Bool dst[32] = {0};
    
    /* Set alternating pattern */
    for (int i = 0; i < 32; i++) {
        src[i] = (i % 2);
    }
    
    const int lo = 0;
    const int hi = 31;
    int count = hi - lo + 1;  /* count = 32 */
    /* TYPE_SIZE(_Bool) is at least 8 bits, total >= 32 * 8 = 256 bits = 32 bytes */
    
    /* Copy a subset with constant bounds */
    const int copy_lo = 4;
    const int copy_hi = 19;
    int copy_count = copy_hi - copy_lo + 1;  /* count = 16 */
    
    for (int i = copy_lo; i <= copy_hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = copy_lo; i <= copy_hi; i++) {
        sum += dst[i];
    }
    
    return sum;  /* Should be 8 (half of 16 are true) */
}

/* Test 9: Pointer array with constant bounds */
static int test_pointer_array(void) {
    int x = 1, y = 2, z = 3, w = 4;
    int* src[4] = {&x, &y, &z, &w};
    int* dst[4] = {0};
    
    const int lo = 0;
    const int hi = 1;
    int count = hi - lo + 1;  /* count = 2 */
    
    /* Copy two pointers */
    dst[lo] = src[lo];
    dst[hi] = src[hi];
    
    return *dst[0] + *dst[1];  /* 1 + 2 = 3 */
}

/* Test 10: Nested constant bounds in loop */
static int test_nested_constant_bounds(void) {
    int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    int result[3][3] = {{0}};
    
    /* Outer loop with constant bounds */
    for (int i = 0; i < 3; i++) {
        const int lo = 0;
        const int hi = 2;
        int count = hi - lo + 1;  /* count = 3 */
        
        /* Copy row with constant bounds */
        for (int j = lo; j <= hi; j++) {
            result[i][j] = matrix[i][j];
        }
    }
    
    return result[0][0] + result[1][1] + result[2][2];  /* 1 + 5 + 9 = 15 */
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_single_element_copy();      /* +3 */
    total += test_two_element_copy();         /* +7 */
    total += test_small_char_array();         /* +sum of "Hello, Wor" */
    total += test_bitfield_extract();         /* +0xEF = 239 */
    total += test_struct_copy();              /* +'X' + 'Y' + 'Z' = 88 + 89 + 90 = 267 */
    total += test_array_init();               /* +50 */
    total += test_mixed_small_types();        /* +1000 */
    total += test_bool_array();               /* +8 */
    total += test_pointer_array();            /* +3 */
    total += test_nested_constant_bounds();   /* +15 */
    
    printf("Total checksum: %d\n", total);
    
    /* Verify expected total */
    int expected = 3 + 7 + (72+101+108+108+111+44+32+87+111+114) /* Hello, Wor */ 
                   + 239 + 267 + 50 + 1000 + 8 + 3 + 15;
    
    printf("Expected: %d\n", expected);
    printf("Test %s\n", total == expected ? "PASSED" : "FAILED");
    
    return total == expected ? 0 : 1;
}
