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
    char data[];  /* zero-length array */
};

/* Test functions covering different paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Memory target with constant index */
    
    /* Two-element range initialization */
    int arr2[10] = {[2] = 100, [3] = 200};  /* Two elements with constant indices */
    
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars, type size = 1 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars */
    
    /* Path: MEM_P(target) true, count > 2, larger element type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 ints, type size = 4 */
}

void test_non_mem_target_paths(void) {
    /* Path: non-MEM_P(target) - results in registers */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Register targets with constant indexing */
    int reg1 = arr[2] + arr[3];      /* Addition result in register */
    int reg2 = arr[4] * arr[5];      /* Multiplication result in register */
    int reg3 = (arr[6] > arr[7]) ? arr[6] : arr[7];  /* Conditional in register */
}

void test_vector_operations(void) {
    /* Vector extensions with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[2];      /* Constant index 2 */
    v4si vec3 = vec1 + vec2;  /* Vector operation */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    
    /* Small element vector */
    v16c char_vec = {[0 ... 15] = 32};
    char_vec[5] = 64;  /* Constant index on char vector */
}

void test_complex_expressions(void) {
    int arr[20] = {[0 ... 19] = 0};
    
    /* Nested array access with constant inner index */
    arr[arr[2]] = 10;  /* arr[2] is 0, so arr[0] = 10 */
    
    /* Multiple constant indices in expression */
    int x = arr[1] + arr[2] * arr[3] - arr[4];
    
    /* Conditional with constant array indices */
    int y = (arr[5] > 0) ? arr[6] : arr[7];
    
    /* String literal with constant indexing */
    char c1 = "constant string"[5];
    char c2 = "another"[2];
}

void test_loop_with_constant_bounds(void) {
    int arr[10];
    
    /* Loop with compile-time constant bound */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Unrolled loop pattern */
    arr[0] = 0;
    arr[1] = 2;
    arr[2] = 4;
    arr[3] = 6;
    arr[4] = 8;
}

void test_struct_array_access(void) {
    struct ArrayStruct s = {0};
    
    /* Constant indexing into struct array member */
    s.data[5] = 100;
    s.data[6] = 200;
    s.data[7] = 300;
    
    /* Pointer to sub-range with constant start */
    int *p = &s.data[3];  /* Constant offset 3 */
    p[0] = 50;            /* Equivalent to s.data[3] */
    p[1] = 60;            /* Equivalent to s.data[4] */
    
    /* Char array in struct with constant range */
    memset(&s.buffer[10], 'A', 5);  /* Constant start, count > 2 */
}

void test_switch_with_array_indices(void) {
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int selector = 2;
    
    /* Switch with constant array indices in cases */
    switch (selector) {
        case 0: arr[1] = 100; break;  /* Constant index 1 */
        case 1: arr[2] = 200; break;  /* Constant index 2 */
        case 2: arr[3] = 300; break;  /* Constant index 3 */
        default: arr[4] = 400; break; /* Constant index 4 */
    }
}

void test_builtin_constant_p(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[2])) {
        arr[3] = 99;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(sizeof(arr))) {
        arr[4] = 88;
    }
}

void test_designated_initializers(void) {
    /* Various constant range initializers */
    int range1[10] = {[2 ... 5] = 42};      /* 4 elements */
    int range2[10] = {[3 ... 4] = 99};      /* 2 elements */
    char range3[100] = {[20 ... 40] = 'Z'}; /* 21 chars, count > 2 */
    
    /* Multiple ranges */
    int multi[20] = {
        [0] = 1,
        [5 ... 8] = 2,    /* 4 elements */
        [10] = 3,
        [15 ... 19] = 4   /* 5 elements */
    };
}

int main(void) {
    printf("Testing constant-bounds array/vector operations...\n");
    
    /* Execute all test patterns */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_complex_expressions();
    test_loop_with_constant_bounds();
    test_struct_array_access();
    test_switch_with_array_indices();
    test_builtin_constant_p();
    test_designated_initializers();
    
    printf("All tests completed (coverage occurs during compilation).\n");
    return 0;
}
