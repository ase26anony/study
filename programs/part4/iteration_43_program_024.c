/* test_expr_coverage.c - Test cases for GCC expr.cc lines 7691-7700 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: Small array initialization with constant bounds (count <= 2) */
static int test_small_array_init(void) {
    int arr[5];
    
    /* Constant bounds: lo=0, hi=1, count=2 */
    int *p = arr;
    p[0] = 1;
    p[1] = 2;
    
    return arr[0] + arr[1];
}

/* Test 2: Array slice copy with constant bounds (count <= 2) */
static int test_array_slice_copy(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};
    
    /* Constant bounds: lo=2, hi=3, count=2 */
    const int lo = 2;
    const int hi = 3;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[2] + dst[3];
}

/* Test 3: Char array with larger count but small total size */
static int test_char_array_init(void) {
    char buffer[10];
    
    /* Constant bounds: lo=0, hi=9, count=10, but char size is 1 */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum;
}

/* Test 4: Short array with moderate count */
static int test_short_array(void) {
    short data[4];
    
    /* Constant bounds: lo=0, hi=3, count=4, short size is 2 */
    for (int i = 0; i < 4; i++) {
        data[i] = (short)(i * 100);
    }
    
    return data[0] + data[1] + data[2] + data[3];
}

/* Test 5: Structure with small array member */
static int test_struct_with_array(void) {
    struct small {
        char id;
        int values[2];  /* Constant bounds for initialization */
    };
    
    struct small s = { 'X', {100, 200} };
    return s.values[0] + s.values[1] + s.id;
}

/* Test 6: Bit-field extraction into register (!MEM_P(target)) */
static int test_bitfield_extract(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
    } p = {5, 10, 15};
    
    /* Extracting bitfields into registers (non-MEM target) */
    unsigned int x = p.a;
    unsigned int y = p.b;
    unsigned int z = p.c;
    
    return x + y + z;
}

/* Test 7: Compound literal with constant bounds */
static int test_compound_literal(void) {
    /* The compound literal creates a constant-bounded initialization */
    int *ptr = (int[]){1, 2, 3, 4, 5};
    
    /* Access with constant bounds */
    int sum = 0;
    for (int i = 1; i <= 3; i++) {  /* lo=1, hi=3, count=3 */
        sum += ptr[i];
    }
    return sum;
}

/* Test 8: Boolean array (small element size) */
static int test_bool_array(void) {
    _Bool flags[8];
    
    /* Constant bounds: lo=0, hi=7, count=8, _Bool size is 1 */
    for (int i = 0; i < 8; i++) {
        flags[i] = (i % 2 == 0);
    }
    
    int true_count = 0;
    for (int i = 0; i < 8; i++) {
        true_count += flags[i];
    }
    return true_count;
}

/* Test 9: Pointer array with constant bounds */
static int test_pointer_array(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int *ptrs[4];
    
    /* Constant bounds initialization */
    ptrs[0] = &a;
    ptrs[1] = &b;
    ptrs[2] = &c;
    ptrs[3] = &d;
    
    return *ptrs[0] + *ptrs[1] + *ptrs[2] + *ptrs[3];
}

/* Test 10: Nested constant bounds in loop */
static int test_nested_constant_bounds(void) {
    int matrix[3][3];
    int value = 1;
    
    /* Both loops have constant bounds */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = value++;
        }
    }
    
    return matrix[0][0] + matrix[2][2];
}

int main(void) {
    int total = 0;
    
    total += test_small_array_init();      /* Should trigger count <= 2 path */
    total += test_array_slice_copy();      /* Should trigger count <= 2 path */
    total += test_char_array_init();       /* Should trigger TYPE_SIZE * count path */
    total += test_short_array();           /* Should trigger TYPE_SIZE * count path */
    total += test_struct_with_array();     /* Structure with small array */
    total += test_bitfield_extract();      /* Should trigger !MEM_P(target) path */
    total += test_compound_literal();      /* Compound literal with bounds */
    total += test_bool_array();            /* Boolean array with small elements */
    total += test_pointer_array();         /* Pointer array */
    total += test_nested_constant_bounds(); /* Nested constant loops */
    
    printf("Result: %d\n", total);
    return 0;
}
