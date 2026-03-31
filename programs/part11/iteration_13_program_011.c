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

/* Test functions to cover different paths */
void test_mem_target_paths(void) {
    /* Path 1: MEM_P(target) true with count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range in designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements - count = 2 */
    
    /* Path 2: MEM_P(target) true with count > 2 and type size calculation */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count = 11, eltsize = 1 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars - same pattern */
    
    /* Larger element type with count > 2 */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 ints - count = 6, eltsize = 4 */
}

void test_non_mem_target_paths(void) {
    /* Path 3: Non-MEM_P(target) - results in registers */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Register results from constant indexing */
    int reg1 = arr[2] + arr[3];      /* Both constant indices */
    int reg2 = arr[4] * arr[5];      /* Multiplication in register */
    int reg3 = (arr[6] << 2) | arr[7]; /* Bit operations in register */
    
    /* Conditional with constant indices */
    int cond = 1;
    int reg4 = (cond ? arr[2] : arr[3]);  /* Both branches constant indices */
    
    /* Complex expression with nested constant access */
    int reg5 = arr[arr[2]];  /* Outer index from array, inner constant */
}

void test_vector_operations(void) {
    /* Vector extensions with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[2];  /* Constant index 2 */
    int elem2 = vec2[1];  /* Constant index 1 */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;  /* Vector addition */
    v4si vec4 = vec1 * vec2;  /* Vector multiplication */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 & mask;  /* Constant mask operation */
    
    /* Character vector with many elements */
    v16c char_vec = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = char_vec[5];  /* Constant index */
    char c2 = char_vec[10]; /* Constant index */
}

void test_constant_bounds_loops(void) {
    int arr[10];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Nested loops with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {      /* Constant bound */
        for (int j = 0; j < 5; j++) {  /* Constant bound */
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_array_indexing(void) {
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int x = 2;
    
    /* Switch with constant array indices */
    switch (x) {
        case 0: arr[1] = 100; break;  /* Constant index 1 */
        case 1: arr[2] = 200; break;  /* Constant index 2 */
        case 2: arr[3] = 300; break;  /* Constant index 3 */
        default: arr[4] = 400; break; /* Constant index 4 */
    }
}

void test_string_literals(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[4];    /* Constant index 4 = 'o' */
    char c2 = "constant"[2];       /* Constant index 2 = 'n' */
    
    /* Array initialized from string literal */
    char strarr[] = "test string";
    char c3 = strarr[5];  /* Constant index 5 = 's' */
}

void test_struct_array_access(void) {
    struct ArrayStruct s;
    
    /* Access array within struct with constant bounds */
    s.data[5] = 100;           /* Single element */
    s.buffer[10] = 'A';        /* Single char element */
    
    /* Pointer to sub-range with constant start */
    int *p1 = &s.data[3];      /* Constant offset 3 */
    char *p2 = &s.buffer[20];  /* Constant offset 20 */
    
    /* Multiple element access in struct array */
    s.data[6] = s.data[5] + s.data[4];  /* Multiple constant indices */
}

void test_builtin_constant(void) {
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Force constant evaluation with __builtin_constant_p */
    if (__builtin_constant_p(arr[5])) {
        arr[6] = 99;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(5)) {
        arr[7] = arr[5] + 1;
    }
}

/* Main function combining all tests */
int main(void) {
    /* Execute all test patterns to cover different compiler paths */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_constant_bounds_loops();
    test_switch_array_indexing();
    test_string_literals();
    test_struct_array_access();
    test_builtin_constant();
    
    /* Additional complex expressions in main */
    int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    
    /* Multi-dimensional constant indexing */
    int val1 = matrix[1][2];  /* Constant indices 1,2 */
    
    /* Pointer arithmetic with constant bounds */
    int *ptr = &matrix[0][0];
    int val2 = *(ptr + 4);    /* Constant offset 4 */
    
    /* Conditional with array indexing on both sides */
    int a = 1, b = 2;
    int result = (a < b) ? matrix[0][1] : matrix[2][0];
    
    /* Nested array access with constant intermediate */
    int arr[5] = {10, 20, 30, 40, 50};
    int idx = 2;
    int nested = arr[arr[idx] / 10];  /* arr[30/10] = arr[3] */
    
    return 0;
}
