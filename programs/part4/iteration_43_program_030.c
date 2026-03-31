/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* This should trigger !MEM_P(target) path */
    unsigned int result = 0;
    result |= s.a << 0;
    result |= s.b << 4;
    result |= s.c << 8;
    result |= s.d << 12;
    
    return result; /* Returns 0x4321 */
}

/* Test 2: MEM target with count <= 2 */
static int test_small_array_init(void) {
    /* Initialize first 2 elements of array - count = 2 */
    int arr[10] = {42, 17};  /* Only first 2 initialized */
    
    /* Also test array slice copy with constant bounds */
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5];
    
    /* Copy 2 elements - constant bounds */
    dst[0] = src[0];
    dst[1] = src[1];
    
    return arr[0] + arr[1] + dst[0] + dst[1]; /* Returns 42 + 17 + 1 + 2 = 62 */
}

/* Test 3: MEM target with count > 2 but small total size */
static int test_char_array_init(void) {
    /* 10 chars = 10 bytes total - should trigger TYPE_SIZE * count path */
    char buffer[10];
    
    /* Initialize with constant values */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Also test with explicit constant bounds copy */
    char src[10] = "ABCDEFGHIJ";
    char dst[10];
    
    /* This copy has constant bounds 0..9 */
    for (int i = 0; i < 10; i++) {
        dst[i] = src[i];
    }
    
    return buffer[0] + dst[5]; /* Returns 'A' + 'F' = 65 + 70 = 135 */
}

/* Test 4: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        short x, y;  /* 2 shorts = 4 bytes total */
    };
    
    struct Point p1 = {100, 200};
    struct Point p2;
    
    /* Structure copy - constant size operation */
    p2 = p1;
    
    return p2.x + p2.y; /* Returns 300 */
}

/* Test 5: Mixed types with constant loop bounds */
static int test_mixed_types(void) {
    /* Use different element types with constant bounds */
    short shorts[3] = {10, 20, 30};
    int ints[2] = {100, 200};
    
    /* Operations with different constant bounds */
    int sum = 0;
    
    /* Loop with constant bound 3 */
    for (int i = 0; i < 3; i++) {
        sum += shorts[i];
    }
    
    /* Loop with constant bound 2 */
    for (int i = 0; i < 2; i++) {
        sum += ints[i];
    }
    
    return sum; /* Returns 10+20+30+100+200 = 360 */
}

/* Test 6: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int *ptr = array;
    
    /* Constant offset operations */
    int a = ptr[0];  /* offset 0 */
    int b = ptr[1];  /* offset 1 */
    int c = ptr[2];  /* offset 2 */
    
    return a + b + c; /* Returns 0 + 1 + 2 = 3 */
}

/* Test 7: Boolean array - very small element size */
static int test_bool_array(void) {
    /* _Bool is typically 1 byte */
    _Bool flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    
    int count = 0;
    /* Constant loop bound 8 */
    for (int i = 0; i < 8; i++) {
        if (flags[i]) count++;
    }
    
    return count; /* Returns 4 */
}

int main(void) {
    int total = 0;
    
    total += test_bitfield_extraction();    /* 0x4321 = 17185 */
    total += test_small_array_init();       /* 62 */
    total += test_char_array_init();        /* 135 */
    total += test_struct_copy();            /* 300 */
    total += test_mixed_types();            /* 360 */
    total += test_pointer_arithmetic();     /* 3 */
    total += test_bool_array();             /* 4 */
    
    /* Expected total: 17185 + 62 + 135 + 300 + 360 + 3 + 4 = 18049 */
    printf("Result: %d\n", total);
    
    return 0;
}
