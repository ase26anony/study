/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_single_element_copy(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element at index 2 */
    const int lo = 2;
    const int hi = 2;
    dst[lo] = src[lo];  /* count = hi - lo + 1 = 1 */
    
    return dst[2];  /* Should be 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_two_element_copy(void) {
    short src[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    short dst[10] = {0};
    
    /* Constant bounds: copy two elements starting at index 3 */
    const int start = 3;
    const int end = 4;
    for (int i = start; i <= end; i++) {  /* Compiler should see constant bounds */
        dst[i] = src[i];
    }
    
    return dst[3] + dst[4];  /* Should be 40 + 50 = 90 */
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[15] = "Hello, World!";
    char dst[15] = {0};
    
    /* Constant bounds: copy 13 characters (indices 0-12) */
    const int lo = 0;
    const int hi = 12;  /* strlen("Hello, World!") - 1 */
    
    /* This should trigger TYPE_SIZE * count calculation */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return (int)dst[7];  /* ASCII 'W' = 87 */
}

/* Test 4: MEM target with small struct array */
static int test_small_struct_array(void) {
    struct point { char x; char y; } src[4] = {{1,2}, {3,4}, {5,6}, {7,8}};
    struct point dst[4] = {{0,0}};
    
    /* Constant bounds: copy 3 elements (indices 0-2) */
    const int lo = 0;
    const int hi = 2;  /* count = 3, element size = 2 bytes, total = 6 bytes */
    
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[1].x + dst[2].y;  /* 3 + 6 = 9 */
}

/* Test 5: Non-MEM target (register operation) - bitfield extraction */
static int test_bitfield_extraction(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } data = {.a = 5, .b = 10, .c = 15, .d = 0};
    
    /* Extract multiple bitfields into registers (non-MEM targets) */
    unsigned int result = 0;
    result |= data.a << 0;   /* Extracted to register */
    result |= data.b << 4;   /* Extracted to register */
    result |= data.c << 8;   /* Extracted to register */
    
    return result;  /* Should be 5 | (10<<4) | (15<<8) = 5 + 160 + 3840 = 4005 */
}

/* Test 6: Constant loop unrolling creates multiple small operations */
static int test_constant_loop_unroll(void) {
    int values[4] = {0};
    
    /* Constant loop - should unroll into individual assignments */
    for (int i = 0; i < 3; i++) {  /* 3 iterations, constant bound */
        values[i] = (i + 1) * 10;
    }
    
    return values[0] + values[1] + values[2];  /* 10 + 20 + 30 = 60 */
}

/* Test 7: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Direct initialization with constant bounds */
    int arr[5] = { [1] = 100, [2] = 200, [3] = 300 };
    
    return arr[1] + arr[2] + arr[3];  /* 100 + 200 + 300 = 600 */
}

/* Test 8: Memory copy with constant offset using memcpy */
static int test_const_memcpy(void) {
    char buffer[20] = {0};
    const char *src = "Constant";
    
    /* memcpy with constant size - compiler may lower to inline copy */
    memcpy(buffer + 2, src, 8);  /* 8 bytes at constant offset */
    
    return (int)buffer[2] + (int)buffer[9];  /* 'C' + 't' = 67 + 116 = 183 */
}

/* Test 9: Mixed types to test different element sizes */
static int test_mixed_types(void) {
    /* Array of shorts - element size = 2 bytes */
    short shorts[8] = {0};
    const int s_lo = 1;
    const int s_hi = 4;  /* count = 4, total = 8 bytes */
    
    for (int i = s_lo; i <= s_hi; i++) {
        shorts[i] = i * 100;
    }
    
    /* Array of chars - element size = 1 byte */
    char chars[10] = {0};
    const int c_lo = 0;
    const int c_hi = 7;  /* count = 8, total = 8 bytes */
    
    for (int i = c_lo; i <= c_hi; i++) {
        chars[i] = 'A' + i;
    }
    
    return shorts[2] + chars[3];  /* 200 + 'D' = 200 + 68 = 268 */
}

int main(void) {
    int total = 0;
    
    total += test_single_element_copy();      /* +3 */
    total += test_two_element_copy();         /* +90 */
    total += test_small_char_array();         /* +87 */
    total += test_small_struct_array();       /* +9 */
    total += test_bitfield_extraction();      /* +4005 */
    total += test_constant_loop_unroll();     /* +60 */
    total += test_compound_literal();         /* +600 */
    total += test_const_memcpy();             /* +183 */
    total += test_mixed_types();              /* +268 */
    
    printf("Result: %d\n", total);  /* Expected: 3+90+87+9+4005+60+600+183+268 = 5305 */
    return 0;
}
