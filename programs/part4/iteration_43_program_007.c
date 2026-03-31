/* Test for constant-bounded memory operations in expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count <= 2 */
static int test_small_array_init(void) {
    /* Initialize first 2 elements of an array - count = 2 */
    int arr[10] = {0};
    
    /* Constant bounds: lo=0, hi=1, count=2 */
    arr[0] = 1;
    arr[1] = 2;
    
    /* Use result to prevent optimization */
    return arr[0] + arr[1];
}

/* Test 2: MEM target with count > 2 but small total size */
static int test_char_array_init(void) {
    /* 10 chars = 10 bytes total - small enough for scalar expansion */
    char buffer[10];
    
    /* Initialize with constant bounds: lo=0, hi=9, count=10 */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Use result */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum;
}

/* Test 3: Non-MEM target (register operation) */
static int test_bitfield_extract(void) {
    /* Extract constant-sized bitfield into register (!MEM_P(target)) */
    unsigned int value = 0x12345678;
    
    /* Extract bits 8-15 (constant bounds) */
    unsigned int mask = 0xFF00;
    unsigned int extracted = (value & mask) >> 8;
    
    return extracted;
}

/* Test 4: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a, b, c;  /* 3 bytes total */
    };
    
    struct small src = {1, 2, 3};
    struct small dst;
    
    /* Constant-sized copy: sizeof(struct small) = 3 */
    dst = src;
    
    return dst.a + dst.b + dst.c;
}

/* Test 5: Array slice copy with constant bounds */
static int test_array_slice_copy(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};
    
    /* Copy slice with constant bounds: lo=1, hi=3, count=3 */
    for (int i = 1; i <= 3; i++) {
        dst[i] = src[i];
    }
    
    return dst[1] + dst[2] + dst[3];
}

/* Test 6: Mixed types with constant bounds */
static int test_mixed_types(void) {
    short data[4] = {100, 200, 300, 400};
    short result[4] = {0};
    
    /* Constant bounds: lo=0, hi=2, count=3, element size=2 bytes */
    for (int i = 0; i < 3; i++) {
        result[i] = data[i] + 1;
    }
    
    return result[0] + result[1] + result[2];
}

/* Test 7: Boolean array initialization */
static int test_bool_array(void) {
    _Bool flags[8] = {0};
    
    /* Initialize with constant bounds: lo=0, hi=7, count=8 */
    for (int i = 0; i < 8; i++) {
        flags[i] = (i % 2) == 0;
    }
    
    int count = 0;
    for (int i = 0; i < 8; i++) {
        count += flags[i];
    }
    return count;
}

/* Test 8: Pointer array with constant bounds */
static int test_pointer_array(void) {
    int x = 1, y = 2, z = 3;
    int* ptrs[3] = {&x, &y, &z};
    int* copy[3] = {0};
    
    /* Copy with constant bounds: lo=0, hi=2, count=3 */
    for (int i = 0; i < 3; i++) {
        copy[i] = ptrs[i];
    }
    
    return *copy[0] + *copy[1] + *copy[2];
}

/* Test 9: Nested constant bounds through function */
static void init_range(int* arr, int start, int end) {
    /* Function with constant arguments can lead to constant bounds */
    for (int i = start; i <= end; i++) {
        arr[i] = i * 2;
    }
}

static int test_constant_function_args(void) {
    int arr[10] = {0};
    
    /* Call with compile-time constants */
    init_range(arr, 0, 4);  /* lo=0, hi=4, count=5 */
    
    int sum = 0;
    for (int i = 0; i <= 4; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 10: Using sizeof for constant bounds */
static int test_sizeof_bounds(void) {
    char src[32];
    char dst[32];
    
    /* Initialize source */
    for (int i = 0; i < 32; i++) {
        src[i] = (char)(i % 26 + 'A');
    }
    
    /* Copy using sizeof - constant size */
    for (size_t i = 0; i < sizeof(src); i++) {
        dst[i] = src[i];
    }
    
    /* Use result */
    return dst[0] + dst[31];
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_small_array_init();      /* Test 1 */
    total += test_char_array_init();       /* Test 2 */
    total += test_bitfield_extract();      /* Test 3 */
    total += test_struct_copy();           /* Test 4 */
    total += test_array_slice_copy();      /* Test 5 */
    total += test_mixed_types();           /* Test 6 */
    total += test_bool_array();            /* Test 7 */
    total += test_pointer_array();         /* Test 8 */
    total += test_constant_function_args(); /* Test 9 */
    total += test_sizeof_bounds();         /* Test 10 */
    
    printf("Result: %d\n", total);
    return 0;
}
