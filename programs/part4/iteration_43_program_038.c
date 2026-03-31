/* Test for GCC expr.cc constant-bounds checking logic */
#include <stdio.h>
#include <string.h>

/* Helper to prevent optimization */
static volatile int sink;

/* Test 1: Non-MEM target - bitfield extraction into register */
int test_non_mem_target(void) {
    unsigned long long value = 0x123456789ABCDEF0ULL;
    
    /* Extract constant-sized bitfield - should trigger !MEM_P(target) path */
    unsigned int extracted = (value >> 8) & 0xFF;  /* Extract byte at offset 8 */
    
    /* Use in computation to prevent dead code elimination */
    return extracted * 2;
}

/* Test 2: MEM target with count <= 2 - small array initialization */
int test_mem_small_count(void) {
    int arr[5] = {0};
    
    /* Initialize first 2 elements with constants - count = 2 */
    arr[0] = 42;
    arr[1] = 43;
    
    /* Also test with single element */
    int single[3];
    single[1] = 99;  /* count = 1 for this store */
    
    return arr[0] + arr[1] + single[1];
}

/* Test 3: MEM target with larger count but small total size - char array */
int test_mem_char_array(void) {
    /* 10 chars = 10 bytes total - should trigger TYPE_SIZE * count calculation */
    char buffer[10];
    
    /* Initialize with constant values */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    
    return sum;
}

/* Test 4: MEM target with short array - medium element size */
int test_mem_short_array(void) {
    /* 4 shorts = 8 bytes total - should also be considered */
    short values[4];
    
    /* Constant initialization */
    values[0] = 100;
    values[1] = 200;
    values[2] = 300;
    values[3] = 400;
    
    return values[0] + values[1] + values[2] + values[3];
}

/* Test 5: Structure copy with constant size */
int test_struct_copy(void) {
    struct SmallStruct {
        char a;
        char b;
        char c;
    };
    
    struct SmallStruct src = {'X', 'Y', 'Z'};
    struct SmallStruct dst;
    
    /* Constant-sized structure copy - 3 bytes total */
    dst = src;
    
    return dst.a + dst.b + dst.c;
}

/* Test 6: Array slice copy with constant bounds */
int test_array_slice(void) {
    int source[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int dest[10] = {0};
    
    /* Copy slice with constant bounds: indices 2 through 5 inclusive */
    /* count = 5 - 2 + 1 = 4 elements */
    for (int i = 2; i <= 5; i++) {
        dest[i] = source[i];
    }
    
    return dest[2] + dest[3] + dest[4] + dest[5];
}

/* Test 7: Boolean array - very small element size */
int test_bool_array(void) {
    _Bool flags[8];  /* Each _Bool is typically 1 byte */
    
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

/* Test 8: Mixed operations to trigger different paths */
int test_mixed(void) {
    /* Test with compile-time constants */
    const int LO = 1;
    const int HI = 3;
    const int COUNT = HI - LO + 1;  /* = 3 */
    
    int data[5] = {10, 20, 30, 40, 50};
    int result[5] = {0};
    
    /* Copy constant slice */
    for (int i = LO; i <= HI; i++) {
        result[i] = data[i];
    }
    
    return result[1] + result[2] + result[3];
}

/* Test 9: Pointer arithmetic with constant offsets */
int test_pointer_arithmetic(void) {
    int array[10] = {0};
    int *ptr = array;
    
    /* Initialize using pointer with constant offsets */
    ptr[0] = 1;  /* count = 1 */
    ptr[1] = 2;  /* count = 1 */
    ptr[2] = 3;  /* count = 1 */
    
    /* Also test with constant index calculation */
    int idx = 5;
    array[idx] = array[idx - 1] + 1;  /* May be optimized to constant bounds */
    
    return array[0] + array[1] + array[2] + array[5];
}

/* Test 10: Nested constant loops */
int test_nested_loops(void) {
    int matrix[2][3] = {{0}};
    
    /* Nested loops with constant bounds */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * 3 + j + 1;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += matrix[i][j];
        }
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_non_mem_target();      /* Should trigger !MEM_P(target) */
    total += test_mem_small_count();     /* Should trigger count <= 2 */
    total += test_mem_char_array();      /* Should trigger TYPE_SIZE * count */
    total += test_mem_short_array();     /* Another TYPE_SIZE * count case */
    total += test_struct_copy();         /* Constant-sized structure copy */
    total += test_array_slice();         /* Array slice with constant bounds */
    total += test_bool_array();          /* Very small element type */
    total += test_mixed();               /* Mixed constant expressions */
    total += test_pointer_arithmetic();  /* Pointer-based constant accesses */
    total += test_nested_loops();        /* Nested constant loops */
    
    /* Use volatile to prevent optimization */
    sink = total;
    
    printf("Result: %d\n", total);
    
    /* Expected result calculation:
       test_non_mem_target: 0xCD * 2 = 0x19A = 410
       test_mem_small_count: 42 + 43 + 99 = 184
       test_mem_char_array: Sum of 'A' to 'J' = 65+66+...+74 = 695
       test_mem_short_array: 100+200+300+400 = 1000
       test_struct_copy: 'X'+'Y'+'Z' = 88+89+90 = 267
       test_array_slice: 2+3+4+5 = 14
       test_bool_array: 4 true values = 4
       test_mixed: 20+30+40 = 90
       test_pointer_arithmetic: 1+2+3+6 = 12
       test_nested_loops: Sum 1..6 = 21
       Total: 410+184+695+1000+267+14+4+90+12+21 = 2697
    */
    
    return 0;
}
