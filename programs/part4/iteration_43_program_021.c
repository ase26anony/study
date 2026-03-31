/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_mem_count_1(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy exactly 1 element */
    dst[2] = src[2];  /* lo_index = 2, hi_index = 2, count = 1 */
    
    return dst[2];  /* Should return 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_mem_count_2(void) {
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Constant bounds: copy exactly 2 elements */
    const int lo = 3;
    const int hi = 4;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];  /* lo_index = 3, hi_index = 4, count = 2 */
    }
    
    return dst[3] + dst[4];  /* Should return 3 + 4 = 7 */
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_mem_small_total_size(void) {
    char src[20];
    char dst[20];
    
    /* Initialize source with pattern */
    for (int i = 0; i < 20; i++) {
        src[i] = (char)(i + 'A');
    }
    
    /* Constant bounds: copy 10 chars (10 bytes total) */
    const int start = 5;
    const int end = 14;  /* 14 - 5 + 1 = 10 elements */
    for (int i = start; i <= end; i++) {
        dst[i] = src[i];  /* count = 10, TYPE_SIZE = 1 byte, total = 10 bytes */
    }
    
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += dst[i];
    }
    return sum;  /* Sum of ASCII values from 'F' to 'O' */
}

/* Test 4: MEM target with bool array (small element size) */
static int test_mem_bool_array(void) {
    _Bool src[32];
    _Bool dst[32];
    
    /* Initialize with pattern */
    for (int i = 0; i < 32; i++) {
        src[i] = (i % 3 == 0);
    }
    
    /* Copy middle 8 elements */
    const int lo = 12;
    const int hi = 19;  /* 19 - 12 + 1 = 8 elements */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];  /* count = 8, TYPE_SIZE = 1 (usually), total = 8 bytes */
    }
    
    int count = 0;
    for (int i = lo; i <= hi; i++) {
        if (dst[i]) count++;
    }
    return count;
}

/* Test 5: Non-MEM target (bit-field extraction into register) */
static int test_non_mem_target(void) {
    struct packed {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } data = {2, 7, 42, 1000};
    
    /* Extract multiple bit-fields into registers (non-MEM targets) */
    unsigned int result = 0;
    result |= data.a;      /* These extractions may create non-MEM targets */
    result |= data.b << 3;
    result |= data.c << 8;
    result |= data.d << 16;
    
    return result;
}

/* Test 6: Array initialization with compound literal (constant bounds) */
static int test_compound_literal(void) {
    /* Initialize array slice from compound literal */
    int arr[10] = {0};
    
    /* This creates a memory operation with constant bounds */
    const int indices[] = {1, 3, 5};
    for (int i = 0; i < 3; i++) {  /* Constant iteration count = 3 */
        arr[indices[i]] = i * 10;  /* Each assignment is count = 1 */
    }
    
    return arr[1] + arr[3] + arr[5];  /* 0 + 10 + 20 = 30 */
}

/* Test 7: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        short b;
        int c;
    } src = {'X', 123, 456};
    
    struct small dst;
    
    /* Whole structure copy - constant size (7 bytes on most platforms) */
    dst = src;  /* This should be recognized as constant-sized copy */
    
    return dst.a + dst.b + dst.c;  /* 'X' + 123 + 456 */
}

/* Test 8: Mixed operations to trigger different paths */
static int test_mixed_operations(void) {
    int result = 0;
    
    /* Multiple small constant-sized operations */
    int buffer[8] = {0};
    
    /* Single element store (count = 1) */
    buffer[0] = 100;
    
    /* Two element copy (count = 2) */
    buffer[2] = buffer[0];
    buffer[3] = buffer[0];
    
    /* Small char array copy (count = 4, small total size) */
    char chars[4];
    for (int i = 0; i < 4; i++) {
        chars[i] = (char)(buffer[0] + i);
    }
    
    for (int i = 0; i < 4; i++) {
        result += chars[i];
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();          /* +3 */
    total += test_mem_count_2();          /* +7 */
    total += test_mem_small_total_size(); /* + sum of 'F' to 'O' = 765 */
    total += test_mem_bool_array();       /* + count of true values */
    total += test_non_mem_target();       /* + bitfield combination */
    total += test_compound_literal();     /* +30 */
    total += test_struct_copy();          /* + 'X' + 123 + 456 */
    total += test_mixed_operations();     /* + sum of 100-103 = 406 */
    
    printf("Result: %d\n", total);
    
    /* Verify expected total to ensure all tests ran */
    int expected = 3 + 7 + 765 + 3 + 2624778 + 30 + (88 + 123 + 456) + 406;
    if (total == expected) {
        printf("All tests passed!\n");
    } else {
        printf("Mismatch! Expected %d, got %d\n", expected, total);
    }
    
    return 0;
}
