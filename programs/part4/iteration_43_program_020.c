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
    
    return result; /* Should be 0x4321 */
}

/* Test 2: MEM target with count <= 2 - small array initialization */
static int test_small_array_init(void) {
    int arr[5];
    
    /* Initialize first 2 elements with constants */
    arr[0] = 42;   /* lo_index = 0, hi_index = 0, count = 1 */
    arr[1] = 43;   /* lo_index = 1, hi_index = 1, count = 1 */
    
    /* Initialize slice of 2 elements */
    int src[3] = {10, 20, 30};
    arr[2] = src[0];  /* count = 1 */
    arr[3] = src[1];  /* count = 1 */
    
    return arr[0] + arr[1] + arr[2] + arr[3]; /* 42 + 43 + 10 + 20 = 115 */
}

/* Test 3: MEM target with count > 2 but small total size - char array */
static int test_char_array_init(void) {
    char buffer[10];
    
    /* Initialize all 10 chars - count = 10, TYPE_SIZE = 8 bits */
    for (int i = 0; i < 10; i++) {
        buffer[i] = 'A' + i;  /* 'A' through 'J' */
    }
    
    /* Also test with explicit constant bounds */
    char dest[5];
    const int start = 0;
    const int end = 4;  /* count = 5 */
    
    for (int i = start; i <= end; i++) {
        dest[i] = buffer[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += dest[i];  /* Sum ASCII values */
    }
    return sum;  /* 'A' + 'B' + 'C' + 'D' + 'E' = 65+66+67+68+69 = 335 */
}

/* Test 4: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
    };
    
    struct Point p1 = {100, 200};
    struct Point p2;
    
    /* This is a constant-sized copy (4 bytes total) */
    p2 = p1;
    
    return p2.x + p2.y;  /* 100 + 200 = 300 */
}

/* Test 5: Mixed types with constant bounds */
static int test_mixed_types(void) {
    /* Array of shorts - each element 16 bits */
    short data[3];
    const int idx1 = 0;
    const int idx2 = 2;  /* count = 3 */
    
    for (int i = idx1; i <= idx2; i++) {
        data[i] = i * 100;
    }
    
    /* Also test _Bool array (typically 8 bits) */
    _Bool flags[8];
    for (int i = 0; i < 8; i++) {
        flags[i] = i % 2;
    }
    
    int sum = data[0] + data[1] + data[2];  /* 0 + 100 + 200 = 300 */
    for (int i = 0; i < 8; i++) {
        sum += flags[i];  /* Add 4 ones */
    }
    return sum;  /* 300 + 4 = 304 */
}

/* Test 6: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int *ptr = array;
    
    /* Access with constant offsets */
    int a = ptr[0];  /* lo_index = 0, hi_index = 0, count = 1 */
    int b = ptr[1];  /* lo_index = 1, hi_index = 1, count = 1 */
    
    /* Small constant range copy */
    int dest[2];
    dest[0] = ptr[2];  /* count = 1 */
    dest[1] = ptr[3];  /* count = 1 */
    
    return a + b + dest[0] + dest[1];  /* 0 + 1 + 2 + 3 = 6 */
}

/* Test 7: Using enum for constant bounds */
static int test_enum_bounds(void) {
    enum { START = 0, END = 3 };  /* count = 4 */
    
    int values[5];
    for (int i = START; i <= END; i++) {
        values[i] = i * 10;
    }
    
    return values[0] + values[1] + values[2] + values[3];  /* 0 + 10 + 20 + 30 = 60 */
}

/* Test 8: Nested constant operations */
static int test_nested_operations(void) {
    /* Compound operation with constant parts */
    struct Small {
        char a;
        char b;
        char c;
    } s1 = {1, 2, 3}, s2;
    
    /* Copy 3 chars - total size 24 bits */
    s2 = s1;
    
    /* Extract to register (!MEM_P target) */
    int combined = (s2.a << 16) | (s2.b << 8) | s2.c;
    
    return combined;  /* 0x010203 */
}

int main(void) {
    int total = 0;
    
    total += test_bitfield_extraction();    /* 0x4321 = 17185 */
    total += test_small_array_init();       /* 115 */
    total += test_char_array_init();        /* 335 */
    total += test_struct_copy();            /* 300 */
    total += test_mixed_types();            /* 304 */
    total += test_pointer_arithmetic();     /* 6 */
    total += test_enum_bounds();            /* 60 */
    total += test_nested_operations();      /* 0x010203 = 66051 */
    
    /* Expected total: 17185 + 115 + 335 + 300 + 304 + 6 + 60 + 66051 = 84356 */
    printf("Result: %d\n", total);
    
    return 0;
}
