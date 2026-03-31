/* Test for constant-bounded memory operations in expr.cc */
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
    
    return dst[2];
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
    
    return dst[1] + dst[2];
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_mem_small_total_size(void) {
    char src[10] = "abcdefghi";
    char dst[10] = {0};
    
    /* Constant bounds: copy 5 chars (5 bytes total) */
    const int lo = 2;
    const int hi = 6;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return (int)dst[2] + dst[3] + dst[4] + dst[5] + dst[6];
}

/* Test 4: MEM target with short array (2 bytes per element) */
static int test_mem_short_array(void) {
    short src[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    short dst[8] = {0};
    
    /* Constant bounds: copy 4 shorts (8 bytes total) */
    const int lo = 1;
    const int hi = 4;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[1] + dst[2] + dst[3] + dst[4];
}

/* Test 5: Non-MEM target (bit-field extraction into register) */
static int test_non_mem_target(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* Extract constant-sized bit-field into register */
    unsigned int result = 0;
    const int start = 4;  /* bit position */
    const int width = 4;  /* bit width */
    
    /* This should create a non-MEM target operation */
    result = (s.b << start) & ((1u << width) - 1);
    
    return result;
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        int x;
        int y;
    };
    
    struct Point src = {42, 24};
    struct Point dst;
    
    /* Structure copy - constant size (2 ints = 8 bytes typically) */
    dst = src;
    
    return dst.x + dst.y;
}

/* Test 7: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Initialize array with constant bounds */
    int arr[3] = { [0] = 100, [2] = 300 };
    arr[1] = 200;  /* Fill the middle element */
    
    return arr[0] + arr[1] + arr[2];
}

/* Test 8: Mixed types to test TYPE_SIZE calculation */
static int test_mixed_types(void) {
    typedef struct {
        char a;
        int b;
    } SmallStruct;
    
    SmallStruct src[2] = {{'x', 1}, {'y', 2}};
    SmallStruct dst[2];
    
    /* Copy with constant bounds */
    const int lo = 0;
    const int hi = 1;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[0].b + dst[1].b;
}

/* Test 9: Boolean array (small element size) */
static int test_bool_array(void) {
    _Bool src[16] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
    _Bool dst[16] = {0};
    
    /* Copy with constant bounds - 8 elements */
    const int lo = 4;
    const int hi = 11;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = lo; i <= hi; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 10: Pointer array copy */
static int test_pointer_array(void) {
    const char *src[4] = {"hello", "world", "test", "data"};
    const char *dst[4] = {0};
    
    /* Copy with constant bounds */
    const int lo = 0;
    const int hi = 2;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    /* Use pointers to prevent optimization */
    return (dst[0] != 0) + (dst[1] != 0) + (dst[2] != 0);
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();      /* Should return 3 */
    total += test_mem_count_2();      /* Should return 50 */
    total += test_mem_small_total_size(); /* Sum of char values */
    total += test_mem_short_array();  /* Should return 2000 */
    total += test_non_mem_target();   /* Should return 2 */
    total += test_struct_copy();      /* Should return 66 */
    total += test_compound_literal(); /* Should return 600 */
    total += test_mixed_types();      /* Should return 3 */
    total += test_bool_array();       /* Should return 4 */
    total += test_pointer_array();    /* Should return 3 */
    
    printf("Total: %d\n", total);
    return 0;
}
