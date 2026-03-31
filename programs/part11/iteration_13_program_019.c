/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* Vector extension types */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* Structures with arrays */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];  /* Flexible array member */
};

/* Test functions for different code paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) == true, count <= 2 */
    struct ArrayStruct s1;
    s1.data[5] = 42;           /* Single element - count == 1 */
    s1.data[6] = s1.data[7];   /* Two elements - count == 2 */
    
    /* Path: MEM_P(target) == true, count > 2, small element type */
    char buf[100] = {[10 ... 20] = 'x'};  /* 11 elements, char size 1 */
    char buf2[50] = {[5 ... 15] = 'y'};   /* 11 elements */
    
    /* Path: MEM_P(target) == true, count > 2, larger element type */
    int arr[50] = {[20 ... 30] = 99};     /* 11 elements, int size */
}

void test_non_mem_target_paths(void) {
    /* Path: MEM_P(target) == false (register targets) */
    int local_arr[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Results likely go to registers */
    int reg1 = local_arr[2] + local_arr[3];      /* Constant indices */
    int reg2 = local_arr[4] * local_arr[5];      /* Constant indices */
    int reg3 = (local_arr[6] > 0) ? local_arr[7] : local_arr[8];
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[0];      /* Constant index 0 */
    int elem2 = vec1[2];      /* Constant index 2 */
    
    /* Vector operations that might trigger the logic */
    v4si vec3 = vec1 + vec2;
    int sum = vec3[1] + vec3[3];  /* Constant indices */
    
    /* Small element vector */
    v16c char_vec = {[0 ... 15] = 32};  /* 16 char elements */
    char c1 = char_vec[5];              /* Constant index */
    char c2 = char_vec[10];             /* Constant index */
}

void test_designated_initializers(void) {
    /* Various designated initializers with constant ranges */
    int arr1[10] = {[2 ... 5] = 42};           /* 4 elements */
    int arr2[20] = {[3] = 1, [4] = 2};         /* 2 elements */
    int arr3[30] = {[10 ... 14] = 99, [20 ... 22] = 77}; /* Multiple ranges */
    
    /* Mixed initialization */
    char str_arr[50] = {0, 1, 2, [10 ... 15] = 'A', [40 ... 45] = 'B'};
}

void test_complex_expressions(void) {
    int base[100] = {[0 ... 99] = 0};
    
    /* Nested array accesses with constant indices */
    base[10] = base[base[5]];  /* Outer index from array element */
    
    /* Conditional with constant indices */
    int x = 1;
    int y = (x > 0) ? base[2] : base[3];
    
    /* Complex expression with multiple constant indices */
    int z = base[1] + base[2] * base[3] - base[4];
    
    /* String literal with constant indexing */
    char c = "constant_string"[7];  /* Constant index 7 */
}

void test_loop_unrolling(void) {
    int arr[10];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
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

void test_switch_cases(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int selector = 2;
    int result = 0;
    
    /* Switch with array indexing in cases */
    switch (selector) {
        case 0:
            result = arr[1];  /* Constant index 1 */
            break;
        case 1:
            result = arr[2];  /* Constant index 2 */
            break;
        case 2:
            result = arr[3] + arr[4];  /* Two constant indices */
            break;
        default:
            result = arr[5];  /* Constant index 5 */
    }
}

void test_builtin_constants(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        arr[6] = 100;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(5)) {
        arr[7] = 200;
    }
}

void test_pointer_arithmetic(void) {
    struct ArrayStruct s;
    int *p1 = &s.data[2];      /* Constant offset 2 */
    int *p2 = &s.data[5];      /* Constant offset 5 */
    
    /* Pointer difference with constant indices */
    ptrdiff_t diff = p2 - p1;  /* Should be 3 */
    
    /* Array access through pointer with constant offset */
    p1[0] = 10;    /* Equivalent to s.data[2] = 10 */
    p1[1] = 20;    /* Equivalent to s.data[3] = 20 */
}

/* Main function that exercises all paths */
int main(void) {
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_designated_initializers();
    test_complex_expressions();
    test_loop_unrolling();
    test_switch_cases();
    test_builtin_constants();
    test_pointer_arithmetic();
    
    return 0;
}
