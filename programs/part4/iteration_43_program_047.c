/* test_expr_coverage.c - Test cases for GCC expr.cc constant bounds checking */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
/* This should trigger the !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } s = {5, 127, 10};
    
    /* Extract multiple bit-fields - compiler may use constant bounds */
    unsigned int result = 0;
    result |= (s.a << 0);   /* Extract first 4 bits */
    result |= (s.b << 4);   /* Extract next 8 bits */
    result |= (s.c << 12);  /* Extract last 4 bits */
    
    return result;  /* Should be: 5 | (127 << 4) | (10 << 12) = 5 + 2032 + 40960 = 42997 */
}

/* Test 2: MEM target with count <= 2 - small array initialization */
/* This should trigger count <= 2 for MEM target */
static int test_small_array_init(void) {
    int arr[5];
    
    /* Initialize first 2 elements with constant indices */
    arr[0] = 42;      /* lo_index = 0, hi_index = 0, count = 1 */
    arr[1] = 43;      /* lo_index = 1, hi_index = 1, count = 1 */
    
    /* Initialize slice of 2 elements */
    int src[3] = {100, 200, 300};
    arr[2] = src[0];  /* Single element copy */
    arr[3] = src[1];  /* Single element copy */
    
    return arr[0] + arr[1] + arr[2] + arr[3];  /* 42 + 43 + 100 + 200 = 385 */
}

/* Test 3: MEM target with count > 2 but small total size - char array */
/* This should trigger TYPE_SIZE * count calculation */
static int test_small_char_array(void) {
    char buffer[10];
    
    /* Initialize with constant bounds - count = 10, TYPE_SIZE = 1 */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Copy a constant-sized slice */
    char src[10] = "ABCDEFGHIJ";
    char dest[10];
    
    /* This copy has constant bounds: lo=0, hi=9, count=10 */
    for (int i = 0; i < 10; i++) {
        dest[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dest[i];
    }
    return sum;  /* Sum of ASCII values A-J: 65+66+...+74 = 695 */
}

/* Test 4: MEM target with short array - medium element size */
static int test_short_array(void) {
    short data[4];
    
    /* Initialize with constant bounds - count = 4, TYPE_SIZE = 2 */
    data[0] = 1000;
    data[1] = 2000;
    data[2] = 3000;
    data[3] = 4000;
    
    /* Copy between arrays with constant indices */
    short copy[4];
    for (int i = 0; i < 4; i++) {
        copy[i] = data[i];
    }
    
    return copy[0] + copy[1] + copy[2] + copy[3];  /* 10000 */
}

/* Test 5: Mixed operations with compile-time constants */
static int test_mixed_constants(void) {
    const int LO = 2;
    const int HI = 5;
    const int COUNT = HI - LO + 1;  /* 4 */
    
    int array[10];
    
    /* Initialize slice with constant bounds */
    for (int i = LO; i <= HI; i++) {
        array[i] = i * 10;
    }
    
    /* Copy slice to another location */
    int dest[10];
    for (int i = LO; i <= HI; i++) {
        dest[i] = array[i];
    }
    
    return dest[2] + dest[3] + dest[4] + dest[5];  /* 20 + 30 + 40 + 50 = 140 */
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        int x;
        int y;
        char label[3];  /* Small char array inside struct */
    };
    
    struct Point p1 = {10, 20, {'A', 'B', 'C'}};
    struct Point p2;
    
    /* Structure copy - constant size (2 ints + 3 chars = 11+ bytes padded) */
    p2 = p1;
    
    return p2.x + p2.y + p2.label[0] + p2.label[1] + p2.label[2];  /* 10 + 20 + 65 + 66 + 67 = 228 */
}

/* Test 7: Boolean array - very small element size */
static int test_bool_array(void) {
    _Bool flags[16];  /* TYPE_SIZE = 1 (usually) */
    
    /* Initialize with constant bounds - count = 16 */
    for (int i = 0; i < 16; i++) {
        flags[i] = (i % 3 == 0);
    }
    
    /* Copy to another array */
    _Bool copy[16];
    for (int i = 0; i < 16; i++) {
        copy[i] = flags[i];
    }
    
    int true_count = 0;
    for (int i = 0; i < 16; i++) {
        true_count += copy[i];
    }
    return true_count;  /* ceil(16/3) = 6 */
}

/* Test 8: Pointer array with constant indices */
static int test_pointer_array(void) {
    const char *strings[4];
    const char *temp[4];
    
    strings[0] = "Hello";
    strings[1] = "World";
    strings[2] = "Test";
    strings[3] = "Case";
    
    /* Copy with constant bounds */
    for (int i = 0; i < 4; i++) {
        temp[i] = strings[i];
    }
    
    /* Compute checksum from first chars */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        if (temp[i]) sum += temp[i][0];
    }
    return sum;  /* 'H' + 'W' + 'T' + 'C' = 72 + 87 + 84 + 67 = 310 */
}

/* Test 9: Nested constant bounds in loop */
static int test_nested_constant_loop(void) {
    int matrix[3][3];
    int result = 0;
    
    /* Outer loop with constant bounds */
    for (int row = 0; row < 3; row++) {
        /* Inner loop with constant bounds - each iteration creates
           a constant-bounded memory operation */
        for (int col = 0; col < 3; col++) {
            matrix[row][col] = row * 10 + col;
        }
        
        /* Copy row with constant bounds */
        int row_copy[3];
        for (int col = 0; col < 3; col++) {
            row_copy[col] = matrix[row][col];
            result += row_copy[col];
        }
    }
    
    return result;  /* Sum of 0..2, 10..12, 20..22 = 99 */
}

/* Test 10: Using sizeof for constant bounds */
static int test_sizeof_bounds(void) {
    typedef int small_array[5];
    small_array data = {1, 2, 3, 4, 5};
    small_array copy;
    
    /* Use sizeof to get constant bounds */
    int elements = sizeof(data) / sizeof(data[0]);  /* 5 */
    
    /* Copy using compile-time known size */
    for (int i = 0; i < elements; i++) {
        copy[i] = data[i];
    }
    
    int sum = 0;
    for (int i = 0; i < elements; i++) {
        sum += copy[i];
    }
    return sum;  /* 15 */
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();    /* Test 1 */
    total += test_small_array_init();       /* Test 2 */
    total += test_small_char_array();       /* Test 3 */
    total += test_short_array();            /* Test 4 */
    total += test_mixed_constants();        /* Test 5 */
    total += test_struct_copy();            /* Test 6 */
    total += test_bool_array();             /* Test 7 */
    total += test_pointer_array();          /* Test 8 */
    total += test_nested_constant_loop();   /* Test 9 */
    total += test_sizeof_bounds();          /* Test 10 */
    
    printf("Total checksum: %d\n", total);
    
    /* Expected total:
       42997 + 385 + 695 + 10000 + 140 + 228 + 6 + 310 + 99 + 15 = 54875
    */
    
    return 0;
}
