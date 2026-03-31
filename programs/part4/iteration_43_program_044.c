/* test_expr_coverage.c - Test cases for expr.cc lines 7691-7700 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int sink;

/* Test 1: Non-MEM target - bitfield extraction into register */
/* This should trigger !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* Extract constant-sized bitfield - target is register */
    unsigned int extracted = s.b;  /* Constant bounds: extracting 4 bits */
    return extracted;
}

/* Test 2: MEM target with count = 1 */
static int test_single_element_copy(void) {
    int src[3] = {10, 20, 30};
    int dst[3] = {0};
    
    /* Copy single element with constant bounds */
    dst[1] = src[1];  /* lo_index = 1, hi_index = 1, count = 1 */
    return dst[1];
}

/* Test 3: MEM target with count = 2 */
static int test_two_element_copy(void) {
    short src[5] = {1, 2, 3, 4, 5};
    short dst[5] = {0};
    
    /* Copy two elements with constant bounds */
    dst[0] = src[0];  /* First element */
    dst[1] = src[1];  /* Second element - count = 2 */
    return dst[0] + dst[1];
}

/* Test 4: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    char dst[10] = {0};
    
    /* Copy 10 chars - TYPE_SIZE = 8 bits, count = 10, total = 80 bits */
    for (int i = 0; i < 10; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 5: MEM target with count > 2 but small total size (bool array) */
static int test_bool_array(void) {
    _Bool src[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    _Bool dst[8] = {0};
    
    /* Copy 8 bools - TYPE_SIZE is small, total size is small */
    for (int i = 0; i < 8; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 6: Constant-bounded array slice copy */
static int test_array_slice(void) {
    int arr1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int arr2[10] = {0};
    
    /* Copy slice [2..4] - constant bounds: lo=2, hi=4, count=3 */
    for (int i = 2; i <= 4; i++) {
        arr2[i] = arr1[i];
    }
    
    return arr2[2] + arr2[3] + arr2[4];
}

/* Test 7: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
    } p1 = {100, 200}, p2;
    
    /* Structure copy - constant bounds, count = 2 (shorts) */
    p2 = p1;
    return p2.x + p2.y;
}

/* Test 8: Mixed types with constant indices */
static int test_mixed_types(void) {
    struct Mixed {
        char a;
        int b;
        short c;
    } m1 = {'X', 42, 100}, m2;
    
    /* Copy individual fields with constant offsets */
    m2.a = m1.a;  /* char copy - count = 1 */
    m2.b = m1.b;  /* int copy - count = 1 */
    m2.c = m1.c;  /* short copy - count = 1 */
    
    return m2.a + m2.b + m2.c;
}

/* Test 9: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Initialize array with constant bounds */
    int *arr = (int[]){10, 20, 30, 40};  /* count = 4 */
    return arr[0] + arr[1] + arr[2] + arr[3];
}

/* Test 10: Nested constant-bounded operations */
static int test_nested_operations(void) {
    struct Inner {
        unsigned char data[4];
    };
    
    struct Outer {
        struct Inner parts[2];
    } o1 = {{{1,2,3,4}, {5,6,7,8}}}, o2;
    
    /* Copy constant-sized nested structure */
    o2.parts[0] = o1.parts[0];  /* Copy 4 chars - count = 4 */
    o2.parts[1] = o1.parts[1];  /* Copy 4 chars - count = 4 */
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += o2.parts[0].data[i] + o2.parts[1].data[i];
    }
    return sum;
}

/* Test 11: Using sizeof for constant bounds */
static int test_sizeof_bounds(void) {
    char buffer1[32] = "Hello, World!";
    char buffer2[32] = {0};
    
    /* Copy using sizeof - constant at compile time */
    size_t copy_size = sizeof("Hello, World!");
    for (size_t i = 0; i < copy_size; i++) {
        buffer2[i] = buffer1[i];
    }
    
    return (int)buffer2[0] + (int)buffer2[7];
}

/* Test 12: Bitfield operations with constant bounds */
static int test_bitfield_packing(void) {
    struct Packed {
        unsigned int a : 3;
        unsigned int b : 3;
        unsigned int c : 3;
        unsigned int d : 3;
    } p1 = {3, 5, 2, 7}, p2;
    
    /* Copy individual bitfields - non-MEM targets */
    unsigned int temp = p1.b;  /* Extract to register */
    p2.b = temp;               /* Store from register */
    
    return p2.b;
}

/* Test 13: Small array in union */
static int test_union_array(void) {
    union U {
        int as_int;
        char as_chars[4];
    } u1, u2;
    
    u1.as_int = 0x41424344;  /* "ABCD" in ASCII */
    
    /* Copy char array within union - count = 4 */
    for (int i = 0; i < 4; i++) {
        u2.as_chars[i] = u1.as_chars[i];
    }
    
    return u2.as_int & 0xFF;
}

/* Test 14: Constant loop unrolling opportunity */
static int test_constant_loop(void) {
    int values[3] = {100, 200, 300};
    int result = 0;
    
    /* Loop with constant bounds - may be unrolled */
    for (int i = 0; i < 3; i++) {
        result += values[i];
    }
    
    return result;
}

/* Test 15: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int array[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int *ptr1 = &array[2];
    int *ptr2 = &array[5];
    
    /* Copy with constant offset difference */
    int temp[3];
    for (int i = 0; i < 3; i++) {
        temp[i] = ptr1[i];  /* Copy 3 elements */
    }
    
    return temp[0] + temp[1] + temp[2];
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();      /* Test 1 */
    total += test_single_element_copy();      /* Test 2 */
    total += test_two_element_copy();         /* Test 3 */
    total += test_small_char_array();         /* Test 4 */
    total += test_bool_array();               /* Test 5 */
    total += test_array_slice();              /* Test 6 */
    total += test_struct_copy();              /* Test 7 */
    total += test_mixed_types();              /* Test 8 */
    total += test_compound_literal();         /* Test 9 */
    total += test_nested_operations();        /* Test 10 */
    total += test_sizeof_bounds();            /* Test 11 */
    total += test_bitfield_packing();         /* Test 12 */
    total += test_union_array();              /* Test 13 */
    total += test_constant_loop();            /* Test 14 */
    total += test_pointer_arithmetic();       /* Test 15 */
    
    /* Use sink to prevent dead code elimination */
    sink = total;
    
    printf("Total checksum: %d\n", total);
    return 0;
}
