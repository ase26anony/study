/* Test for expr.cc constant bounds checking logic */
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
    
    /* Extract multiple bitfields - compiler may use register operations */
    unsigned int result = (s.a << 12) | (s.b << 8) | (s.c << 4) | s.d;
    return result & 0xFFFF;  /* 0x1234 */
}

/* Test 2: MEM target with count <= 2 - small array initialization */
static int test_small_array_init(void) {
    int arr[5];
    
    /* Initialize first 2 elements with constants - count = 2 */
    arr[0] = 42;
    arr[1] = 43;
    
    /* Also test with compound literal copy */
    int src[2] = {100, 200};
    arr[2] = src[0];
    arr[3] = src[1];
    
    return arr[0] + arr[1] + arr[2] + arr[3];  /* 42 + 43 + 100 + 200 = 385 */
}

/* Test 3: MEM target with count > 2 but small total size - char array */
static int test_char_array_copy(void) {
    char dest[10];
    char src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Copy constant-sized region - count = 10, element size = 1 byte */
    for (int i = 0; i < 10; i++) {
        dest[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dest[i];
    }
    return sum;  /* 55 */
}

/* Test 4: Short array with medium count - element size = 2 bytes */
static int test_short_array(void) {
    short arr[8];
    
    /* Initialize with constant indices */
    for (int i = 0; i < 8; i++) {
        arr[i] = i * 10;
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += arr[i];
    }
    return sum;  /* 0 + 10 + 20 + ... + 70 = 280 */
}

/* Test 5: Mixed operations with constant bounds */
static int test_mixed_operations(void) {
    struct Point {
        int x;
        int y;
    } points[3];
    
    /* Initialize with constant bounds */
    for (int i = 0; i < 3; i++) {
        points[i].x = i * 2;
        points[i].y = i * 3;
    }
    
    /* Copy a slice - 2 elements */
    struct Point slice[2];
    slice[0] = points[1];
    slice[1] = points[2];
    
    return points[0].x + points[0].y + 
           slice[0].x + slice[0].y + 
           slice[1].x + slice[1].y;  /* 0 + 0 + 2 + 3 + 4 + 6 = 15 */
}

/* Test 6: Boolean array - very small element size */
static int test_bool_array(void) {
    _Bool flags[16];
    
    /* Initialize with pattern */
    for (int i = 0; i < 16; i++) {
        flags[i] = (i % 3 == 0);
    }
    
    int count = 0;
    for (int i = 0; i < 16; i++) {
        if (flags[i]) count++;
    }
    return count;  /* Values where i % 3 == 0: 0,3,6,9,12,15 = 6 */
}

/* Test 7: Pointer array with constant initialization */
static int test_pointer_array(void) {
    const char *strs[4];
    const char *tmp = "test";
    
    /* Initialize with constant bounds */
    for (int i = 0; i < 4; i++) {
        strs[i] = tmp + i;
    }
    
    /* Use the pointers to avoid optimization */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += (int)(strs[i] - tmp);
    }
    return sum;  /* 0 + 1 + 2 + 3 = 6 */
}

/* Test 8: Nested constant loops creating multiple small operations */
static int test_nested_constant_loops(void) {
    int matrix[2][3];
    
    /* Nested loops with constant bounds */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += matrix[i][j];
        }
    }
    return sum;  /* (0+1+2) + (10+11+12) = 3 + 33 = 36 */
}

int main(void) {
    int total = 0;
    
    total += test_bitfield_extraction();    /* 0x1234 = 4660 */
    total += test_small_array_init();       /* 385 */
    total += test_char_array_copy();        /* 55 */
    total += test_short_array();            /* 280 */
    total += test_mixed_operations();       /* 15 */
    total += test_bool_array();             /* 6 */
    total += test_pointer_array();          /* 6 */
    total += test_nested_constant_loops();  /* 36 */
    
    /* Expected total: 4660 + 385 + 55 + 280 + 15 + 6 + 6 + 36 = 5443 */
    printf("Result: %d\n", total);
    
    return 0;
}
