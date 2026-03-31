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
    char data[];  /* Zero-length array */
};

/* Test functions for different scenarios */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) == true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count == 1 */
    arr1[4] = arr1[3] + 1;  /* Memory to memory with constant index */
    
    /* Two-element range in designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements, count == 2 */
    
    /* Structure member access with constant indices */
    struct ArrayStruct s1;
    s1.data[5] = 100;       /* Single element access */
    s1.buffer[10] = 'a';    /* char type, single element */
    s1.buffer[11] = 'b';    /* Makes count potentially 2 if combined */
}

void test_large_count_path(void) {
    /* Path: MEM_P(target) == true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements, count > 2 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 elements */
    
    /* Larger range with char type */
    char buf3[200] = {[30 ... 80] = 'z'};  /* 51 elements */
    
    /* Multiple ranges in same initializer */
    int arr3[100] = {[10 ... 19] = 1, [30 ... 39] = 2};  /* Two ranges of 10 */
}

void test_register_target_path(void) {
    /* Path: !MEM_P(target) - results likely go to registers */
    int arr[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Arithmetic with constant array indices - results in registers */
    int x = arr[2] + arr[3];      /* Both constant indices */
    int y = arr[5] * arr[6];      /* Multiplication result in register */
    int z = arr[1] - arr[8];      /* Subtraction */
    
    /* Complex expression with multiple constant indices */
    int w = arr[0] + arr[1] * arr[2] - arr[3] / arr[4];
    
    /* Conditional with constant indices */
    int cond = (arr[2] > arr[3]) ? arr[4] : arr[5];
}

void test_vector_operations(void) {
    /* Vector extensions with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem1 = vec1[2];          /* Constant index 2 */
    int elem2 = vec2[0];          /* Constant index 0 */
    
    /* Vector operations that might use constant masks */
    v4si mask = {0, -1, 0, -1};   /* Constant mask */
    v4si res1 = vec1 * mask;      /* Element-wise with constant mask */
    v4si res2 = vec1 + vec2;      /* Vector addition */
    
    /* char vector with many elements */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = chars[5];           /* Constant index */
    char c2 = chars[10];          /* Constant index */
}

void test_constant_string_ops(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[4];    /* Constant index 4 -> 'o' */
    char c2 = "test string"[0];    /* Constant index 0 -> 't' */
    
    /* Multiple string accesses */
    const char *str = "constant string";
    char c3 = str[7];              /* Constant index 7 -> ' ' */
    char c4 = str[8];              /* Constant index 8 -> 's' */
}

void test_loop_unrolling(void) {
    /* Loop with constant bounds that might be unrolled */
    int arr[10];
    
    /* Small constant loop */
    for (int i = 0; i < 5; i++) {   /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Nested constant loops */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {   /* Outer constant bound */
        for (int j = 0; j < 3; j++) { /* Inner constant bound */
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_cases(void) {
    /* Switch with array indexing in cases */
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int x = 2;
    
    switch (x) {
        case 0:
            x = arr[1];      /* Constant index 1 */
            break;
        case 1:
            x = arr[2];      /* Constant index 2 */
            break;
        case 2:
            x = arr[3];      /* Constant index 3 */
            break;
        default:
            x = arr[0];      /* Constant index 0 */
    }
}

void test_nested_accesses(void) {
    /* Nested array accesses with constant components */
    int arr[10] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    
    /* Outer index from array element, inner constant */
    int x = arr[arr[2]];     /* arr[2] = 4, so arr[4] = 5 */
    
    /* Multiple levels */
    int y = arr[arr[arr[1]]]; /* arr[1] = 1, arr[1] = 1, arr[1] = 1 */
    
    /* With arithmetic */
    int z = arr[arr[3] + arr[4]]; /* 1 + 5 = 6, arr[6] = 2 */
}

void test_builtin_constant(void) {
    /* Using __builtin_constant_p to force constant evaluation */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        int x = arr[5];  /* Constant index 5 */
    }
    
    /* Check multiple indices */
    if (__builtin_constant_p(arr[2]) && __builtin_constant_p(arr[7])) {
        int y = arr[2] + arr[7];
    }
}

void test_mixed_operations(void) {
    /* Mixed operations to trigger different code paths */
    struct ArrayStruct s;
    int local_arr[20];
    
    /* Combination of memory and register targets */
    s.data[5] = local_arr[3] + local_arr[4];  /* RHS in registers, LHS memory */
    
    /* Multiple ranges in initialization */
    int init_arr[50] = {
        [0 ... 9] = 1,      /* 10 elements, count > 2 */
        [10] = 2,           /* Single element */
        [11 ... 12] = 3,    /* 2 elements, count == 2 */
        [20 ... 30] = 4     /* 11 elements, count > 2 */
    };
    
    /* Pointer arithmetic with constant bounds */
    int *p = &s.data[0];
    p[5] = 100;            /* Constant offset 5 */
    p[6] = 200;            /* Constant offset 6 */
    
    /* Different element sizes */
    short shorts[100] = {[10 ... 20] = 255};  /* 11 elements, 2 bytes each */
    long longs[50] = {[5 ... 10] = 999};      /* 6 elements, 8 bytes each */
}

int main(void) {
    /* Execute all test functions */
    test_mem_target_paths();
    test_large_count_path();
    test_register_target_path();
    test_vector_operations();
    test_constant_string_ops();
    test_loop_unrolling();
    test_switch_cases();
    test_nested_accesses();
    test_builtin_constant();
    test_mixed_operations();
    
    return 0;
}
