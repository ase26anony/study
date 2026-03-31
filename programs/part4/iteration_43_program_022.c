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
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Constant bounds: copy two elements */
    const int start = 3;
    const int end = 4;
    for (int i = start; i <= end; i++) {
        dst[i] = src[i];
    }
    
    return dst[3] + dst[4];  /* Should be 3 + 4 = 7 */
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
    const int lo = 5;
    const int hi = 14;  /* count = 10 */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = lo; i <= hi; i++) {
        sum += dst[i];
    }
    return sum;  /* Sum of ASCII values */
}

/* Test 4: Non-MEM target (bit-field extraction into register) */
static int test_non_mem_target(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* Extract constant-sized bit-field into register */
    unsigned int val = s.b;  /* This should trigger !MEM_P(target) path */
    
    return val;  /* Should be 2 */
}

/* Test 5: MEM target with bool array (small element size) */
static int test_bool_array(void) {
    _Bool src[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    _Bool dst[8] = {0};
    
    /* Constant bounds: copy 6 bools */
    const int lo = 1;
    const int hi = 6;  /* count = 6 */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;  /* Should be 3 */
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
    } p1 = {10, 20}, p2;
    
    /* Copy entire structure (constant size = 4 bytes) */
    p2 = p1;
    
    return p2.x + p2.y;  /* Should be 30 */
}

/* Test 7: Array initialization with compound literal */
static int test_compound_literal(void) {
    int arr[5];
    
    /* Initialize with constant bounds */
    const int *src = (const int[]){100, 200, 300, 400, 500};
    const int lo = 1;
    const int hi = 3;  /* count = 3 */
    for (int i = lo; i <= hi; i++) {
        arr[i] = src[i];
    }
    
    return arr[1] + arr[2] + arr[3];  /* Should be 200+300+400=900 */
}

/* Test 8: Using sizeof in bounds calculation */
static int test_sizeof_bounds(void) {
    long src[10];
    long dst[10];
    
    /* Initialize source */
    for (int i = 0; i < 10; i++) {
        src[i] = i * 10L;
    }
    
    /* Constant bounds using sizeof */
    const int elements = sizeof(src) / sizeof(src[0]) / 2;  /* 5 elements */
    const int lo = 0;
    const int hi = elements - 1;  /* count = 5 */
    
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return (int)(dst[0] + dst[4]);  /* Should be 0 + 40 = 40 */
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();      /* +3 */
    total += test_mem_count_2();      /* +7 */
    total += test_mem_small_total_size(); /* + ASCII sum */
    total += test_non_mem_target();   /* +2 */
    total += test_bool_array();       /* +3 */
    total += test_struct_copy();      /* +30 */
    total += test_compound_literal(); /* +900 */
    total += test_sizeof_bounds();    /* +40 */
    
    printf("Result: %d\n", total);
    
    /* Verify expected total */
    int expected = 3 + 7 + (/* ASCII sum for 'F' to 'O': */ 
                           'F' + 'G' + 'H' + 'I' + 'J' + 
                           'K' + 'L' + 'M' + 'N' + 'O') + 
                  2 + 3 + 30 + 900 + 40;
    
    if (total == expected) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Mismatch! Expected: %d, Got: %d\n", expected, total);
        return 1;
    }
}
