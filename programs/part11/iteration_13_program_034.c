/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* Vector extension types */
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

/* Test functions to cover different paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[5] = 99; /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars, size 1 each */
    char buf2[50] = {[5 ... 15] = 0};      /* 11 elements */
    
    /* Path: MEM_P(target) = true, count > 2, larger element type */
    int arr2[50] = {[20 ... 30] = 255};    /* 11 ints */
    short arr3[100] = {[40 ... 50] = 1};   /* 11 shorts */
}

void test_non_mem_target_paths(void) {
    /* Path: MEM_P(target) = false (results in registers) */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Results likely go to registers */
    int reg1 = arr[2] + arr[3];      /* Constant indices 2, 3 */
    int reg2 = arr[4] * arr[5];      /* Constant indices 4, 5 */
    int reg3 = (arr[6] > arr[7]) ? arr[6] : arr[7]; /* Conditional with constants */
    
    /* Complex expression with multiple constant accesses */
    int reg4 = arr[arr[1]] + arr[arr[2]]; /* Nested constant indexing */
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem1 = vec1[0];  /* Constant index 0 */
    int elem2 = vec1[2];  /* Constant index 2 */
    
    /* Vector operations that might expand with constant bounds */
    v4si vec3 = vec1 + vec2;
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;  /* Constant mask vector */
    
    /* Character vector with many elements */
    v16qi char_vec = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = char_vec[5];  /* Constant index 5 */
    char c2 = char_vec[10]; /* Constant index 10 */
}

void test_struct_operations(void) {
    struct ArrayStruct s1;
    
    /* Constant indexing into struct array members */
    s1.data[5] = 100;      /* Single element */
    s1.data[6] = s1.data[7] = 200;  /* Two elements */
    s1.buffer[10] = 'A';   /* Single char */
    
    /* Pointer to sub-range with constant start */
    int *p1 = &s1.data[2];  /* Constant offset 2 */
    char *p2 = &s1.buffer[5]; /* Constant offset 5 */
    
    /* Designated initializer with constant range */
    int arr_range[10] = {[2 ... 5] = 42, [7 ... 9] = 99};
}

void test_constant_string_indexing(void) {
    /* String literal with constant indices */
    char c1 = "Hello World!"[0];  /* 'H' */
    char c2 = "Hello World!"[6];  /* 'W' */
    char c3 = "Constant"[3];      /* 's' */
    
    /* Multiple constant accesses in expression */
    int sum = "abc"[0] + "def"[1] + "ghi"[2];
}

void test_loop_with_constant_bounds(void) {
    int arr[10];
    
    /* Loop with constant bound - may be unrolled */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 2;
    }
    
    /* Nested loop with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_with_array_indexing(int x) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    
    /* Switch with constant array indices in cases */
    switch (x) {
        case 0: arr[1] = 100; break;
        case 1: arr[2] = 200; break;
        case 2: arr[3] = 300; break;
        case 3: arr[4] = 400; break;
        default: arr[5] = 500;
    }
}

void test_builtin_constant_p(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        arr[6] = 1;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(arr[7] - arr[8])) {
        arr[9] = 2;
    }
}

void test_mixed_operations(void) {
    /* Combine multiple patterns in complex expressions */
    struct ArrayStruct s;
    v4si vec = {1, 2, 3, 4};
    
    /* Mixed struct, array, and vector operations */
    s.data[vec[0]] = vec[1] + s.data[2];
    
    /* Conditional with different constant index paths */
    int arr[20];
    int idx = 5;
    int val = (idx < 10) ? arr[5] : arr[15];
    
    /* Chain of constant-index operations */
    arr[1] = arr[2] = arr[3] = 42;
}

/* Main function that exercises all test cases */
int main(void) {
    /* Execute all test functions to trigger different code paths */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_struct_operations();
    test_constant_string_indexing();
    test_loop_with_constant_bounds();
    test_switch_with_array_indexing(2);
    test_builtin_constant_p();
    test_mixed_operations();
    
    /* Additional inline tests in main */
    int final_arr[100];
    
    /* Test various count values with constant bounds */
    final_arr[10] = 1;                     /* count = 1 */
    final_arr[20] = final_arr[21] = 2;    /* count = 2 */
    final_arr[30] = final_arr[31] = final_arr[32] = 3;  /* count = 3 */
    
    /* Large range with char type */
    char large_buf[1000];
    for (int i = 0; i < 100; i++) {
        large_buf[i] = i % 26 + 'A';  /* May be vectorized */
    }
    
    return 0;
}
