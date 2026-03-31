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

/* Test functions to cover different paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range with designated initializer */
    int arr2[10] = {[2] = 10, [3] = 20};  /* Two constant indices */
    
    /* MEM_P target with count = 2 via pointer arithmetic */
    int *p = &arr1[5];
    p[0] = 1;  /* Constant index 0 */
    p[1] = 2;  /* Constant index 1 */
}

void test_large_count_path(void) {
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf[100] = {[10 ... 20] = 'x'};  /* 11 elements, count > 2 */
    
    /* Another with explicit constant bounds */
    char buf2[50];
    for (int i = 5; i < 15; i++) {  /* Constant bounds: 5 to 14 (10 elements) */
        buf2[i] = 'a' + (i - 5);
    }
    
    /* Structure with array */
    struct ArrayStruct s;
    s.data[2] = 100;
    s.data[3] = 200;
    s.data[4] = 300;  /* Three elements - count = 3 */
}

void test_non_mem_target_path(void) {
    /* Path: !MEM_P(target) - results in registers */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Register targets from array accesses */
    int x = arr[2] + arr[3];      /* Sum goes to register */
    int y = arr[4] * arr[5];      /* Product goes to register */
    int z = (arr[6] > arr[7]) ? arr[6] : arr[7];  /* Conditional result in register */
    
    /* Complex expression favoring register allocation */
    int result = arr[1] + arr[2] * arr[3] - arr[4] / (arr[5] + 1);
    (void)result;
}

void test_vector_operations(void) {
    /* Vector extensions with constant indexing */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem = a[2];  /* Constant index 2 */
    
    /* Vector operations that might use registers */
    v4si c = a + b;    /* Vector add - result likely in register */
    v4si d = a * b;    /* Vector multiply */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = a * mask;
    
    /* Character vector with many elements */
    v16c chars = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    char c1 = chars[5];  /* Constant index */
    char c2 = chars[10]; /* Constant index */
}

void test_constant_string_indexing(void) {
    /* String literal with constant indices */
    char c1 = "hello world"[4];      /* 'o' */
    char c2 = "constant string"[7];  /* 't' */
    
    /* Multiple constant indices in expression */
    int diff = "abcd"[2] - "xyz"[1];  /* 'c' - 'y' */
    (void)diff;
}

void test_nested_and_complex_accesses(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int idx_arr[5] = {1, 3, 5, 7, 9};
    
    /* Nested array access with constant inner index */
    int x = arr[idx_arr[2]];  /* arr[5] = 50 */
    
    /* Complex expression with multiple constant indices */
    int y = arr[2] + arr[arr[1]/10];  /* arr[2] + arr[1] = 20 + 10 */
    
    /* Conditional with constant indices in both branches */
    int cond = 1;
    int z = (cond > 0) ? arr[3] : arr[4];
    
    /* Switch with array indexing */
    switch (arr[0]) {
        case 0: z = arr[1]; break;
        case 10: z = arr[2]; break;
        default: z = arr[3]; break;
    }
    
    (void)x; (void)y; (void)z;
}

void test_compile_time_constant_checks(void) {
    int arr[10] = {[0 ... 9] = 0};
    
    /* Use __builtin_constant_p to force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        arr[5] = 100;
    }
    
    /* Constant bounds in loop (might be unrolled) */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 10;
    }
    
    /* Another constant-bound loop */
    for (int i = 3; i < 8; i++) {  /* 3 to 7, count = 5 */
        arr[i] = arr[i-1] + 1;
    }
}

/* Main function combining all tests */
int main(void) {
    /* Execute all test functions to cover different paths */
    test_mem_target_paths();
    test_large_count_path();
    test_non_mem_target_path();
    test_vector_operations();
    test_constant_string_indexing();
    test_nested_and_complex_accesses();
    test_compile_time_constant_checks();
    
    return 0;
}
