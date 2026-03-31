/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* Vector types for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Structures with arrays */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];  /* Flexible array member */
};

/* Test function 1: Various constant array initializations */
void test_constant_bounds_init(void) {
    /* Designated initializers with constant ranges */
    int arr1[10] = {[2 ... 5] = 42};  /* count = 4, > 2 */
    char arr2[100] = {[10 ... 20] = 'x'};  /* count = 11, > 2, small element size */
    int arr3[10] = {[3] = 1, [4] = 2};  /* count = 2 exactly */
    int arr4[10] = {[7] = 99};  /* count = 1, <= 2 */
    
    /* String literal with constant indexing */
    char c1 = "hello"[2];  /* constant index 2 */
    char c2 = "test"[0];   /* constant index 0 */
    
    /* Array slicing through pointer with constant bounds */
    int *slice1 = &arr1[2];  /* start at constant index 2 */
    char *slice2 = &arr2[10]; /* start at constant index 10 */
}

/* Test function 2: Memory operations (MEM_P(target) true) */
void test_memory_operations(void) {
    struct ArrayStruct s;
    int local_arr[15];
    
    /* Direct memory accesses with constant indices */
    s.data[3] = 42;      /* Single element, MEM_P true */
    s.data[4] = s.data[3] + 1;  /* Load and store with constant indices */
    
    /* Two-element range operations */
    local_arr[5] = 10;
    local_arr[6] = 20;   /* Two adjacent elements */
    
    /* Larger range with char elements (count > 2, small type) */
    for (int i = 0; i < 10; i++) {
        s.buffer[i + 5] = 'a' + i;  /* Constant bounds in loop (0-9, offset 5) */
    }
    
    /* Pointer arithmetic with constant bounds */
    int *p = &local_arr[0];
    p[2] = 100;  /* Constant index 2 through pointer */
    p[3] = 200;  /* Constant index 3 */
}

/* Test function 3: Register operations (non-MEM_P(target)) */
int test_register_operations(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Expressions that likely go to registers */
    int reg1 = arr[2] + arr[3];  /* Sum of two array elements */
    int reg2 = arr[5] * 2;       /* Multiplication result */
    int reg3 = (arr[1] > arr[2]) ? arr[1] : arr[2];  /* Conditional */
    
    /* Nested array access with constant outer index */
    int nested[5] = {2, 3, 4, 5, 6};
    int reg4 = arr[nested[2]];  /* arr[4] */
    
    return reg1 + reg2 + reg3 + reg4;
}

/* Test function 4: Vector operations with constant indices */
void test_vector_operations(void) {
    v4si vec1 = {1, 2, 3, 4};
    v16c vec2 = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    v8hi vec3 = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Constant vector element access */
    int v1 = vec1[2];      /* Constant index 2 */
    char v2 = vec2[5];     /* Constant index 5 */
    short v3 = vec3[3];    /* Constant index 3 */
    
    /* Vector operations with constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;  /* Element-wise with constant mask */
    
    /* Vector conditional with constant indices */
    v4si cmp = vec1 > 2;
    v4si res = cmp ? vec1 : (v4si){0, 0, 0, 0};
}

/* Test function 5: Complex expressions with constant bounds */
void test_complex_expressions(void) {
    int arr[20];
    
    /* Initialize array */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * 2;
    }
    
    /* Switch with constant array indexing */
    int idx = 3;
    int result;
    switch (idx) {
        case 0: result = arr[1]; break;
        case 1: result = arr[2]; break;
        case 2: result = arr[3]; break;
        case 3: result = arr[4]; break;  /* Constant index 4 */
        default: result = arr[0]; break;
    }
    
    /* Multiple array accesses in one expression */
    int complex = arr[arr[2] / 2] + arr[5] * arr[3];
    
    /* Conditional with different constant indices */
    int cond = (arr[0] > 0) ? arr[1] : arr[2];
    
    /* Builtin to force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        /* This branch may be taken at compile time */
        arr[6] = 100;
    }
}

/* Test function 6: Flexible array member with constant access */
void test_flex_array(void) {
    /* Allocate flexible struct */
    struct FlexStruct *flex = (struct FlexStruct *)
        malloc(sizeof(struct FlexStruct) + 30 * sizeof(char));
    
    if (flex) {
        /* Constant indexing into flexible array member */
        flex->data[0] = 'A';
        flex->data[1] = 'B';
        flex->data[2] = 'C';  /* Three elements, count > 2 */
        
        /* Range access in flexible array */
        for (int i = 3; i < 10; i++) {
            flex->data[i] = 'x';  /* Constant bounds 3-9 */
        }
        
        free(flex);
    }
}

/* Test function 7: Mixed operations to trigger different paths */
void test_mixed_paths(void) {
    /* Create scenarios for different branches */
    
    /* Path 1: MEM_P true, count <= 2 */
    int mem1[5];
    mem1[0] = 1;  /* Single element */
    mem1[1] = 2;  /* Two elements total */
    
    /* Path 2: MEM_P true, count > 2, small element size */
    char mem2[50];
    for (int i = 10; i < 25; i++) {  /* count = 15 */
        mem2[i] = i % 26 + 'a';
    }
    
    /* Path 3: non-MEM_P (register), various counts */
    int arr[10] = {0};
    int reg_val1 = arr[0];  /* Single element to register */
    int reg_val2 = arr[1] + arr[2];  /* Two elements to register */
    int reg_val3 = arr[3] + arr[4] + arr[5];  /* Three elements to register */
    
    /* Path 4: Vector with constant indices */
    v4si v = {1, 2, 3, 4};
    int v_elem = v[2];  /* Constant index 2 */
}

/* Main function that calls all tests */
int main(void) {
    /* Execute all test functions to trigger different code paths */
    test_constant_bounds_init();
    test_memory_operations();
    int reg_result = test_register_operations();
    test_vector_operations();
    test_complex_expressions();
    test_flex_array();
    test_mixed_paths();
    
    /* Use results to prevent dead code elimination */
    return reg_result;
}
