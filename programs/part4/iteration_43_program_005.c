/* Test for GCC expr.cc lines 7691-7700 coverage */
#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
/* This should trigger !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* Extract multiple small bitfields - compiler may use constant-sized operations */
    unsigned int result = 0;
    result |= s.a << 0;
    result |= s.b << 4;
    result |= s.c << 8;
    result |= s.d << 12;
    
    return result;  /* Should be 0x4321 */
}

/* Test 2: MEM target with count <= 2 - small array initialization */
/* This should trigger count <= 2 for MEM target */
static int test_small_array_init(void) {
    int arr[2] = {10, 20};  /* Constant bounds: indices 0 and 1, count = 2 */
    
    /* Force use of the initialized values */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        sum += arr[i];
    }
    return sum;  /* Should be 30 */
}

/* Test 3: MEM target with count <= 2 - two-element struct copy */
static int test_two_element_copy(void) {
    struct Point {
        int x;
        int y;
    };
    
    struct Point p1 = {5, 10};
    struct Point p2;
    
    /* Constant-sized copy of 2 ints */
    p2 = p1;
    
    return p2.x + p2.y;  /* Should be 15 */
}

/* Test 4: MEM target with larger count but small total size */
/* char array with 10 elements: TYPE_SIZE = 8 bits, count = 10, total = 80 bits */
static int test_small_char_array(void) {
    char buffer[10];
    
    /* Initialize with constant values - compiler knows bounds are 0..9 */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum;  /* Sum of 'A' through 'J' */
}

/* Test 5: MEM target with short array - TYPE_SIZE = 16 bits */
static int test_small_short_array(void) {
    short values[4] = {100, 200, 300, 400};  /* count = 4, total = 64 bits */
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += values[i];
    }
    return sum;  /* Should be 1000 */
}

/* Test 6: Constant slice copy within array */
static int test_array_slice_copy(void) {
    int src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int dst[10];
    
    /* Copy slice from index 2 to 5 (inclusive) - count = 4 */
    const int start = 2;
    const int end = 5;
    
    for (int i = start; i <= end; i++) {
        dst[i] = src[i];
    }
    
    /* Verify the copy */
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += dst[i];
    }
    return sum;  /* 2+3+4+5 = 14 */
}

/* Test 7: _Bool array - small element size */
static int test_bool_array(void) {
    _Bool flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};  /* TYPE_SIZE likely 8 bits */
    
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (flags[i]) count++;
    }
    return count;  /* Should be 4 */
}

/* Test 8: Pointer array with constant initialization */
static int test_pointer_array(void) {
    int a = 1, b = 2, c = 3;
    int* ptrs[3] = {&a, &b, &c};  /* count = 3, TYPE_SIZE = pointer size */
    
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += *ptrs[i];
    }
    return sum;  /* 1+2+3 = 6 */
}

/* Test 9: Nested constant loops creating multiple small operations */
static int test_nested_constant_loops(void) {
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* Unrolled operations with constant bounds */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += matrix[i][j];
        }
    }
    return sum;  /* 1+2+3+4+5+6 = 21 */
}

/* Test 10: Using enum for constant bounds */
static int test_enum_bounds(void) {
    enum { SIZE = 3 };
    int arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 10;
    }
    
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
    }
    return sum;  /* 0+10+20 = 30 */
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();      /* Test 1 */
    total += test_small_array_init();         /* Test 2 */
    total += test_two_element_copy();         /* Test 3 */
    total += test_small_char_array();         /* Test 4 */
    total += test_small_short_array();        /* Test 5 */
    total += test_array_slice_copy();         /* Test 6 */
    total += test_bool_array();               /* Test 7 */
    total += test_pointer_array();            /* Test 8 */
    total += test_nested_constant_loops();    /* Test 9 */
    total += test_enum_bounds();              /* Test 10 */
    
    printf("Total: %d\n", total);
    
    /* Expected total calculation:
     * Test 1: 0x4321 = 17185
     * Test 2: 30
     * Test 3: 15
     * Test 4: 'A'+'B'+'C'+'D'+'E'+'F'+'G'+'H'+'I'+'J' = 65+66+67+68+69+70+71+72+73+74 = 695
     * Test 5: 1000
     * Test 6: 14
     * Test 7: 4
     * Test 8: 6
     * Test 9: 21
     * Test 10: 30
     * Total: 17185 + 30 + 15 + 695 + 1000 + 14 + 4 + 6 + 21 + 30 = 20000
     */
    
    return 0;
}
