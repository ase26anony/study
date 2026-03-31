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

/* Test functions covering different paths */
void test_mem_p_target_paths(void) {
    /* Path: MEM_P(target) true, count <= 2 */
    struct ArrayStruct s1;
    s1.data[5] = 42;           /* Single element - count = 1 */
    s1.data[6] = s1.data[7];   /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf[100];
    /* Designated initializer with constant range (11 elements) */
    char init_buf[100] = {[10 ... 20] = 'x'};
    
    /* Copy constant range to memory target */
    for (int i = 10; i <= 20; i++) {
        buf[i] = init_buf[i];
    }
    
    /* Larger range with char elements */
    char msg[50] = {[0 ... 9] = 'a', [10 ... 19] = 'b', [20 ... 29] = 'c'};
}

void test_non_mem_p_target_paths(void) {
    /* Path: !MEM_P(target) - results in registers */
    int arr[20] = {0};
    
    /* Constant indexing into register results */
    int reg1 = arr[2];                     /* Single element to register */
    int reg2 = arr[3] + arr[4];           /* Two elements combined in register */
    int reg3 = arr[5] * arr[6] - arr[7];  /* Multiple operations in register */
    
    /* Conditional with constant indices */
    int x = 1;
    int reg4 = (x > 0) ? arr[8] : arr[9];
    
    /* Nested array access with constant outer index */
    int matrix[5][5] = {{0}};
    int reg5 = matrix[2][3];              /* Constant 2D indices */
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[0];      /* Constant index 0 */
    int elem2 = vec1[2];      /* Constant index 2 */
    
    /* Vector operations that may use constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si res1 = vec1 * mask;  /* Constant mask operation */
    
    /* Vector shuffle with constant indices */
    v4si shuffled = __builtin_shuffle(vec1, vec2, (v4si){0, 4, 1, 5});
    
    /* Small element vector with many elements */
    v16c char_vec = {0};
    char_vec[0] = 'a';
    char_vec[15] = 'z';
    
    /* Constant range in vector initialization */
    v8s short_vec = {[0 ... 3] = 100, [4 ... 7] = 200};
}

void test_constant_bounds_loops(void) {
    int arr[10];
    
    /* Loop with constant bound (5 iterations) */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 2;
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
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int x = 2;
    
    switch (x) {
        case 0:
            x = arr[1];  /* Constant index 1 */
            break;
        case 1:
            x = arr[2];  /* Constant index 2 */
            break;
        case 2:
            x = arr[3] + arr[4];  /* Two constant indices */
            break;
        default:
            x = arr[5];  /* Constant index 5 */
    }
}

void test_builtin_constant_p(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation of array bounds */
    if (__builtin_constant_p(arr[5])) {
        arr[5] = 100;
    }
    
    /* Check constant index */
    if (__builtin_constant_p(5)) {
        arr[5] = 200;
    }
}

void test_string_literal_indexing(void) {
    /* String literal with constant indices */
    char c1 = "hello"[0];     /* Constant index 0 */
    char c2 = "hello"[2];     /* Constant index 2 */
    char c3 = "hello"[4];     /* Constant index 4 */
    
    /* Array of strings with constant indexing */
    const char *strings[] = {"one", "two", "three"};
    char c4 = strings[1][2];  /* Constant indices 1 and 2 */
}

void test_designated_initializers(void) {
    /* Various designated initializers with constant ranges */
    int range1[10] = {[2 ... 5] = 42};           /* 4 elements */
    int range2[10] = {[1] = 10, [3] = 30};       /* 2 separate elements */
    int range3[10] = {[0 ... 9] = 99};           /* 10 elements */
    
    /* Mixed initialization */
    int mixed[20] = {
        0, 1, 2,
        [5 ... 10] = 50,
        [15] = 100,
        [18 ... 19] = 200
    };
    
    /* Small element type with large range */
    char char_range[100] = {[20 ... 40] = 'X'};  /* 21 chars */
}

void test_pointer_arithmetic_const_bounds(void) {
    struct ArrayStruct s;
    int *p;
    
    /* Pointer with constant offset */
    p = &s.data[5];           /* Constant index 5 */
    *p = 100;
    
    p = &s.data[2];           /* Constant index 2 */
    *(p + 1) = 200;           /* Constant offset 1 */
    
    /* Array section through pointer */
    int arr[20];
    int *start = &arr[5];
    int *end = &arr[10];
    
    /* Constant bound pointer loop */
    for (int *ptr = &arr[0]; ptr < &arr[10]; ptr++) {
        *ptr = (int)(ptr - &arr[0]);
    }
}

int main(void) {
    /* Execute all test functions to cover different paths */
    test_mem_p_target_paths();
    test_non_mem_p_target_paths();
    test_vector_operations();
    test_constant_bounds_loops();
    test_switch_array_indexing();
    test_builtin_constant_p();
    test_string_literal_indexing();
    test_designated_initializers();
    test_pointer_arithmetic_const_bounds();
    
    return 0;
}
