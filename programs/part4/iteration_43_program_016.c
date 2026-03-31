/* test_expr_coverage.c - Test cases for constant-bounded memory operations */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } s = {2, 7, 42};
    
    /* Extract small constant-sized bit-field - should trigger !MEM_P(target) path */
    unsigned int extracted = s.b;  /* 5-bit field extraction */
    return extracted;  /* Returns 7 */
}

/* Test 2: MEM target with count <= 2 - small array initialization */
static int test_small_array_init(void) {
    int arr[5] = {0};
    
    /* Initialize first 2 elements with constants - count = 2 */
    int src[2] = {42, 17};
    
    /* This should be recognized as constant-bounded copy of 2 elements */
    for (int i = 0; i < 2; i++) {
        arr[i] = src[i];
    }
    
    return arr[0] + arr[1];  /* Returns 59 */
}

/* Test 3: MEM target with count = 1 - single element copy */
static int test_single_element_copy(void) {
    struct Point {
        int x, y;
    } p1 = {10, 20}, p2;
    
    /* Copy single structure - count = 1 */
    p2 = p1;
    
    return p2.x + p2.y;  /* Returns 30 */
}

/* Test 4: MEM target with larger count but small total size - char array */
static int test_small_char_array(void) {
    char buffer[20];
    
    /* Initialize 10 chars - count = 10, but total size = 10 bytes */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Also test with explicit bounds */
    char dest[15];
    const int lo = 3;
    const int hi = 12;  /* count = 10 */
    
    /* This should trigger the TYPE_SIZE * count calculation */
    for (int i = lo; i <= hi; i++) {
        dest[i - lo] = (char)(i + '0');
    }
    
    return buffer[0] + dest[0];  /* Returns 'A' + '3' = 65 + 51 = 116 */
}

/* Test 5: MEM target with short array - medium count, small element size */
static int test_short_array(void) {
    short data[8];
    const short src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Copy 8 shorts - count = 8, element size = 2 bytes, total = 16 bytes */
    for (int i = 0; i < 8; i++) {
        data[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += data[i];
    }
    return sum;  /* Returns 36 */
}

/* Test 6: Constant indices with computed bounds */
static int test_constant_indices(void) {
    int array[100];
    
    /* Use enum for constant bounds */
    enum { START = 25, END = 28 };  /* count = 4 */
    
    /* Initialize a slice with constant bounds */
    for (int i = START; i <= END; i++) {
        array[i] = i * 2;
    }
    
    return array[START] + array[END];  /* Returns 50 + 56 = 106 */
}

/* Test 7: Boolean array - very small element size */
static int test_bool_array(void) {
    _Bool flags[32];  /* _Bool is typically 1 byte */
    
    /* Initialize with pattern - count = 32, total size = 32 bytes */
    for (int i = 0; i < 32; i++) {
        flags[i] = (i % 3 == 0);
    }
    
    int count = 0;
    for (int i = 0; i < 32; i++) {
        if (flags[i]) count++;
    }
    return count;  /* Returns ceil(32/3) = 11 */
}

/* Test 8: Pointer array with constant bounds */
static int test_pointer_array(void) {
    const char *strings[5];
    const char *src[5] = {"a", "bb", "ccc", "dddd", "eeeee"};
    
    /* Copy 5 pointers - count = 5, element size = pointer size */
    for (int i = 0; i < 5; i++) {
        strings[i] = src[i];
    }
    
    /* Compute checksum from first chars */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        if (strings[i]) sum += strings[i][0];
    }
    return sum;  /* Returns 'a' + 'b' + 'c' + 'd' + 'e' = 97+98+99+100+101 = 495 */
}

/* Test 9: Nested constant bounds in loop */
static int test_nested_constant_bounds(void) {
    int matrix[4][4];
    
    /* Outer loop with constant bounds */
    for (int row = 0; row < 4; row++) {
        /* Inner loop also constant bounds - each iteration processes 4 elements */
        for (int col = 0; col < 4; col++) {
            matrix[row][col] = row * 10 + col;
        }
    }
    
    return matrix[3][3];  /* Returns 33 */
}

/* Test 10: Mixed types with sizeof for bounds */
static int test_sizeof_bounds(void) {
    typedef struct { char a; int b; } Mixed;
    Mixed items[3];
    
    /* Use sizeof in bounds calculation */
    char buffer[sizeof(Mixed) * 3];
    
    /* Initialize buffer */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (char)(i % 256);
    }
    
    /* Copy to items - compiler knows sizeof(Mixed) is constant */
    memcpy(items, buffer, sizeof(items));
    
    return items[0].a + items[1].a;  /* Returns sum of first two chars */
}

int main(void) {
    int total = 0;
    
    /* Run all test cases */
    total += test_bitfield_extraction();      /* 7 */
    total += test_small_array_init();         /* 59 */
    total += test_single_element_copy();      /* 30 */
    total += test_small_char_array();         /* 116 */
    total += test_short_array();              /* 36 */
    total += test_constant_indices();         /* 106 */
    total += test_bool_array();               /* 11 */
    total += test_pointer_array();            /* 495 */
    total += test_nested_constant_bounds();   /* 33 */
    total += test_sizeof_bounds();            /* varies */
    
    printf("Total: %d\n", total);
    return 0;
}
