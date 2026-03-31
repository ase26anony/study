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
    char data[];  /* Flexible array member */
};

/* Test functions for different code paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range in initialization */
    int arr2[10] = {[2] = 10, [3] = 20};  /* Two elements with constant indices */
    
    /* Two-element range via assignment */
    int arr3[10];
    arr3[5] = 100;
    arr3[6] = 200;  /* Two adjacent elements */
}

void test_large_count_path(void) {
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements, count > 2 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 elements */
    
    /* Larger range with char type */
    char buf3[200];
    for (int i = 30; i < 50; i++) {  /* Constant bounds: 30 to 49, count = 20 */
        buf3[i] = i;
    }
}

void test_non_mem_target_path(void) {
    /* Path: !MEM_P(target) - results likely go to registers */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Register-targeting operations with constant indices */
    int x = arr[2] + arr[3];      /* Both indices constant, result in register */
    int y = arr[4] * arr[5];      /* Multiplication result in register */
    int z = arr[6] | arr[7];      /* Bitwise operation result in register */
    
    /* Complex expression with constant indices */
    int w = (arr[1] << 2) | (arr[2] & 0xFF);
    
    (void)x; (void)y; (void)z; (void)w; /* Suppress unused warnings */
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = a[2];      /* Constant index 2 */
    int elem2 = b[1];      /* Constant index 1 */
    
    /* Vector operations that might trigger constant bounds checking */
    v4si c = a + b;        /* Vector addition */
    v4si d = a * b;        /* Vector multiplication */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = a & mask;
    
    (void)elem1; (void)elem2; (void)c; (void)d; (void)masked;
}

void test_string_constant_indexing(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[4];      /* Constant index 4 = 'o' */
    char c2 = "test string"[0];      /* Constant index 0 = 't' */
    
    /* Array initialized from string constant */
    char arr[] = "constant string";
    char c3 = arr[7];                /* Constant index 7 = 't' */
    
    (void)c1; (void)c2; (void)c3;
}

void test_designated_initializers(void) {
    /* Various designated initializers with constant ranges */
    int arr1[20] = {[2 ... 5] = 42};          /* Range 2-5, count=4 */
    int arr2[15] = {[3] = 10, [7 ... 9] = 20}; /* Mixed single and range */
    char arr3[30] = {[10 ... 19] = 'A'};      /* Range 10-19, count=10 */
    
    /* Nested designated initializers in struct */
    struct ArrayStruct s1 = {
        .data = {[5 ... 8] = 100},
        .buffer = {[0 ... 9] = 'Z'}
    };
    
    (void)arr1; (void)arr2; (void)arr3; (void)s1;
}

void test_constant_bounds_loops(void) {
    /* Loops with constant bounds (may be unrolled) */
    int arr[10];
    
    /* Small constant bound loop */
    for (int i = 0; i < 3; i++) {    /* Constant bound 3 */
        arr[i] = i * 10;
    }
    
    /* Larger constant bound */
    int brr[20];
    for (int j = 5; j < 15; j++) {   /* Constant bounds 5-14, count=10 */
        brr[j] = j * 2;
    }
}

void test_switch_with_array_indexing(void) {
    /* Switch statements with constant array indexing */
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int x = 2;
    
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

void test_nested_array_access(void) {
    /* Nested array accesses with constant indices */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    int idx_arr[5] = {2,3,4,1,0};
    
    /* Outer constant index, inner variable */
    int x = arr[idx_arr[2]];  /* idx_arr[2] = 4, so arr[4] = 4 */
    
    /* Multiple levels */
    int y = arr[arr[3]];      /* arr[3] = 3, so arr[3] = 3 */
    
    (void)x; (void)y;
}

void test_conditional_array_access(void) {
    /* Conditional expressions with constant array indices */
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int cond = 1;
    
    /* Ternary with constant indices */
    int x = cond ? arr[2] : arr[3];  /* Both indices constant */
    
    /* More complex conditional */
    int y = (cond > 0) ? arr[4] + arr[5] : arr[6] - arr[7];
    
    (void)x; (void)y;
}

void test_builtin_constant_p(void) {
    /* Using __builtin_constant_p with array accesses */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    if (__builtin_constant_p(arr[5])) {
        /* Force compiler to consider constant array access */
        int x = arr[5] * 2;
        (void)x;
    }
    
    /* Test with constant index */
    if (__builtin_constant_p(5)) {
        int y = arr[5];  /* Index is constant */
        (void)y;
    }
}

void test_small_vector_types(void) {
    /* Small vector types to test TYPE_SIZE calculations */
    typedef char v8qi __attribute__((vector_size(8)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v8qi a = {1,2,3,4,5,6,7,8};
    v8hi b = {100,200,300,400,500,600,700,800};
    
    /* Constant indexing on small vectors */
    char c1 = a[3];    /* Constant index 3 */
    short s1 = b[5];   /* Constant index 5 */
    
    /* Vector operations */
    v8qi a2 = a + a;   /* Should trigger vector expansion */
    v8hi b2 = b * 2;   /* Vector scalar multiplication */
    
    (void)c1; (void)s1; (void)a2; (void)b2;
}

int main(void) {
    printf("Testing constant bounds array/vector operations...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_large_count_path();
    test_non_mem_target_path();
    test_vector_operations();
    test_string_constant_indexing();
    test_designated_initializers();
    test_constant_bounds_loops();
    test_switch_with_array_indexing();
    test_nested_array_access();
    test_conditional_array_access();
    test_builtin_constant_p();
    test_small_vector_types();
    
    printf("All tests completed (runtime behavior is secondary).\n");
    return 0;
}
