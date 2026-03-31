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
    dst[lo] = src[hi];  /* count = 1 */
    
    return dst[2];
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_mem_count_2(void) {
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Constant bounds: copy two elements */
    const int lo = 3;
    const int hi = 4;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[3] + dst[4];
}

/* Test 3: MEM target with count > 2 but small total size */
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
    return sum;
}

/* Test 4: Non-MEM target (register operation) */
static int test_non_mem_target(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* Extract bit-field into register (non-MEM target) */
    unsigned int val = s.b;  /* This should trigger !MEM_P(target) path */
    
    return val;
}

/* Test 5: Array initialization with constant bounds */
static int test_array_init(void) {
    int arr[10];
    
    /* Initialize slice with constant bounds */
    const int start = 2;
    const int end = 5;  /* count = 4 */
    
    for (int i = start; i <= end; i++) {
        arr[i] = i * 10;
    }
    
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        char x;
        char y;
        char z;
    } p1 = {10, 20, 30}, p2;
    
    /* Structure copy - constant size (3 bytes) */
    p2 = p1;
    
    return p2.x + p2.y + p2.z;
}

/* Test 7: Mixed types with constant bounds */
static int test_mixed_types(void) {
    long data[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    long result[8] = {0};
    
    /* Copy with constant bounds - count = 3, element size = 8 bytes */
    const int lo = 1;
    const int hi = 3;  /* count = 3 */
    
    for (int i = lo; i <= hi; i++) {
        result[i] = data[i];
    }
    
    return (int)(result[1] + result[2] + result[3]);
}

/* Test 8: Pointer arithmetic with constant bounds */
static int test_pointer_arithmetic(void) {
    int buffer[20];
    int *src = buffer + 5;
    int *dst = buffer + 10;
    
    /* Initialize source */
    for (int i = 0; i < 5; i++) {
        src[i] = i * 100;
    }
    
    /* Copy with constant bounds using pointer arithmetic */
    const int count = 3;
    for (int i = 0; i < count; i++) {
        dst[i] = src[i];
    }
    
    return dst[0] + dst[1] + dst[2];
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();      /* Should return 3 */
    total += test_mem_count_2();      /* Should return 7 (3+4) */
    total += test_mem_small_total_size(); /* Sum of ASCII values */
    total += test_non_mem_target();   /* Should return 2 */
    total += test_array_init();       /* Should return 120 (20+30+40+50) */
    total += test_struct_copy();      /* Should return 60 */
    total += test_mixed_types();      /* Should return 900 */
    total += test_pointer_arithmetic(); /* Should return 300 */
    
    printf("Result: %d\n", total);
    
    /* Expected total calculation:
       3 + 7 + ? + 2 + 120 + 60 + 900 + 300 = ? + 1392
       Where ? is sum of ASCII values from 'F' to 'O' (70+71+...+79) = 745
       Total = 745 + 1392 = 2137
    */
    
    return 0;
}
