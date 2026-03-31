/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* Vector extension types */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8s __attribute__((vector_size(16)));

/* Structures with arrays */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];
};

/* Test functions for different scenarios */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[5] = 99; /* Two elements - count = 2 */
    
    /* Designated initializer with constant range (2 elements) */
    int arr2[10] = {[3] = 1, [4] = 2};  /* count = 2 */
    
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};      /* count = 11, char size = 1 */
    char buf2[50] = {[5 ... 15] = 'a'};        /* count = 11 */
    
    /* Path: MEM_P(target) = true, count > 2, larger element type */
    short shorts[50] = {[10 ... 25] = 100};    /* count = 16, short size = 2 */
}

void test_non_mem_target_paths(void) {
    /* Path: MEM_P(target) = false (results in registers) */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Results likely go to registers */
    int reg1 = arr[2] + arr[3];      /* Constant indices 2 and 3 */
    int reg2 = arr[4] * arr[5];      /* Constant indices 4 and 5 */
    int reg3 = arr[1] - arr[6];      /* Mixed constant indices */
    
    /* Complex expression with register result */
    int reg4 = (arr[2] > arr[3]) ? arr[4] : arr[5];
}

void test_vector_operations(void) {
    /* Vector with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    int elem1 = vec1[2];          /* Constant index 2 */
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;      /* Vector operation */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;    /* Constant vector mask */
    
    /* Vector conditional with constant indices */
    v16c chars = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    char c1 = chars[5];           /* Constant index 5 */
    char c2 = chars[10];          /* Constant index 10 */
}

void test_nested_and_complex_accesses(void) {
    int arr[20] = {[0 ... 19] = 0};
    
    /* Nested array access with constant inner index */
    int idx_arr[5] = {2, 3, 4, 5, 6};
    int x = arr[idx_arr[2]];      /* idx_arr[2] = 4 (constant) */
    
    /* Conditional with constant array indices */
    int cond = 1;
    int y = (cond ? arr[3] : arr[7]);  /* Both branches constant indices */
    
    /* Multiple constant range initializations */
    int multi[30] = {
        [0 ... 9] = 100,
        [10 ... 19] = 200,
        [20 ... 29] = 300
    };
}

void test_loop_with_constant_bounds(void) {
    int arr[10];
    
    /* Loop with constant bound (may be unrolled) */
    for (int i = 0; i < 5; i++) {   /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Nested loops with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int selector = 2;
    
    switch (selector) {
        case 0: arr[1] = 100; break;   /* Constant index 1 */
        case 1: arr[2] = 200; break;   /* Constant index 2 */
        case 2: arr[3] = 300; break;   /* Constant index 3 */
        case 3: arr[4] = 400; break;   /* Constant index 4 */
        default: arr[5] = 500; break;  /* Constant index 5 */
    }
}

void test_string_literal_indexing(void) {
    /* String literal with constant indexing */
    char c1 = "Hello World!"[4];    /* Constant index 4 = 'o' */
    char c2 = "Test String"[6];     /* Constant index 6 = 'S' */
    
    /* Array of strings with constant indexing */
    const char *strings[] = {"one", "two", "three", "four"};
    char c3 = strings[2][1];        /* strings[2] = "three", [1] = 'h' */
}

void test_struct_array_access(void) {
    struct ArrayStruct s;
    
    /* Access struct array with constant indices */
    s.data[5] = 100;           /* Constant index 5 */
    s.buffer[10] = 'A';        /* Constant index 10 */
    
    /* Pointer to sub-range with constant start */
    int *p1 = &s.data[3];      /* Constant start index 3 */
    char *p2 = &s.buffer[20];  /* Constant start index 20 */
}

/* Force constant evaluation with builtins */
void test_builtin_constant_p(void) {
    int arr[10] = {0};
    
    /* Force compiler to consider array access as constant */
    if (__builtin_constant_p(arr[5])) {
        arr[6] = 1;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(5) && __builtin_constant_p(8)) {
        int range[10] = {[5 ... 8] = 99};  /* Constant range 5-8 */
    }
}

/* Main function combining all tests */
int main(void) {
    /* Execute all test patterns to maximize coverage */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_nested_and_complex_accesses();
    test_loop_with_constant_bounds();
    test_switch_with_array_indexing();
    test_string_literal_indexing();
    test_struct_array_access();
    test_builtin_constant_p();
    
    return 0;
}
