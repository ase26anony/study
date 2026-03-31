/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_mem_count_1(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element */
    const int lo = 2;
    const int hi = 2;  /* count = 1 */
    dst[lo] = src[hi];
    
    return dst[2];  /* Should be 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_mem_count_2(void) {
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
    
    /* This should trigger TYPE_SIZE * count calculation */
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
    
    /* Extract constant-sized bit-field into register (!MEM_P(target)) */
    unsigned int val = 0;
    
    /* Constant bounds for bit-field extraction */
    val = s.b;  /* This should be treated as non-MEM target */
    
    return val;  /* Should be 2 */
}

/* Test 5: MEM target with bool array (small element size) */
static int test_bool_array(void) {
    _Bool src[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    _Bool dst[8] = {0};
    
    /* Constant bounds: copy 6 bools */
    const int lo = 1;
    const int hi = 6;  /* count = 6, but bool size is small */
    
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = lo; i <= hi; i++) {
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
    
    /* Constant-sized structure copy (4 bytes total) */
    p2 = p1;
    
    return p2.x + p2.y;  /* Should be 30 */
}

/* Test 7: Array slice in loop with constant iteration count */
static int test_unrolled_copy(void) {
    int src[4] = {100, 200, 300, 400};
    int dst[4] = {0};
    
    /* Loop with constant bounds that might get unrolled */
    for (int i = 0; i < 3; i++) {  /* Constant iteration count = 3 */
        dst[i] = src[i];
    }
    
    return dst[0] + dst[1] + dst[2];  /* Should be 600 */
}

/* Test 8: Using sizeof in bounds calculation */
static int test_sizeof_bounds(void) {
    char buffer[100];
    char pattern[] = "Hello";
    
    /* Constant bounds using sizeof */
    const int count = sizeof(pattern) - 1;  /* Compile-time constant = 5 */
    
    for (int i = 0; i < count; i++) {
        buffer[i] = pattern[i];
    }
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += buffer[i];
    }
    return sum;  /* Sum of 'H' 'e' 'l' 'l' 'o' */
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();      /* +3 */
    total += test_mem_count_2();      /* +7 */
    total += test_mem_small_total_size(); /* + ASCII sum */
    total += test_non_mem_target();   /* +2 */
    total += test_bool_array();       /* +3 */
    total += test_struct_copy();      /* +30 */
    total += test_unrolled_copy();    /* +600 */
    total += test_sizeof_bounds();    /* + ASCII sum */
    
    printf("Result: %d\n", total);
    
    /* Verify expected total */
    int expected = 3 + 7 + (/* ASCII for FGHIJKLMNO */ 70+71+72+73+74+75+76+77+78+79) 
                   + 2 + 3 + 30 + 600 + (/* ASCII for Hello */ 72+101+108+108+111);
    printf("Expected: %d\n", expected);
    
    return (total == expected) ? 0 : 1;
}
