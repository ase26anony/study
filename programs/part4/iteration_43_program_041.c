/* Test for GCC expr.cc constant-bounds checking logic */
#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
/* This should trigger the !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } s = {2, 13, 42};
    
    /* Extract multiple bitfields - constant bounds for extraction */
    unsigned int result = 0;
    result |= (s.a & 0x7);      /* Constant bounds: bits 0-2 */
    result |= (s.b & 0x1F) << 3; /* Constant bounds: bits 3-7 */
    result |= (s.c & 0xFF) << 8; /* Constant bounds: bits 8-15 */
    
    return result; /* Should be (42<<8) | (13<<3) | 2 = 10762 */
}

/* Test 2: MEM target with count <= 2 */
/* Initialize first 2 elements of array - triggers count <= 2 path */
static int test_small_array_init(void) {
    int arr[10];
    
    /* Constant bounds: indices 0 and 1 */
    arr[0] = 100;
    arr[1] = 200;
    
    return arr[0] + arr[1]; /* 300 */
}

/* Test 3: MEM target with count > 2 but small total size */
/* char array with 10 elements - triggers TYPE_SIZE * count calculation */
static int test_char_array_init(void) {
    char buffer[10];
    
    /* Initialize all 10 chars - constant bounds: indices 0-9 */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Use the buffer to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum; /* Sum of 'A' through 'J' = 10*'A' + sum(0..9) */
}

/* Test 4: Structure copy with constant size */
/* Should trigger constant bounds checking */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
    };
    
    struct Point src = {100, 200};
    struct Point dst;
    
    /* Constant-sized copy (4 bytes total) */
    dst = src;
    
    return dst.x + dst.y; /* 300 */
}

/* Test 5: Array slice copy with constant bounds */
static int test_array_slice_copy(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5];
    
    /* Copy elements 1 through 3 (inclusive) - constant bounds */
    for (int i = 1; i <= 3; i++) {
        dst[i] = src[i];
    }
    
    return dst[1] + dst[2] + dst[3]; /* 20 + 30 + 40 = 90 */
}

/* Test 6: Mixed types with constant bounds */
static int test_mixed_types(void) {
    /* Test with various small types */
    unsigned char bytes[8];
    unsigned short shorts[4];
    
    /* Initialize with constant bounds */
    for (int i = 0; i < 8; i++) {
        bytes[i] = (unsigned char)(i * 10);
    }
    
    for (int i = 0; i < 4; i++) {
        shorts[i] = (unsigned short)(i * 100);
    }
    
    /* Use results */
    int sum = 0;
    sum += bytes[3];  /* 30 */
    sum += shorts[2]; /* 200 */
    
    return sum; /* 230 */
}

/* Test 7: Constant bounds with pointer arithmetic */
static int test_pointer_arithmetic(void) {
    int array[10] = {0};
    int *ptr = array;
    
    /* Initialize using pointer with constant offset */
    ptr[0] = 1;  /* index 0 */
    ptr[1] = 2;  /* index 1 */
    ptr[2] = 3;  /* index 2 */
    
    return array[0] + array[1] + array[2]; /* 6 */
}

/* Test 8: Nested constant bounds in loop */
static int test_nested_constant_bounds(void) {
    int matrix[3][3];
    int sum = 0;
    
    /* Outer loop with constant bounds */
    for (int i = 0; i < 3; i++) {
        /* Inner loop with constant bounds */
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * 3 + j;
            sum += matrix[i][j];
        }
    }
    
    return sum; /* Sum of 0..8 = 36 */
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();    /* Test 1 */
    total += test_small_array_init();       /* Test 2 */
    total += test_char_array_init();        /* Test 3 */
    total += test_struct_copy();            /* Test 4 */
    total += test_array_slice_copy();       /* Test 5 */
    total += test_mixed_types();            /* Test 6 */
    total += test_pointer_arithmetic();     /* Test 7 */
    total += test_nested_constant_bounds(); /* Test 8 */
    
    printf("Total: %d\n", total);
    
    /* Expected total calculation:
     * Test 1: 10762
     * Test 2: 300
     * Test 3: 10*65 + 45 = 695 (A=65, sum 0..9=45)
     * Test 4: 300
     * Test 5: 90
     * Test 6: 230
     * Test 7: 6
     * Test 8: 36
     * Total: 10762 + 300 + 695 + 300 + 90 + 230 + 6 + 36 = 12419
     */
    
    return 0;
}
