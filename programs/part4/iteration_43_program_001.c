/* Test case for expr.cc lines 7691-7700 - constant-bounded memory operations */

#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
/* This should trigger the !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* Extract multiple bit-fields - compiler may use constant-sized operations */
    unsigned int result = (s.a << 12) | (s.b << 8) | (s.c << 4) | s.d;
    return result;
}

/* Test 2: MEM target with count <= 2 - small array initialization */
/* This should trigger count <= 2 for MEM target */
static int test_small_array_init(void) {
    int arr[5];
    
    /* Initialize first 2 elements with constants */
    arr[0] = 42;    /* lo_index = 0, hi_index = 0, count = 1 */
    arr[1] = 43;    /* lo_index = 1, hi_index = 1, count = 1 */
    
    /* Small slice copy - count = 2 */
    int src[2] = {100, 200};
    arr[2] = src[0];
    arr[3] = src[1];
    
    return arr[0] + arr[1] + arr[2] + arr[3];
}

/* Test 3: MEM target with count > 2 but small total size - char array */
/* This should trigger TYPE_SIZE * count calculation */
static int test_char_array(void) {
    char buffer[10];
    
    /* Initialize entire array - count = 10, TYPE_SIZE = 1, total = 10 bytes */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Copy a constant-sized slice - 5 elements */
    char src[5] = {'X', 'Y', 'Z', 'W', 'V'};
    for (int i = 0; i < 5; i++) {
        buffer[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum;
}

/* Test 4: MEM target with short array - medium element size */
static int test_short_array(void) {
    short data[4] = {0};  /* count = 4, TYPE_SIZE = 2, total = 8 bytes */
    
    /* Constant initialization of all elements */
    data[0] = 1000;
    data[1] = 2000;
    data[2] = 3000;
    data[3] = 4000;
    
    /* Copy between arrays with constant bounds */
    short src[3] = {10, 20, 30};
    data[0] = src[0];
    data[1] = src[1];
    data[2] = src[2];
    
    return data[0] + data[1] + data[2] + data[3];
}

/* Test 5: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        int x;
        int y;
    };
    
    struct Point p1 = {10, 20};
    struct Point p2;
    
    /* Structure copy - count = 2 (for int elements), TYPE_SIZE = 4 */
    p2 = p1;
    
    return p2.x + p2.y;
}

/* Test 6: Mixed operations to encourage various expansion strategies */
static int test_mixed_operations(void) {
    int result = 0;
    
    /* Single element operation (count = 1) */
    int single[1];
    single[0] = 999;
    result += single[0];
    
    /* Two element operation (count = 2) */
    int pair[2];
    pair[0] = 100;
    pair[1] = 200;
    result += pair[0] + pair[1];
    
    /* Three element char array (count = 3, TYPE_SIZE = 1) */
    char triple[3];
    triple[0] = 'a';
    triple[1] = 'b';
    triple[2] = 'c';
    result += triple[0] + triple[1] + triple[2];
    
    return result;
}

/* Test 7: Using enums for constant bounds */
static int test_enum_bounds(void) {
    enum { START = 0, END = 3 };
    int arr[5];
    
    /* Constant bounds from enum */
    for (int i = START; i <= END; i++) {
        arr[i] = i * 10;
    }
    
    return arr[0] + arr[1] + arr[2] + arr[3];
}

/* Test 8: Compile-time computable range */
static int test_computable_range(void) {
    const int base = 5;
    const int offset = 2;
    int values[10];
    
    /* Range: base to base+offset (5 to 7) = 3 elements */
    for (int i = base; i <= base + offset; i++) {
        values[i] = i * 100;
    }
    
    return values[5] + values[6] + values[7];
}

/* Test 9: Boolean array - small element size */
static int test_bool_array(void) {
    _Bool flags[8];  /* count = 8, TYPE_SIZE = 1 (typically) */
    
    /* Initialize with pattern */
    for (int i = 0; i < 8; i++) {
        flags[i] = (i % 2) == 0;
    }
    
    int true_count = 0;
    for (int i = 0; i < 8; i++) {
        true_count += flags[i];
    }
    
    return true_count;
}

/* Test 10: Pointer array with constant initialization */
static int test_pointer_array(void) {
    const char *strings[3];
    const char *src[3] = {"hello", "world", "test"};
    
    /* Copy pointer array - count = 3, TYPE_SIZE = sizeof(pointer) */
    for (int i = 0; i < 3; i++) {
        strings[i] = src[i];
    }
    
    /* Use pointers to ensure they're not optimized away */
    int len_sum = 0;
    for (int i = 0; i < 3; i++) {
        if (strings[i]) len_sum += 1;
    }
    
    return len_sum;
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();
    total += test_small_array_init();
    total += test_char_array();
    total += test_short_array();
    total += test_struct_copy();
    total += test_mixed_operations();
    total += test_enum_bounds();
    total += test_computable_range();
    total += test_bool_array();
    total += test_pointer_array();
    
    printf("Total: %d\n", total);
    return 0;
}
