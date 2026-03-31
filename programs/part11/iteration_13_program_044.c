/* Test program to trigger constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Structures with arrays */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];
};

/* Test functions for different code paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Memory to memory with constant index */
    
    /* Two-element range with designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* count = 2 */
    
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars, count > 2 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars */
    
    /* Path: MEM_P(target) true, count > 2, larger element type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 ints, count > 2 */
    short arr4[30] = {[2 ... 8] = 7};  /* 7 shorts, count > 2 */
}

void test_non_mem_target_paths(void) {
    /* Path: non-MEM_P(target) - results in registers */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Register targets with constant indexing */
    int x = arr[2] + arr[3];      /* Both constant indices */
    int y = arr[4] * arr[5];      /* Multiplication in register */
    int z = arr[6] - arr[7];      /* Subtraction */
    
    /* Complex expression favoring register allocation */
    int result = (arr[1] << 2) | (arr[2] & 0xFF);
    
    /* Conditional with constant indices */
    int cond = 1;
    int w = (cond ? arr[2] : arr[3]);  /* Both branches constant indices */
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = a[2];      /* Constant index 2 */
    int elem2 = b[3];      /* Constant index 3 */
    
    /* Vector operations that might use constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si res1 = a * mask;  /* Constant mask */
    v4si res2 = b & mask;
    
    /* Small element vector with many elements */
    v16qi chars = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    char c1 = chars[5];    /* Constant index */
    char c2 = chars[10];   /* Constant index */
    
    /* Medium element vector */
    v8hi shorts = {0,1,2,3,4,5,6,7};
    short s1 = shorts[3];  /* Constant index */
    short s2 = shorts[6];  /* Constant index */
}

void test_constant_string_indexing(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[2];    /* 'l' */
    char c2 = "test string"[5];    /* 's' */
    
    /* Array from string literal */
    const char *str = "constant";
    char c3 = str[3];              /* 's' */
    
    /* Multiple constant string operations */
    int len1 = strlen("short");
    int len2 = strlen("a longer string");
}

void test_nested_and_complex_accesses(void) {
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    int idx_arr[5] = {2,3,4,1,0};
    
    /* Nested array access with outer constant */
    int x1 = arr[arr[2]];      /* arr[2] = 2, so arr[2] = 2 */
    
    /* Nested with inner constant */
    int x2 = idx_arr[arr[3]];  /* arr[3] = 3, so idx_arr[3] = 1 */
    
    /* Multi-dimensional array with constant indices */
    int matrix[3][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};
    int cell = matrix[1][2];   /* Constant indices: 1,2 -> value 7 */
    
    /* Structure with array */
    struct ArrayStruct s;
    s.data[5] = 100;           /* Constant index 5 */
    s.buffer[10] = 'A';        /* Constant index 10 */
    
    /* Pointer to array element with constant offset */
    int *p1 = &s.data[2];      /* Constant offset 2 */
    int *p2 = &arr[7];         /* Constant offset 7 */
}

void test_loops_with_constant_bounds(void) {
    int arr[10];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 2;        /* Constant bound 5 */
    }
    
    /* Nested loops with constant bounds */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i + j;  /* Both bounds constant */
        }
    }
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int x = 2;
    
    /* Switch with constant array indices in cases */
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
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Force constant evaluation with __builtin_constant_p */
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        int x = arr[5] + 1;
    }
    
    /* Check constant index access */
    if (__builtin_constant_p(arr[2])) {
        int y = arr[2] * 2;
    }
}

void test_mixed_operations(void) {
    /* Combine multiple patterns in single expressions */
    int arr[20];
    
    /* Complex expression with multiple constant indices */
    arr[5] = (arr[1] + arr[2]) * (arr[3] - arr[4]);
    
    /* Conditional with different constant indices */
    int cond = arr[0] > 0;
    arr[6] = cond ? arr[7] : arr[8];
    
    /* Chain of constant-index operations */
    arr[9] = arr[10] = arr[11] = 42;
    
    /* Array section copy with constant bounds */
    int src[10] = {0,1,2,3,4,5,6,7,8,9};
    int dst[10];
    
    /* Manual array section copy - might trigger bounds checking */
    for (int i = 2; i <= 7; i++) {  /* Constant bounds: 2 to 7 */
        dst[i] = src[i] * 2;
    }
}

int main(void) {
    printf("Testing constant bounds array operations...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_constant_string_indexing();
    test_nested_and_complex_accesses();
    test_loops_with_constant_bounds();
    test_switch_with_array_indexing();
    test_builtin_constant_p();
    test_mixed_operations();
    
    printf("All tests completed.\n");
    return 0;
}
