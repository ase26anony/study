/* Test for GCC expr.cc lines 7691-7700 - constant-bounded memory operations */

#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
/* This should trigger the !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } s = { .a = 5, .b = 42, .c = 3 };
    
    /* Extract a constant-sized bit-field into register */
    unsigned int extracted = s.b;  /* 8-bit field extraction */
    
    /* Use the result to prevent optimization */
    return extracted * 2;
}

/* Test 2: MEM target with count = 1 */
/* Single element array initialization */
static int test_single_element(void) {
    int arr[5];
    
    /* Constant bounds: lo=2, hi=2, count=1 */
    int src = 42;
    arr[2] = src;  /* Single element store with constant index */
    
    return arr[2];
}

/* Test 3: MEM target with count = 2 */
/* Two element array initialization */
static int test_two_elements(void) {
    int arr[5];
    
    /* Constant bounds: lo=1, hi=2, count=2 */
    arr[1] = 10;
    arr[2] = 20;  /* Two element stores with constant indices */
    
    return arr[1] + arr[2];
}

/* Test 4: MEM target with count > 2 but small total size */
/* char array with 10 elements = 10 bytes total */
static int test_small_chars(void) {
    char buffer[20];
    
    /* Initialize first 10 chars with constant values */
    /* Constant bounds: lo=0, hi=9, count=10 */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Use the result */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum;
}

/* Test 5: MEM target with short array, count=4, total=8 bytes */
static int test_small_shorts(void) {
    short data[8];
    
    /* Constant bounds: lo=2, hi=5, count=4 */
    for (int i = 2; i <= 5; i++) {
        data[i] = (short)(i * 100);
    }
    
    return data[2] + data[5];
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        int x;
        int y;
    };
    
    struct Point p1 = {10, 20};
    struct Point p2;
    
    /* Constant-sized structure copy (2 ints = 8 bytes typically) */
    p2 = p1;
    
    return p2.x + p2.y;
}

/* Test 7: Array slice copy with constant bounds */
static int test_array_slice(void) {
    int src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int dst[10];
    
    /* Copy slice [3..6] - 4 elements */
    /* Constant bounds: lo=3, hi=6, count=4 */
    for (int i = 3; i <= 6; i++) {
        dst[i] = src[i];
    }
    
    return dst[3] + dst[6];
}

/* Test 8: _Bool array initialization - small element size */
static int test_bool_array(void) {
    _Bool flags[16];  /* Each _Bool is typically 1 byte */
    
    /* Initialize first 8 elements */
    /* Constant bounds: lo=0, hi=7, count=8 */
    for (int i = 0; i < 8; i++) {
        flags[i] = (i % 2 == 0);
    }
    
    int count = 0;
    for (int i = 0; i < 8; i++) {
        count += flags[i];
    }
    return count;
}

/* Test 9: Pointer array with constant bounds */
static int test_pointer_array(void) {
    const char *strings[5];
    const char *temp[5];
    
    /* Initialize with string literals */
    strings[0] = "a";
    strings[1] = "bb";
    strings[2] = "ccc";
    
    /* Copy 3 pointers - constant bounds: lo=0, hi=2, count=3 */
    for (int i = 0; i < 3; i++) {
        temp[i] = strings[i];
    }
    
    /* Use the pointers to prevent optimization */
    int len_sum = 0;
    for (int i = 0; i < 3; i++) {
        if (temp[i]) len_sum++;
    }
    return len_sum;
}

/* Test 10: Nested constant loop that should unroll */
static int test_unrolled_loop(void) {
    int values[4];
    
    /* Small constant loop that compiler might unroll */
    for (int i = 0; i < 4; i++) {
        values[i] = i * i;
    }
    
    return values[0] + values[3];
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();      /* Non-MEM target */
    total += test_single_element();           /* count = 1 */
    total += test_two_elements();             /* count = 2 */
    total += test_small_chars();              /* count = 10, small elements */
    total += test_small_shorts();             /* count = 4, 2-byte elements */
    total += test_struct_copy();              /* structure copy */
    total += test_array_slice();              /* array slice copy */
    total += test_bool_array();               /* _Bool array */
    total += test_pointer_array();            /* pointer array */
    total += test_unrolled_loop();            /* unrolled loop */
    
    printf("Result: %d\n", total);
    return 0;
}
