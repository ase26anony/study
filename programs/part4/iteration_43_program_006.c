/* test_expr_coverage.c - Test cases for GCC expr.cc uncovered lines 7691-7700 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
static int test_bitfield_extraction(void) {
    /* This should trigger !MEM_P(target) path */
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } bf = {5, 127, 9};
    
    /* Extract multiple bitfields - constant bounds for extraction */
    unsigned int extracted = 0;
    extracted |= (bf.a & 0xF);      /* Constant bounds: 4 bits */
    extracted |= (bf.b & 0xFF) << 4; /* Constant bounds: 8 bits */
    extracted |= (bf.c & 0xF) << 12; /* Constant bounds: 4 bits */
    
    return extracted; /* Should be 0x97F5 */
}

/* Test 2: MEM target with count <= 2 - small array initialization */
static int test_small_array_init(void) {
    /* This should trigger count <= 2 for MEM target */
    int arr[5] = {0};
    
    /* Initialize first two elements with constant indices */
    arr[0] = 42;   /* lo_index = 0, hi_index = 0, count = 1 */
    arr[1] = 73;   /* lo_index = 1, hi_index = 1, count = 1 */
    
    /* Initialize slice of 2 elements */
    int src[2] = {100, 200};
    arr[2] = src[0];  /* Could be expanded as two separate stores */
    arr[3] = src[1];
    
    return arr[0] + arr[1] + arr[2] + arr[3];
}

/* Test 3: MEM target with count > 2 but small total size - char array */
static int test_char_array_init(void) {
    /* This should trigger TYPE_SIZE * count calculation */
    /* char size = 1 byte, count = 10, total = 10 bytes */
    char buffer[20] = {0};
    
    /* Initialize a constant-bounded region of 10 chars */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)('A' + i);  /* Constant loop bounds */
    }
    
    /* Also test with explicit constant bounds */
    char data[5] = {1, 2, 3, 4, 5};
    buffer[10] = data[0];  /* Single element */
    buffer[11] = data[1];  /* Single element */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += buffer[i];
    }
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
    
    /* This is a constant-sized copy operation */
    s2 = s1;  /* Total size = 1 + 2 + 4 = 7 bytes (with padding maybe 8) */
    
    return s2.a + s2.b + s2.c;
}

/* Test 5: Array slice with constant bounds */
static int test_array_slice(void) {
    int source[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int dest[10] = {0};
    
    /* Copy slice with constant bounds: indices 2 through 5 inclusive */
    /* lo_index = 2, hi_index = 5, count = 4 */
    for (int i = 2; i <= 5; i++) {
        dest[i] = source[i];
    }
    
    /* Also test with compile-time constant indices */
    dest[0] = source[0];  /* count = 1 */
    dest[1] = source[1];  /* count = 1 */
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dest[i];
    }
    return sum;  /* Should be 0+1+2+3+4+5 = 15 */
}

/* Test 6: Mixed types with small total size */
static int test_mixed_types(void) {
    /* Use _Bool which is typically 1 byte */
    _Bool flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};  /* 8 bytes total */
    
    /* Use short for 2-byte elements */
    short values[4] = {100, 200, 300, 400};  /* 8 bytes total */
    
    int result = 0;
    for (int i = 0; i < 8; i++) {
        result += flags[i];
    }
    for (int i = 0; i < 4; i++) {
        result += values[i];
    }
    
    return result;
}

/* Test 7: Constant-index pointer arithmetic */
static int test_pointer_arithmetic(void) {
    int array[20] = {0};
    
    /* Initialize using pointer with constant offsets */
    int *ptr = array;
    ptr[0] = 10;    /* lo_index = 0 */
    ptr[1] = 20;    /* lo_index = 1 */
    ptr[2] = 30;    /* lo_index = 2 */
    
    /* Another constant-bounded region */
    for (int i = 5; i < 8; i++) {  /* lo_index = 5, hi_index = 7, count = 3 */
        array[i] = i * 10;
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += array[i];
    }
    return sum;
}

/* Test 8: Using enums for constant bounds */
static int test_enum_bounds(void) {
    enum { START = 3, END = 7, COUNT = END - START + 1 };
    
    int data[10] = {0};
    
    /* Constant bounds from enum */
    for (int i = START; i <= END; i++) {
        data[i] = i * 2;
    }
    
    int sum = 0;
    for (int i = START; i <= END; i++) {
        sum += data[i];
    }
    return sum;  /* 3*2 + 4*2 + 5*2 + 6*2 + 7*2 = 50 */
}

/* Test 9: Nested constant loops */
static int test_nested_loops(void) {
    int matrix[3][3] = {{0}};
    
    /* Nested loops with constant bounds */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * 3 + j;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            sum += matrix[i][j];
        }
    }
    return sum;  /* 0+1+2+3+4+5+6+7+8 = 36 */
}

/* Test 10: sizeof-based constant bounds */
static int test_sizeof_bounds(void) {
    char buffer[100];
    
    /* Use sizeof to get constant bounds */
    const int chunk_size = sizeof(int) * 2;  /* 8 bytes typically */
    
    /* Initialize first chunk_size bytes */
    for (int i = 0; i < chunk_size; i++) {
        buffer[i] = (char)(i + 1);
    }
    
    int sum = 0;
    for (int i = 0; i < chunk_size; i++) {
        sum += buffer[i];
    }
    return sum;  /* 1+2+3+4+5+6+7+8 = 36 */
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();      /* Test 1 */
    total += test_small_array_init();         /* Test 2 */
    total += test_char_array_init();          /* Test 3 */
    total += test_struct_copy();              /* Test 4 */
    total += test_array_slice();              /* Test 5 */
    total += test_mixed_types();              /* Test 6 */
    total += test_pointer_arithmetic();       /* Test 7 */
    total += test_enum_bounds();              /* Test 8 */
    total += test_nested_loops();             /* Test 9 */
    total += test_sizeof_bounds();            /* Test 10 */
    
    printf("Total checksum: %d\n", total);
    
    /* Expected total calculation:
       Test 1: 0x97F5 = 38885
       Test 2: 42 + 73 + 100 + 200 = 415
       Test 3: Sum of 'A' to 'J' (65-74) + 1 + 2 = 65+66+...+74 + 3 = 695 + 3 = 698
       Test 4: 'X' (88) + 1234 + 56789 = 58111
       Test 5: 0+1+2+3+4+5 = 15
       Test 6: 4 (true flags) + 100+200+300+400 = 1004
       Test 7: 10+20+30 + 50+60+70 = 240
       Test 8: 50
       Test 9: 36
       Test 10: 36
       Total: 38885 + 415 + 698 + 58111 + 15 + 1004 + 240 + 50 + 36 + 36 = 100490
    */
    
    return 0;
}
