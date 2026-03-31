/* Test program to exercise constant-bounds array/vector operations in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

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
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Single element with computation */
    
    /* Two-element range in initialization */
    int arr2[10] = {[3] = 1, [4] = 2};  /* count = 2 */
    
    /* Two-element range in struct */
    struct ArrayStruct s1;
    s1.data[5] = 10;
    s1.data[6] = 20;  /* Two consecutive elements */
    
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements, char size = 1 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 elements */
    
    /* Path: MEM_P(target) = true, count > 2, larger element type */
    int arr3[50] = {[10 ... 20] = 99};  /* 11 elements, int size = 4 */
}

void test_non_mem_target_paths(void) {
    /* Path: MEM_P(target) = false (results in registers) */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Register targets from array accesses */
    int reg1 = arr[2];                    /* Single element to register */
    int reg2 = arr[3] + arr[4];           /* Sum to register */
    int reg3 = arr[5] * arr[6] - arr[7];  /* Complex expression to register */
    
    /* Conditional with constant indices */
    int cond = 1;
    int reg4 = (cond ? arr[2] : arr[3]);  /* Both branches constant indices */
    
    /* Nested array access with constant inner index */
    int idx_arr[5] = {2, 3, 4, 1, 0};
    int reg5 = arr[idx_arr[2]];  /* Outer index from array, inner constant */
}

void test_vector_operations(void) {
    /* Vector with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem1 = vec1[2];  /* Constant index 2 */
    int elem2 = vec2[0];  /* Constant index 0 */
    
    /* Vector operations that might go to registers */
    v4si vec3 = vec1 + vec2;  /* Vector add, result likely in register */
    v4si vec4 = vec1 * vec2;  /* Vector multiply */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si vec5 = vec1 & mask;  /* Constant mask operation */
    
    /* Character vector with many elements */
    v16c char_vec = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                     'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    char c1 = char_vec[5];  /* Constant index */
    char c2 = char_vec[10]; /* Constant index */
}

void test_constant_string_ops(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[4];    /* Constant index 4 */
    char c2 = "test string"[0];    /* Constant index 0 */
    
    /* String copy with constant bounds */
    char dest[20];
    strcpy(dest, "constant");  /* String literal with known bounds */
    
    /* Array from string literal */
    char arr[] = "abcdefghij";  /* 11 chars including null */
    char c3 = arr[5];           /* Constant index */
}

void test_loop_unrolling(void) {
    /* Loop with constant bounds that might unroll */
    int arr[10];
    
    /* Small constant loop */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Larger constant loop */
    int sum = 0;
    for (int i = 0; i < 10; i++) {  /* Constant bound 10 */
        sum += arr[i];
    }
}

void test_switch_with_array_indices(void) {
    int arr[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int x = 2;
    
    /* Switch with constant array indices */
    switch (x) {
        case 0:
            printf("%d\n", arr[0]);  /* Constant index 0 */
            break;
        case 1:
            printf("%d\n", arr[1]);  /* Constant index 1 */
            break;
        case 2:
            printf("%d\n", arr[2]);  /* Constant index 2 */
            break;
        case 3:
            printf("%d\n", arr[3]);  /* Constant index 3 */
            break;
        default:
            printf("%d\n", arr[9]);  /* Constant index 9 */
    }
}

void test_builtin_constant_p(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        arr[5] = 100;
    }
    
    /* Test with constant index */
    if (__builtin_constant_p("test"[2])) {
        arr[0] = 1;
    }
}

void test_designated_initializers(void) {
    /* Various designated initializers with constant ranges */
    
    /* Single element */
    int arr1[10] = {[5] = 50};
    
    /* Two-element range */
    int arr2[10] = {[2 ... 3] = 99};
    
    /* Multiple ranges */
    int arr3[20] = {[0 ... 4] = 1, [10 ... 14] = 2, [19] = 3};
    
    /* Character array with ranges */
    char str1[50] = {[10 ... 20] = 'A', [30 ... 35] = 'B'};
    
    /* Struct with array initialization */
    struct ArrayStruct s = {
        .data = {[0 ... 9] = 1, [10 ... 19] = 2},
        .buffer = {[0 ... 24] = 'x', [25 ... 49] = 'y'}
    };
}

void test_pointer_arithmetic_const_bounds(void) {
    struct ArrayStruct s;
    int *p;
    
    /* Pointer to array element with constant offset */
    p = &s.data[5];  /* Constant index 5 */
    *p = 100;
    
    /* Pointer arithmetic with constant bounds */
    p = s.data + 3;  /* Constant offset 3 */
    p[0] = 10;       /* Equivalent to s.data[3] */
    p[1] = 20;       /* Equivalent to s.data[4] - two elements */
    
    /* Multiple pointer dereferences with constant indices */
    int arr[10][10];
    arr[2][3] = 42;   /* 2D array with constant indices */
    arr[5][6] = arr[2][3] + 1;
}

int main(void) {
    printf("Testing constant bounds array/vector operations...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_constant_string_ops();
    test_loop_unrolling();
    test_switch_with_array_indices();
    test_builtin_constant_p();
    test_designated_initializers();
    test_pointer_arithmetic_const_bounds();
    
    printf("All tests completed (coverage occurs during compilation)\n");
    return 0;
}
