/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
int test_bitfield_extraction(void) {
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
    
    return result;  /* Should be 0x4321 */
}

/* Test 2: MEM target with count <= 2 - small array initialization */
int test_small_array_init(void) {
    int arr[5];
    
    /* Initialize first 2 elements with constants - count = 2 */
    arr[0] = 42;
    arr[1] = 43;
    
    /* Use both elements to prevent optimization */
    return arr[0] + arr[1];  /* Should be 85 */
}

/* Test 3: MEM target with count <= 2 - two-element copy */
int test_two_element_copy(void) {
    int src[3] = {10, 20, 30};
    int dst[3];
    
    /* Copy first 2 elements - count = 2 */
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = 0;  /* Different from src to ensure copy happened */
    
    return dst[0] + dst[1];  /* Should be 30 */
}

/* Test 4: MEM target with larger count but small total size - char array */
int test_char_array_init(void) {
    char buffer[10];
    
    /* Initialize all 10 chars - count = 10, but element size = 1 byte */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Use the array to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum;  /* Sum of 'A' through 'J' */
}

/* Test 5: MEM target with short array - medium count, small total size */
int test_short_array(void) {
    short data[4];
    
    /* Initialize 4 shorts - count = 4, element size = 2 bytes */
    data[0] = 100;
    data[1] = 200;
    data[2] = 300;
    data[3] = 400;
    
    return data[0] + data[1] + data[2] + data[3];  /* Should be 1000 */
}

/* Test 6: Structure copy with constant size */
int test_struct_copy(void) {
    struct Point {
        int x;
        int y;
    };
    
    struct Point p1 = {5, 10};
    struct Point p2;
    
    /* Structure copy - constant size, count = 2 ints */
    p2 = p1;
    
    return p2.x + p2.y;  /* Should be 15 */
}

/* Test 7: Mixed operations to trigger different paths */
int test_mixed_operations(void) {
    int result = 0;
    
    /* Single element initialization - count = 1 */
    int single[3];
    single[2] = 999;
    result += single[2];
    
    /* Two-element initialization in middle of array */
    int arr[5] = {0};
    arr[1] = 50;  /* index 1 */
    arr[2] = 60;  /* index 2 */
    /* This creates hi=2, lo=1, count=2 */
    result += arr[1] + arr[2];
    
    return result;  /* Should be 1109 */
}

/* Test 8: Using enum for constant bounds */
int test_enum_bounds(void) {
    enum { START = 1, END = 3 };
    int values[5];
    
    /* Constant bounds from enum - count = 3 */
    values[START] = 7;   /* index 1 */
    values[START+1] = 8; /* index 2 */
    values[END] = 9;     /* index 3 */
    
    return values[1] + values[2] + values[3];  /* Should be 24 */
}

/* Test 9: Boolean array - very small element size */
int test_bool_array(void) {
    _Bool flags[16];  /* 16 elements, each potentially 1 byte */
    
    /* Initialize first 8 elements - count = 8 */
    for (int i = 0; i < 8; i++) {
        flags[i] = (i % 2 == 0);
    }
    
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (flags[i]) count++;
    }
    return count;  /* Should be 4 */
}

/* Test 10: Pointer array with small constant initialization */
int test_pointer_array(void) {
    const char *strings[3];
    
    /* Initialize 3 pointers - count = 3, element size = pointer size */
    strings[0] = "Hello";
    strings[1] = "World";
    strings[2] = "Test";
    
    /* Use the pointers to prevent optimization */
    int len_sum = 0;
    for (int i = 0; i < 3; i++) {
        if (strings[i]) len_sum++;
    }
    return len_sum;  /* Should be 3 */
}

int main(void) {
    int total = 0;
    
    total += test_bitfield_extraction();    /* Test 1: Non-MEM target */
    total += test_small_array_init();       /* Test 2: count = 2 */
    total += test_two_element_copy();       /* Test 3: count = 2 copy */
    total += test_char_array_init();        /* Test 4: char[10] */
    total += test_short_array();            /* Test 5: short[4] */
    total += test_struct_copy();            /* Test 6: struct copy */
    total += test_mixed_operations();       /* Test 7: mixed */
    total += test_enum_bounds();            /* Test 8: enum bounds */
    total += test_bool_array();             /* Test 9: _Bool array */
    total += test_pointer_array();          /* Test 10: pointer array */
    
    printf("Total: %d\n", total);
    
    /* Expected total calculation:
     * Test 1: 0x4321 = 17185
     * Test 2: 85
     * Test 3: 30
     * Test 4: 'A'+'B'+...+'J' = 65+66+...+74 = 695
     * Test 5: 1000
     * Test 6: 15
     * Test 7: 1109
     * Test 8: 24
     * Test 9: 4
     * Test 10: 3
     * Total: 17185 + 85 + 30 + 695 + 1000 + 15 + 1109 + 24 + 4 + 3 = 20150
     */
    
    return 0;
}
