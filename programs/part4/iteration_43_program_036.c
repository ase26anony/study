/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count <= 2 */
static int test_small_array_init(void) {
    /* Initialize first 2 elements of an array - count = 2 */
    int arr1[10] = {1, 2};  /* Explicit initialization of 2 elements */
    
    /* Copy 2-element slice - constant bounds */
    int arr2[10];
    arr2[0] = arr1[0];
    arr2[1] = arr1[1];
    
    /* Compound literal with 2 elements */
    int *p = (int[2]){arr1[0], arr1[1]};
    
    return arr1[0] + arr1[1] + arr2[0] + arr2[1] + p[0] + p[1];
}

/* Test 2: MEM target with count > 2 but small total size */
static int test_char_array_ops(void) {
    /* 10 char elements = 10 bytes total */
    char src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char dst[10];
    
    /* Constant-bounded copy of all 10 chars */
    for (int i = 0; i < 10; i++) {
        dst[i] = src[i];
    }
    
    /* Initialize with compound literal */
    char *p = (char[10]){1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i] + p[i];
    }
    return sum;
}

/* Test 3: MEM target with short array (2 bytes each, 3 elements = 6 bytes) */
static int test_short_array(void) {
    short arr1[3] = {100, 200, 300};
    short arr2[3];
    
    /* Constant-bounded copy */
    arr2[0] = arr1[0];
    arr2[1] = arr1[1];
    arr2[2] = arr1[2];
    
    return arr1[0] + arr1[1] + arr1[2] + arr2[0] + arr2[1] + arr2[2];
}

/* Test 4: Non-MEM target (register operations) */
static int test_bitfield_ops(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
    } s = {1, 2, 3};
    
    /* Extract bitfields into registers (non-MEM targets) */
    unsigned int a = s.a;  /* Extracted to register */
    unsigned int b = s.b;
    unsigned int c = s.c;
    
    /* Pack values into integer (register target) */
    unsigned int packed = (a << 8) | (b << 4) | c;
    
    return packed + a + b + c;
}

/* Test 5: Mixed operations with constant indices */
static int test_mixed_const_indices(void) {
    int arr[100];
    
    /* Initialize specific constant-index elements */
    arr[10] = 42;   /* lo_index = 10, hi_index = 10, count = 1 */
    arr[20] = 84;   /* count = 1 */
    arr[30] = 126;  /* count = 1 */
    
    /* Copy between constant indices */
    arr[11] = arr[10];  /* count = 1 */
    arr[21] = arr[20];  /* count = 1 */
    
    return arr[10] + arr[11] + arr[20] + arr[21] + arr[30];
}

/* Test 6: Boolean array (1-bit elements in practice, but byte-sized) */
static int test_bool_array(void) {
    _Bool flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    _Bool copy[8];
    
    /* Copy all 8 bools - total size = 8 bytes */
    for (int i = 0; i < 8; i++) {
        copy[i] = flags[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += copy[i];
    }
    return sum;
}

/* Test 7: Pointer array with constant bounds */
static int test_pointer_array(void) {
    int x = 1, y = 2, z = 3;
    int *ptrs[3] = {&x, &y, &z};
    int *copy[3];
    
    /* Copy 3 pointers - each pointer is word-sized */
    copy[0] = ptrs[0];
    copy[1] = ptrs[1];
    copy[2] = ptrs[2];
    
    return *copy[0] + *copy[1] + *copy[2];
}

/* Test 8: Nested constant loops that might unroll */
static int test_unrolled_loop(void) {
    int src[4] = {10, 20, 30, 40};
    int dst[4] = {0};
    
    /* Loop with constant bound that might unroll */
    for (int i = 0; i < 4; i++) {
        dst[i] = src[i];
    }
    
    return dst[0] + dst[1] + dst[2] + dst[3];
}

/* Test 9: Using enum for constant bounds */
static int test_enum_bounds(void) {
    enum { SIZE = 3 };
    int arr[SIZE] = {100, 200, 300};
    int copy[SIZE];
    
    /* Copy using enum constant */
    for (int i = 0; i < SIZE; i++) {
        copy[i] = arr[i];
    }
    
    return copy[0] + copy[1] + copy[2];
}

/* Test 10: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
    } p1 = {10, 20}, p2;
    
    /* Structure copy - constant size (4 bytes) */
    p2 = p1;
    
    return p2.x + p2.y;
}

int main(void) {
    int total = 0;
    
    total += test_small_array_init();      /* MEM target, count = 2 */
    total += test_char_array_ops();        /* MEM target, 10 chars = 10 bytes */
    total += test_short_array();           /* MEM target, 3 shorts = 6 bytes */
    total += test_bitfield_ops();          /* Non-MEM target */
    total += test_mixed_const_indices();   /* Multiple count=1 operations */
    total += test_bool_array();            /* MEM target, 8 bools = 8 bytes */
    total += test_pointer_array();         /* MEM target, 3 pointers */
    total += test_unrolled_loop();         /* Possibly unrolled loop */
    total += test_enum_bounds();           /* Enum constant bounds */
    total += test_struct_copy();           /* Structure copy */
    
    printf("Result: %d\n", total);
    return 0;
}
