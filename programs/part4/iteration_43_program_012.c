/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_mem_count_1(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Copy single element with constant bounds */
    dst[2] = src[2];  /* lo_index = 2, hi_index = 2, count = 1 */
    
    return dst[2];  /* Should return 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_mem_count_2(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};
    
    /* Copy two elements with constant bounds */
    dst[0] = src[0];  /* First element */
    dst[1] = src[1];  /* Second element - compiler might combine these */
    
    /* Alternative: Use memcpy with compile-time known small size */
    int src2[3] = {100, 200, 300};
    int dst2[3] = {0};
    
    /* This should be recognized as constant-bounded copy of 2 ints */
    for (int i = 0; i < 2; i++) {  /* Constant loop bound */
        dst2[i] = src2[i];
    }
    
    return dst[0] + dst[1] + dst2[0] + dst2[1];  /* 10 + 20 + 100 + 200 = 330 */
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_mem_small_total_size(void) {
    char src[10] = "123456789";
    char dst[10] = {0};
    
    /* Copy 9 chars + null terminator = 10 bytes total */
    /* TYPE_SIZE(char) = 8 bits, count = 10, total = 80 bits = 10 bytes */
    for (int i = 0; i < 10; i++) {  /* Constant loop bound */
        dst[i] = src[i];
    }
    
    /* Alternative: Initialize small char array */
    char arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};  /* 8 elements, 8 bytes total */
    
    int sum = 0;
    for (int i = 0; i < 10; i++) sum += dst[i];
    for (int i = 0; i < 8; i++) sum += arr[i];
    
    return sum;  /* Sum of ASCII '1'..'9' + null + 1..8 */
}

/* Test 4: MEM target with short array (16-bit elements) */
static int test_mem_short_array(void) {
    short src[4] = {100, 200, 300, 400};
    short dst[4] = {0};
    
    /* Copy 4 shorts = 8 bytes total */
    /* TYPE_SIZE(short) = 16 bits, count = 4, total = 64 bits = 8 bytes */
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    
    return dst[0] + dst[1] + dst[2] + dst[3];  /* 100 + 200 + 300 + 400 = 1000 */
}

/* Test 5: Non-MEM target (register operations) */
static int test_non_mem_target(void) {
    /* Bit-field extraction into register (!MEM_P(target) path) */
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } bf = {1, 2, 3, 4};
    
    /* Extracting multiple bit-fields - might trigger scalar expansion */
    unsigned int a = bf.a;  /* Extracted to register */
    unsigned int b = bf.b;
    unsigned int c = bf.c;
    unsigned int d = bf.d;
    
    /* Packing values into integer */
    uint32_t packed = (a << 12) | (b << 8) | (c << 4) | d;
    
    return packed;  /* Should be 0x1234 */
}

/* Test 6: Mixed operations with constant indices */
static int test_mixed_constant_indices(void) {
    int array[10] = {0};
    
    /* Initialize specific elements with constant indices */
    const int lo = 3;
    const int hi = 6;
    
    /* This creates a constant-bounded region: indices 3-6 inclusive */
    for (int i = lo; i <= hi; i++) {  /* count = 4 */
        array[i] = i * 10;
    }
    
    /* Another with different element type */
    char chars[20];
    const int start = 5;
    const int end = 14;
    
    for (int i = start; i <= end; i++) {  /* count = 10, char elements */
        chars[i] = 'A' + (i - start);
    }
    
    int sum = 0;
    for (int i = lo; i <= hi; i++) sum += array[i];  /* 30+40+50+60 = 180 */
    for (int i = start; i <= end; i++) sum += chars[i];  /* ASCII A-J */
    
    return sum;  /* 180 + ASCII sum of A-J */
}

/* Test 7: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {'x', 'y', 'z'}, s2;
    
    /* Copy structure - constant size (3 bytes) */
    s2 = s1;
    
    /* Array of small structures */
    struct small arr1[2] = {{'a','b','c'}, {'d','e','f'}};
    struct small arr2[2];
    
    /* Copy array of structs - count = 2, element size = 3 bytes */
    for (int i = 0; i < 2; i++) {
        arr2[i] = arr1[i];
    }
    
    return s2.a + s2.b + s2.c + arr2[0].a + arr2[1].c;
}

/* Test 8: Using sizeof in bounds calculation */
static int test_sizeof_bounds(void) {
    int buffer[100];
    int init_value = 42;
    
    /* Initialize first N elements where N is compile-time constant */
    #define INIT_COUNT 5
    for (int i = 0; i < INIT_COUNT; i++) {
        buffer[i] = init_value + i;
    }
    
    /* Another using sizeof-derived count */
    char small_buf[sizeof(int) * 3];  /* 12 bytes for 32-bit, 24 for 64-bit */
    for (size_t i = 0; i < sizeof(small_buf); i++) {
        small_buf[i] = (char)(i + 1);
    }
    
    int sum = 0;
    for (int i = 0; i < INIT_COUNT; i++) sum += buffer[i];
    for (size_t i = 0; i < sizeof(small_buf); i++) sum += small_buf[i];
    
    return sum;
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();          /* Should add 3 */
    total += test_mem_count_2();          /* Should add 330 */
    total += test_mem_small_total_size(); /* Should add sum of ASCII values */
    total += test_mem_short_array();      /* Should add 1000 */
    total += test_non_mem_target();       /* Should add 0x1234 = 4660 */
    total += test_mixed_constant_indices(); /* Should add computed sum */
    total += test_struct_copy();          /* Should add sum of chars */
    total += test_sizeof_bounds();        /* Should add computed sum */
    
    printf("Total: %d\n", total);
    return 0;
}
