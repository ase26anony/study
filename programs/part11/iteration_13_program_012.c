/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* Vector extensions for GCC */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* Structures with arrays */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];  /* Zero-length array */
};

/* Test functions covering different paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still memory target */
    
    /* Two-element range with designated initializer */
    int arr2[10] = {[2] = 10, [3] = 20};  /* Two separate elements */
    
    /* Structure member access - memory target */
    struct ArrayStruct s1;
    s1.data[5] = 100;
    s1.buffer[10] = 'a';
    
    /* Pointer dereference with constant index */
    int *ptr = arr1;
    ptr[2] = 30;  /* Memory target via pointer */
}

void test_large_count_path(void) {
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf[100] = {[10 ... 20] = 'x'};  /* 11 elements, count > 2 */
    
    /* Another large range with char */
    char msg[50] = {[5 ... 15] = '*', [20 ... 30] = '#'};
    
    /* Structure with char array */
    struct ArrayStruct s;
    for (int i = 0; i < 10; i++) {  /* Constant bound 10 */
        s.buffer[i] = '0' + i;
    }
}

void test_register_target_path(void) {
    /* Path: !MEM_P(target) - results likely go to registers */
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Arithmetic with array elements - results in registers */
    int x = arr[2] + arr[3];      /* Register target */
    int y = arr[4] * arr[5];      /* Register target */
    int z = arr[1] - arr[0];      /* Register target */
    
    /* Complex expression with constant indices */
    int result = arr[arr[2]] + (arr[3] > 0 ? arr[4] : arr[5]);
    
    /* Loop with constant bounds - may be unrolled */
    int sum = 0;
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        sum += arr[i];
    }
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem = a[2];  /* Constant index 2 */
    
    /* Vector arithmetic - may use registers */
    v4si c = a + b;
    v4si d = a * b;
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = a * mask;
    
    /* Char vector with many elements */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char ch = chars[10];  /* Constant index */
}

void test_constant_string_indexing(void) {
    /* String literal with constant indices */
    char c1 = "hello world"[4];    /* Constant index 4 */
    char c2 = "test string"[0];    /* Constant index 0 */
    
    /* Array initialized from string */
    char arr[] = "constant string";
    char c3 = arr[7];  /* Constant index 7 */
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int index = 3;
    
    /* Switch with constant array indices in cases */
    switch (index) {
        case 0: arr[1] = 100; break;   /* Constant index 1 */
        case 1: arr[2] = 200; break;   /* Constant index 2 */
        case 2: arr[3] = 300; break;   /* Constant index 3 */
        case 3: arr[4] = 400; break;   /* Constant index 4 */
        default: arr[5] = 500; break;  /* Constant index 5 */
    }
}

void test_nested_and_complex_accesses(void) {
    int arr[10][10];
    
    /* Nested array access with constant indices */
    arr[2][3] = 42;
    arr[4][5] = arr[2][3] + 1;
    
    /* Conditional with array indexing */
    int x = 5;
    int result = (x > 0) ? arr[1][2] : arr[3][4];
    
    /* Multiple array accesses in expression */
    int val = arr[0][1] + arr[2][3] * arr[4][5] - arr[6][7];
}

void test_builtin_constant_p(void) {
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        arr[6] = 99;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(sizeof(arr) / sizeof(arr[0]))) {
        arr[7] = 88;
    }
}

void test_mixed_sized_elements(void) {
    /* Test with different element sizes */
    short shorts[20] = {[5 ... 10] = 255};      /* 6 elements, size 2 */
    char chars[30] = {[0 ... 9] = 'A'};         /* 10 elements, size 1 */
    int ints[15] = {[2 ... 5] = 0xDEADBEEF};    /* 4 elements, size 4 */
    
    /* Mixed in structure */
    struct Mixed {
        char c_arr[20];
        short s_arr[10];
        int i_arr[5];
    } m;
    
    /* Access different sized elements with constant indices */
    m.c_arr[5] = 'X';
    m.s_arr[3] = 1234;
    m.i_arr[2] = 567890;
}

int main(void) {
    /* Execute all test functions to cover different paths */
    test_mem_target_paths();
    test_large_count_path();
    test_register_target_path();
    test_vector_operations();
    test_constant_string_indexing();
    test_switch_with_array_indexing();
    test_nested_and_complex_accesses();
    test_builtin_constant_p();
    test_mixed_sized_elements();
    
    return 0;
}
