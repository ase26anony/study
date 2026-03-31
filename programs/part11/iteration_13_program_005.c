/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* GCC vector extensions */
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

/* Test functions for different code paths */
void test_mem_p_target_paths(void) {
    /* Path: MEM_P(target) == true with various count values */
    
    /* Single element - count == 1 */
    int arr1[10];
    arr1[3] = 42;  /* Constant index, targets memory */
    
    /* Two elements - count == 2 */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Constant range of 2 elements */
    
    /* Multiple elements - count > 2, small element size */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars, size 1 each */
    
    /* Multiple elements - count > 2, larger element size */
    int arr3[50] = {[5 ... 15] = 99};  /* 11 ints */
    
    /* Structure member access with constant indices */
    struct ArrayStruct s1;
    s1.data[5] = 100;  /* Single element */
    s1.buffer[10] = 'a';  /* Single char */
    
    /* Pointer dereference with constant offset */
    int *ptr1 = &arr1[0];
    ptr1[7] = 88;  /* Constant index through pointer */
}

void test_non_mem_p_target_paths(void) {
    /* Path: MEM_P(target) == false (results in registers) */
    
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Expression results likely go to registers */
    int reg1 = arr[2] + arr[3];  /* Constant indices, result in register */
    int reg2 = arr[4] * arr[5];  /* Multiplication result in register */
    
    /* Conditional with constant indices */
    int cond = 1;
    int reg3 = (cond ? arr[2] : arr[3]);  /* Both branches constant indices */
    
    /* Nested array access with outer constant index */
    int idx_arr[5] = {2,3,4,5,6};
    int reg4 = arr[idx_arr[2]];  /* Outer index constant (2) */
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Vector element access with constant indices */
    int elem1 = vec1[0];  /* Constant index 0 */
    int elem2 = vec1[2];  /* Constant index 2 */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;  /* Result likely in vector register */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si vec4 = vec1 * mask;  /* Constant mask */
    
    /* Char vector with many elements */
    v16c char_vec = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = char_vec[5];  /* Constant index */
    char c2 = char_vec[10]; /* Constant index */
}

void test_constant_bounds_loops(void) {
    /* Loops with constant bounds that might be unrolled */
    int arr[10];
    
    /* Small constant loop - might trigger count <= 2 logic when unrolled */
    for (int i = 0; i < 2; i++) {
        arr[i] = i * 10;
    }
    
    /* Larger constant loop */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 20;
    }
    
    /* Nested loops with constant bounds */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_array_indexing(void) {
    /* Switch statements with array indexing */
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int x = 2;
    
    switch (x) {
        case 0:
            x = arr[1];  /* Constant index 1 */
            break;
        case 1:
            x = arr[2];  /* Constant index 2 */
            break;
        case 2:
            x = arr[3];  /* Constant index 3 */
            break;
        default:
            x = arr[0];  /* Constant index 0 */
    }
}

void test_builtin_constant_p(void) {
    /* Using __builtin_constant_p to force constant evaluation */
    int arr[5] = {1, 2, 3, 4, 5};
    
    if (__builtin_constant_p(arr[2])) {
        /* Force compiler to consider arr[2] as constant */
        int x = arr[2] + 10;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(3) && __builtin_constant_p(7)) {
        /* Both bounds are constants */
        int range[10];
        for (int i = 3; i <= 7; i++) {
            range[i] = i * 2;
        }
    }
}

void test_string_literals(void) {
    /* String literal indexing with constant bounds */
    const char *str = "Hello, World!";
    
    char c1 = str[0];   /* Constant index 0 */
    char c2 = str[7];   /* Constant index 7 */
    char c3 = "Test"[2]; /* Direct string literal indexing */
    
    /* Array initialized with string */
    char arr[] = "Constant";
    char c4 = arr[3];  /* Constant index 3 */
}

void test_designated_initializers(void) {
    /* Various designated initializers with constant ranges */
    
    /* Single element */
    int arr1[10] = {[5] = 100};
    
    /* Range of 2 elements */
    int arr2[10] = {[3 ... 4] = 50};
    
    /* Range > 2 elements with char (small type) */
    char buf1[20] = {[5 ... 10] = 'A'};  /* 6 elements */
    
    /* Range > 2 elements with int */
    int arr3[20] = {[2 ... 8] = 999};  /* 7 elements */
    
    /* Multiple ranges */
    int arr4[30] = {[1 ... 3] = 1, [10 ... 12] = 2, [20 ... 22] = 3};
    
    /* Overlapping ranges (later overwrites earlier) */
    int arr5[10] = {[2 ... 6] = 10, [4 ... 8] = 20};
}

/* Complex expression combining multiple patterns */
int test_complex_expression(void) {
    int arr[20] = {[0 ... 19] = 0};
    
    /* Complex expression with multiple constant-index accesses */
    int result = arr[1] + (arr[2] * arr[3]) - arr[4];
    
    /* Nested with conditional */
    result = (arr[5] > 0) ? arr[6] : arr[7];
    
    /* Chain of array accesses */
    int idx_arr[5] = {2, 4, 6, 8, 10};
    result = arr[idx_arr[2]] + arr[idx_arr[3]];  /* arr[6] + arr[8] */
    
    return result;
}

/* Main function that exercises all test cases */
int main(void) {
    /* Execute all test functions to trigger various code paths */
    test_mem_p_target_paths();
    test_non_mem_p_target_paths();
    test_vector_operations();
    test_constant_bounds_loops();
    test_switch_array_indexing();
    test_builtin_constant_p();
    test_string_literals();
    test_designated_initializers();
    
    int result = test_complex_expression();
    
    /* Use result to prevent dead code elimination */
    return result % 2;
}
