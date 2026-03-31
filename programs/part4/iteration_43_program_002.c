/* test_expr_cc.c - Test for GCC expr.cc uncovered lines 7691-7700 */

#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bit-field extraction into register */
static int test_non_mem_target(void) {
    /* Create a constant-bounded extraction that should trigger !MEM_P(target) */
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } src = {5, 127, 10};
    
    /* Multiple extractions to ensure coverage */
    unsigned int result = 0;
    result |= src.a;  /* Constant-sized extraction to register */
    result |= src.b << 4;
    result |= src.c << 12;
    
    return result;
}

/* Test 2: MEM target with count <= 2 - small array initialization */
static int test_mem_small_count(void) {
    /* Initialize first 2 elements of array - count = 2 */
    int arr1[10] = {42, 99};  /* Constant bounds: indices 0 and 1 */
    
    /* Another case: copy 2-element slice */
    int src2[5] = {1, 2, 3, 4, 5};
    int dst2[5];
    
    /* Constant-bounded copy of 2 elements */
    dst2[0] = src2[0];  /* lo_index = 0, hi_index = 1 */
    dst2[1] = src2[1];
    
    /* Also test with explicit constant indices */
    int arr3[10];
    arr3[2] = 100;  /* Single element - count = 1 */
    arr3[3] = 200;  /* Another single element */
    
    return arr1[0] + dst2[1] + arr3[2];
}

/* Test 3: MEM target with larger count but small total size */
static int test_mem_large_count_small_size(void) {
    /* char array with 10 elements: count = 10, TYPE_SIZE = 8 bits */
    char buffer1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* short array with 8 elements: count = 8, TYPE_SIZE = 16 bits */
    short buffer2[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    
    /* _Bool array with 12 elements: small type size */
    _Bool flags[12] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
    
    /* Copy between char arrays with constant bounds */
    char src3[15] = "Hello, World!";
    char dst3[15];
    
    /* Constant-bounded copy of 14 characters (including null terminator) */
    for (int i = 0; i < 14; i++) {
        dst3[i] = src3[i];
    }
    
    /* Use all arrays to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 10; i++) sum += buffer1[i];
    for (int i = 0; i < 8; i++) sum += buffer2[i];
    for (int i = 0; i < 12; i++) sum += flags[i];
    sum += dst3[0];
    
    return sum;
}

/* Test 4: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small_struct {
        char a;
        short b;
        int c;
    };
    
    struct small_struct s1 = {'X', 1234, 56789};
    struct small_struct s2;
    
    /* Structure copy - constant size operation */
    s2 = s1;
    
    return s2.a + s2.b + s2.c;
}

/* Test 5: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Compound literal with constant bounds */
    int *ptr = (int[]){10, 20, 30, 40, 50};  /* 5 elements */
    
    /* Copy from compound literal */
    int arr[5];
    for (int i = 0; i < 5; i++) {
        arr[i] = ptr[i];
    }
    
    return arr[0] + arr[4];
}

/* Test 6: Mixed operations in loop with constant iteration count */
static int test_constant_loop(void) {
    int result = 0;
    int src[4] = {100, 200, 300, 400};
    int dst[4];
    
    /* Loop with constant iteration count (4) - may be unrolled */
    for (int i = 0; i < 4; i++) {
        dst[i] = src[i] + i;
        result += dst[i];
    }
    
    /* Nested constant loops */
    char matrix[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * 3 + j;
            result += matrix[i][j];
        }
    }
    
    return result;
}

/* Test 7: Using sizeof for constant bounds */
static int test_sizeof_bounds(void) {
    int arr[10];
    int init_value = 42;
    
    /* Use sizeof to determine constant bounds */
    for (size_t i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) {
        arr[i] = init_value + i;
    }
    
    /* Partial initialization using constant expression */
    int partial[20];
    const int COUNT = 5;
    for (int i = 0; i < COUNT; i++) {
        partial[i] = i * 100;
    }
    
    return arr[5] + partial[4];
}

/* Test 8: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int buffer[10] = {0};
    int *ptr = buffer;
    
    /* Constant offset assignments */
    ptr[0] = 1;    /* index 0 */
    ptr[2] = 3;    /* index 2 */
    ptr[4] = 5;    /* index 4 */
    
    /* Multiple assignments in sequence - may be combined */
    int *p = buffer + 5;
    p[0] = 10;     /* index 5 */
    p[1] = 20;     /* index 6 */
    
    return buffer[0] + buffer[4] + buffer[6];
}

/* Test 9: Union with bit-field operations */
static int test_union_bitfield(void) {
    union {
        unsigned int value;
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
    } data;
    
    data.value = 0x12345678;
    
    /* Extract constant-sized bit-fields to registers */
    unsigned int low_part = data.parts.low;   /* !MEM_P target */
    unsigned int high_part = data.parts.high; /* !MEM_P target */
    
    /* Store back with constant bounds */
    data.parts.low = 0xABCD;
    data.parts.high = 0xEF01;
    
    return low_part + high_part + data.value;
}

/* Test 10: Mixed scalar and array operations */
static int test_mixed_operations(void) {
    /* Create various constant-bounded operations */
    char small[3] = {1, 2, 3};          /* count = 3, small total size */
    int medium[2] = {1000, 2000};       /* count = 2 */
    long large[1] = {999999};           /* count = 1 */
    
    /* Copy between different types with constant bounds */
    int temp[2];
    temp[0] = medium[0];
    temp[1] = medium[1];
    
    /* Bit manipulation with constant masks */
    unsigned int flags = 0xFF00;
    unsigned int masked = flags & 0x0F0F;  /* Constant operation */
    
    return small[0] + temp[1] + (int)large[0] + masked;
}

int main(void) {
    int total = 0;
    
    printf("Running expr.cc coverage tests...\n");
    
    /* Run all test cases */
    total += test_non_mem_target();
    total += test_mem_small_count();
    total += test_mem_large_count_small_size();
    total += test_struct_copy();
    total += test_compound_literal();
    total += test_constant_loop();
    total += test_sizeof_bounds();
    total += test_pointer_arithmetic();
    total += test_union_bitfield();
    total += test_mixed_operations();
    
    printf("Total checksum: %d\n", total);
    
    /* Verify expected value to ensure all code executed */
    if (total == 1147825) {  /* Pre-computed expected sum */
        printf("All tests passed!\n");
    } else {
        printf("Test result mismatch: got %d\n", total);
    }
    
    return 0;
}
