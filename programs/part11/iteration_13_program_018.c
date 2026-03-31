/* Test program to exercise constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* GCC vector extensions */
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
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range with designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count = 11 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars - count = 11 */
    
    /* Path: MEM_P(target) true, count > 2, larger element type */
    int arr3[50] = {[10 ... 15] = 99};     /* 6 ints - count = 6 */
}

void test_non_mem_target_paths(void) {
    /* Path: non-MEM_P(target) - results in registers */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* These likely go to registers */
    int reg1 = arr[2] + arr[3];      /* Constant indices 2 and 3 */
    int reg2 = arr[4] * arr[5];      /* Constant indices 4 and 5 */
    int reg3 = arr[1] - arr[8];      /* Constant indices 1 and 8 */
    
    /* Complex expression with register result */
    int reg4 = (arr[2] > arr[3]) ? arr[4] : arr[5];
    
    /* Use results to avoid dead code elimination */
    printf("Reg results: %d %d %d %d\n", reg1, reg2, reg3, reg4);
}

void test_vector_operations(void) {
    /* Vector with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem1 = vec1[2];      /* Constant index 2 */
    int elem2 = vec2[1];      /* Constant index 1 */
    
    /* Vector operations that might trigger the logic */
    v4si vec3 = vec1 + vec2;
    int elem3 = vec3[3];      /* Constant index 3 */
    
    /* Vector with char elements - small type size */
    v16c char_vec = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    char c1 = char_vec[5];    /* Constant index 5 */
    char c2 = char_vec[10];   /* Constant index 10 */
    
    printf("Vector elements: %d %d %d %c %c\n", elem1, elem2, elem3, c1, c2);
}

void test_constant_string_indexing(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[4];      /* Constant index 4 -> 'o' */
    char c2 = "constant string"[7];  /* Constant index 7 -> 't' */
    
    /* Array initialized from string literal */
    char arr[] = "test array";
    char c3 = arr[5];                /* Constant index 5 -> 'a' */
    
    printf("Chars: %c %c %c\n", c1, c2, c3);
}

void test_nested_and_conditional_access(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    
    /* Nested array access with constant inner index */
    int idx_arr[5] = {2, 3, 4, 1, 0};
    int x = arr[idx_arr[2]];  /* idx_arr[2] is constant 4 -> arr[4] = 40 */
    
    /* Conditional with constant indices */
    int cond = 1;
    int y = (cond > 0) ? arr[3] : arr[7];  /* Both indices constant */
    
    /* Switch with constant array indexing */
    int z;
    switch (cond) {
        case 0: z = arr[1]; break;
        case 1: z = arr[2]; break;  /* This path taken */
        case 2: z = arr[3]; break;
        default: z = arr[4]; break;
    }
    
    printf("Nested/conditional: %d %d %d\n", x, y, z);
}

void test_loop_with_constant_bounds(void) {
    int arr[10];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 10;
    }
    
    /* Nested loops with constant bounds */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {      /* Constant bound 3 */
        for (int j = 0; j < 3; j++) {  /* Constant bound 3 */
            matrix[i][j] = i * 3 + j;
        }
    }
    
    /* Verify some values */
    printf("Loop results: %d %d\n", arr[3], matrix[1][2]);
}

void test_struct_array_access(void) {
    struct ArrayStruct s;
    
    /* Access struct array members with constant indices */
    s.data[5] = 100;      /* Constant index 5 */
    s.data[6] = 200;      /* Constant index 6 */
    s.buffer[10] = 'A';   /* Constant index 10 */
    s.buffer[11] = 'B';   /* Constant index 11 */
    
    /* Pointer to sub-range with constant offset */
    int *p = &s.data[3];  /* Constant offset 3 */
    p[0] = 30;            /* Equivalent to s.data[3] */
    p[1] = 40;            /* Equivalent to s.data[4] */
    
    printf("Struct array: %d %d %c %c\n", 
           s.data[5], s.data[6], s.buffer[10], s.buffer[11]);
}

void test_compile_time_constant_checks(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Use __builtin_constant_p to force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        printf("arr[5] is compile-time constant\n");
    }
    
    /* Constant expression with array index */
    int x = 2 + 3;
    int y = arr[x];  /* x is constant 5 after optimization */
    
    /* Multi-dimensional array with constant indices */
    int md[4][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}, {13,14,15,16}};
    int z = md[2][3];  /* Constant indices 2 and 3 -> 12 */
    
    printf("Constant checks: %d %d\n", y, z);
}

int main(void) {
    printf("Testing constant bounds array operations...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_constant_string_indexing();
    test_nested_and_conditional_access();
    test_loop_with_constant_bounds();
    test_struct_array_access();
    test_compile_time_constant_checks();
    
    printf("All tests completed.\n");
    return 0;
}
