/* Test program to cover expr.cc lines 7691-7700 */
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
    char data[];  /* zero-length array */
};

/* Test functions for different scenarios */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range with designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements - count = 2 */
    
    /* MEM_P target with count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count > 2, size = 1 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars */
    
    /* MEM_P target with count > 2, larger element type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 ints - count > 2, size = 4 */
}

void test_non_mem_target_paths(void) {
    /* Path: non-MEM_P(target) - results in registers */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Register targets from constant array accesses */
    int reg1 = arr[2];                    /* Single element to register */
    int reg2 = arr[3] + arr[4];           /* Sum to register */
    int reg3 = arr[5] * arr[6] - arr[7];  /* Complex expression to register */
    
    /* Conditional with constant indices */
    int cond = 1;
    int reg4 = (cond ? arr[2] : arr[3]);  /* Both branches constant indices */
    
    /* Nested array access with constant inner index */
    int idx_arr[10] = {0};
    int reg5 = arr[idx_arr[2]];  /* Outer index from array, inner constant */
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem1 = vec1[2];      /* Constant index 2 */
    int elem2 = vec2[0];      /* Constant index 0 */
    
    /* Vector operations that might go to registers */
    v4si vec3 = vec1 + vec2;  /* Vector add, result likely in register */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;  /* Element-wise multiplication */
    
    /* Character vectors - small element size */
    v16c char_vec = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                     'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    char c1 = char_vec[5];    /* Constant index */
    char c2 = char_vec[10];   /* Constant index */
}

void test_constant_string_ops(void) {
    /* String literal with constant indices */
    char c1 = "hello world"[2];    /* 'l' - constant index 2 */
    char c2 = "test string"[5];    /* 's' - constant index 5 */
    
    /* String copy with constant bounds */
    char dest[20];
    strcpy(dest, "constant");      /* Whole string copy */
    
    /* Partial string copy */
    char partial[10];
    memcpy(partial, "abcdefghij", 5);  /* Constant count = 5 */
}

void test_struct_array_access(void) {
    /* Array slicing in structs with constant bounds */
    struct ArrayStruct s = {0};
    
    /* Constant start index */
    int *p1 = &s.data[2];      /* Pointer to element 2 */
    char *p2 = &s.buffer[10];  /* Pointer to element 10 */
    
    /* Multiple element access in struct */
    s.data[3] = 100;
    s.data[4] = 200;           /* Two elements */
    
    /* Range in struct array */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        s.data[i] = i * 10;
    }
}

void test_switch_with_const_indices(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int x = 2;
    
    /* Switch with constant array indices */
    switch (x) {
        case 0:
            printf("%d\n", arr[1]);  /* Constant index 1 */
            break;
        case 1:
            printf("%d\n", arr[2]);  /* Constant index 2 */
            break;
        case 2:
            printf("%d\n", arr[3]);  /* Constant index 3 */
            break;
        default:
            printf("%d\n", arr[0]);  /* Constant index 0 */
    }
}

void test_builtin_constant_p(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        arr[5] = 1;
    }
    
    /* Test with constant index */
    if (__builtin_constant_p(arr[2])) {
        arr[2] = 2;
    }
}

void test_mixed_expressions(void) {
    /* Complex expressions combining multiple patterns */
    int arr[20] = {0};
    struct ArrayStruct s = {0};
    
    /* Mixed memory and register targets */
    int val1 = arr[5] + s.data[3];  /* Both MEM_P accesses, result in register */
    
    /* Chain of constant-index accesses */
    arr[arr[2]] = arr[3] + arr[4];  /* Outer index from array, inner constants */
    
    /* Conditional with struct array access */
    int cond = 1;
    s.data[5] = cond ? arr[6] : arr[7];  /* Constant indices in both branches */
    
    /* Loop with constant bounds and array access */
    for (int i = 2; i < 8; i++) {  /* Constant bounds 2-7, count = 6 */
        arr[i] = i * i;
    }
}

int main(void) {
    printf("Testing constant bounds array/vector operations...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_constant_string_ops();
    test_struct_array_access();
    test_switch_with_const_indices();
    test_builtin_constant_p();
    test_mixed_expressions();
    
    printf("All tests completed.\n");
    return 0;
}
