/* Test program to exercise constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

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
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[5] = 99; /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements, char size 1 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 elements */
    
    /* Path: MEM_P(target) true, count > 2, larger element type */
    int arr2[50] = {[20 ... 30] = 100};    /* 11 elements, int size */
    
    /* Designated initializers with constant ranges */
    int arr3[20] = {[2 ... 5] = 42, [10 ... 12] = 99};
}

void test_non_mem_target_paths(void) {
    /* Path: non-MEM_P(target) - results likely go to registers */
    int arr[20] = {1,2,3,4,5,6,7,8,9,10};
    
    /* Register targets with constant indexing */
    int x = arr[2] + arr[3];      /* Both constant indices */
    int y = arr[4] * arr[5];      /* Register results */
    int z = arr[6] - arr[7];
    
    /* Complex expression with register target */
    int result = (arr[8] << 2) | (arr[9] & 0xFF);
    
    /* Conditional with constant indices */
    int cond = 1;
    int w = (cond ? arr[10] : arr[11]);  /* Both branches constant indices */
    
    /* Nested array access with outer constant */
    int nested[10] = {0,1,2,3,4,5,6,7,8,9};
    int idx = nested[3];          /* Constant index 3 */
    int val = arr[idx];           /* Variable index from constant access */
}

void test_vector_operations(void) {
    /* Vector extensions with constant indexing */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem1 = a[0];      /* Constant index 0 */
    int elem2 = a[2];      /* Constant index 2 */
    int elem3 = a[3];      /* Constant index 3 */
    
    /* Vector operations that might use registers */
    v4si c = a + b;        /* Vector add - result likely in register */
    v4si d = a * b;        /* Vector multiply */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = a * mask;  /* Constant mask */
    
    /* Small vector type with many elements */
    v16qi chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = chars[5];    /* Constant index */
    char c2 = chars[10];   /* Constant index */
}

void test_constant_string_indexing(void) {
    /* String literal with constant indices */
    char c1 = "hello world"[0];   /* Constant index 0 */
    char c2 = "hello world"[5];   /* Constant index 5 */
    char c3 = "hello world"[10];  /* Constant index 10 */
    
    /* Array slicing from string */
    const char *str = "constant string";
    char slice1 = str[3];   /* Constant index */
    char slice2 = str[7];   /* Constant index */
}

void test_loop_with_constant_bounds(void) {
    int arr[20];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {   /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Nested loops with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {     /* Constant bound */
        for (int j = 0; j < 5; j++) { /* Constant bound */
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int x = 2;
    
    /* Switch with constant indices in cases */
    switch (x) {
        case 0: 
            x = arr[1];   /* Constant index 1 */
            break;
        case 1:
            x = arr[2];   /* Constant index 2 */
            break;
        case 2:
            x = arr[3];   /* Constant index 3 */
            break;
        case 3:
            x = arr[4];   /* Constant index 4 */
            break;
        default:
            x = arr[0];   /* Constant index 0 */
    }
}

void test_struct_array_access(void) {
    struct ArrayStruct s;
    
    /* Access struct array with constant indices */
    s.data[5] = 100;      /* Constant index 5 */
    s.data[6] = 200;      /* Constant index 6 */
    s.buffer[10] = 'a';   /* Constant index 10 */
    s.buffer[11] = 'b';   /* Constant index 11 */
    s.buffer[12] = 'c';   /* Constant index 12 - count > 2, char type */
    
    /* Pointer to array slice with constant offset */
    int *p1 = &s.data[3];  /* Constant offset 3 */
    char *p2 = &s.buffer[20]; /* Constant offset 20 */
    
    /* Multiple assignments with constant indices */
    s.data[10] = s.data[11] = s.data[12] = 999; /* Three elements */
}

void test_builtin_constant_p(void) {
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        int x = arr[5] * 2;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(arr[9])) {
        int y = arr[9] + 100;
    }
}

void test_mixed_operations(void) {
    /* Combine multiple patterns in complex expressions */
    int arr[20];
    v4si vec = {1,2,3,4};
    
    /* Mixed array and vector operations */
    arr[0] = vec[0] + vec[1];  /* Vector elements to array */
    arr[1] = vec[2] * vec[3];
    
    /* Conditional with mixed constant indices */
    int cond = arr[2] > 0;
    int result = cond ? arr[3] : arr[4];  /* Both constant indices */
    
    /* Nested with constant from array */
    int idx = arr[5];          /* Might be constant if arr[5] known */
    arr[idx] = 42;             /* Variable index */
    
    /* Chain of constant-index accesses */
    int val = arr[arr[6]];     /* Outer index from array element */
}

int main(void) {
    printf("Testing constant bounds array operations...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_constant_string_indexing();
    test_loop_with_constant_bounds();
    test_switch_with_array_indexing();
    test_struct_array_access();
    test_builtin_constant_p();
    test_mixed_operations();
    
    printf("All tests completed (coverage occurs during compilation).\n");
    return 0;
}
