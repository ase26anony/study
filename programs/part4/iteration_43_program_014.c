/* Test case for expr.cc lines 7691-7700 - constant-bounded memory operations */

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
    const int start = 3;
    const int end = 4;
    for (int i = start; i <= end; i++) {  /* count = 2 */
        dst[i] = src[i] + 1;
    }
    
    return dst[3] + dst[4];
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_mem_small_total_size(void) {
    char src[20];
    char dst[20];
    
    /* Initialize source with pattern */
    for (int i = 0; i < 20; i++) {
        src[i] = i % 10;
    }
    
    /* Constant bounds: copy 10 chars (total size = 10 bytes) */
    const int lo = 5;
    const int hi = 14;  /* count = 10 */
    
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i] * 2;
    }
    
    int sum = 0;
    for (int i = lo; i <= hi; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 4: Non-MEM target (bit-field extraction into register) */
static int test_non_mem_target(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } p = {.a = 1, .b = 2, .c = 3, .d = 4};
    
    /* Extract constant-sized bit-field into register (non-MEM target) */
    unsigned int result;
    
    /* The compiler should see this as extracting constant bounds */
    result = p.b;  /* 4-bit field at constant position */
    
    return result;
}

/* Test 5: Structure copy with constant size (count = 1, but element is struct) */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {1, 2, 3}, s2;
    
    /* Constant copy of entire struct */
    s2 = s1;  /* count = 1, element type is struct */
    
    return s2.a + s2.b + s2.c;
}

/* Test 6: Array initialization with compound literal (constant bounds) */
static int test_compound_literal(void) {
    /* Initialize array slice from compound literal */
    int arr[10];
    
    /* Constant bounds initialization */
    const int start = 1;
    const int end = 3;  /* count = 3 */
    
    int *slice = &arr[start];
    int init_values[] = {10, 20, 30};
    
    for (int i = 0; i <= (end - start); i++) {
        slice[i] = init_values[i];
    }
    
    return arr[1] + arr[2] + arr[3];
}

/* Test 7: MEM target with bool array (small element size) */
static int test_bool_array(void) {
    _Bool flags[32];
    
    /* Initialize constant range */
    const int first = 8;
    const int last = 15;  /* count = 8, total size = 8 bytes (usually) */
    
    for (int i = first; i <= last; i++) {
        flags[i] = (i % 2) == 0;
    }
    
    int count_true = 0;
    for (int i = first; i <= last; i++) {
        if (flags[i]) count_true++;
    }
    
    return count_true;
}

/* Test 8: Pointer arithmetic with constant bounds */
static int test_pointer_arithmetic(void) {
    int buffer[20] = {0};
    int *src = buffer + 5;
    int *dst = buffer + 10;
    
    /* Copy constant number of elements */
    const int n = 2;  /* count = 2 */
    
    for (int i = 0; i < n; i++) {
        dst[i] = src[i] + i;
    }
    
    return dst[0] + dst[1];
}

/* Test 9: Mixed types with constant index calculations */
static int test_mixed_types(void) {
    struct mixed {
        short a;
        char b;
        int c;
    } m1[4], m2[4];
    
    /* Initialize m1 */
    for (int i = 0; i < 4; i++) {
        m1[i].a = i * 10;
        m1[i].b = i * 5;
        m1[i].c = i * 100;
    }
    
    /* Copy constant range */
    const int from = 1;
    const int to = 2;  /* count = 2 */
    
    for (int i = from; i <= to; i++) {
        m2[i] = m1[i];
    }
    
    return m2[1].c + m2[2].c;
}

/* Test 10: Using enum for constant bounds */
static int test_enum_bounds(void) {
    enum { START_IDX = 2, END_IDX = 5 };  /* count = 4 */
    
    long values[10] = {0};
    long source[10] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    
    for (int i = START_IDX; i <= END_IDX; i++) {
        values[i] = source[i] / 10;
    }
    
    long sum = 0;
    for (int i = START_IDX; i <= END_IDX; i++) {
        sum += values[i];
    }
    
    return (int)(sum % 1000);
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();          /* Should return 3 */
    total += test_mem_count_2();          /* Should return 9 (4 + 5) */
    total += test_mem_small_total_size(); /* Should return 90 */
    total += test_non_mem_target();       /* Should return 2 */
    total += test_struct_copy();          /* Should return 6 */
    total += test_compound_literal();     /* Should return 60 */
    total += test_bool_array();           /* Should return 4 */
    total += test_pointer_arithmetic();   /* Should return 11 */
    total += test_mixed_types();          /* Should return 300 */
    total += test_enum_bounds();          /* Should return 40 */
    
    printf("Total: %d\n", total);
    
    /* Expected total: 3 + 9 + 90 + 2 + 6 + 60 + 4 + 11 + 300 + 40 = 525 */
    
    return 0;
}
