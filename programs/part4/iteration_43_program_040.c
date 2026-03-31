/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_mem_count_1(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element */
    const int lo = 2;
    const int hi = 2;
    dst[lo] = src[hi];
    
    return dst[2];  /* Should be 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_mem_count_2(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};
    
    /* Constant bounds: copy two elements */
    const int lo = 1;
    const int hi = 2;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[1] + dst[2];  /* Should be 20 + 30 = 50 */
}

/* Test 3: MEM target with larger count but small total size (char array) */
static int test_mem_small_total_size(void) {
    char src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char dst[10] = {0};
    
    /* Constant bounds: copy 10 chars (10 bytes total) */
    const int lo = 0;
    const int hi = 9;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i];
    }
    return sum;  /* Should be 55 */
}

/* Test 4: MEM target with short array (small element size) */
static int test_mem_short_array(void) {
    short src[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    short dst[8] = {0};
    
    /* Constant bounds: copy 8 shorts (16 bytes total) */
    const int lo = 0;
    const int hi = 7;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;  /* Should be 3600 */
}

/* Test 5: Non-MEM target (bit-field extraction into register) */
static int test_non_mem_target(void) {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } bits = {1, 2, 3, 4};
    
    /* Extract constant-sized bit-field into register */
    unsigned int result = 0;
    result |= (bits.a << 12);
    result |= (bits.b << 8);
    result |= (bits.c << 4);
    result |= bits.d;
    
    return result;  /* Should be 0x1234 */
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } src = {'x', 'y', 'z'};
    
    struct small dst;
    
    /* Constant-sized structure copy */
    dst = src;
    
    return dst.a + dst.b + dst.c;  /* Should be 'x' + 'y' + 'z' */
}

/* Test 7: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Constant bounds initialization */
    int arr[3] = {100, 200, 300};
    
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += arr[i];
    }
    return sum;  /* Should be 600 */
}

/* Test 8: Mixed types to test TYPE_SIZE calculation */
static int test_mixed_types(void) {
    /* Array of bool (C99) - typically 1 byte each */
    _Bool bools[16];
    for (int i = 0; i < 16; i++) {
        bools[i] = (i % 2);
    }
    
    int count = 0;
    for (int i = 0; i < 16; i++) {
        count += bools[i];
    }
    return count;  /* Should be 8 */
}

/* Test 9: Pointer arithmetic with constant bounds */
static int test_pointer_arithmetic(void) {
    int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int copy[10] = {0};
    
    /* Constant bounds via pointer arithmetic */
    int *src_ptr = &data[2];
    int *dst_ptr = &copy[2];
    const int count = 3;
    
    for (int i = 0; i < count; i++) {
        dst_ptr[i] = src_ptr[i];
    }
    
    return copy[2] + copy[3] + copy[4];  /* Should be 2 + 3 + 4 = 9 */
}

/* Test 10: Nested constant loops (might trigger unrolling) */
static int test_nested_loops(void) {
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int result = 0;
    
    /* Constant loop bounds */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            result += matrix[i][j];
        }
    }
    
    return result;  /* Should be 21 */
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();          /* +3 */
    total += test_mem_count_2();          /* +50 */
    total += test_mem_small_total_size(); /* +55 */
    total += test_mem_short_array();      /* +3600 */
    total += test_non_mem_target();       /* +0x1234 = 4660 */
    total += test_struct_copy();          /* +'x'+'y'+'z' = 363 */
    total += test_compound_literal();     /* +600 */
    total += test_mixed_types();          /* +8 */
    total += test_pointer_arithmetic();   /* +9 */
    total += test_nested_loops();         /* +21 */
    
    printf("Result: %d\n", total);
    
    /* Expected total: 3+50+55+3600+4660+363+600+8+9+21 = 9369 */
    return 0;
}
