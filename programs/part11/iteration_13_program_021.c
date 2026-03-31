/* Test program to exercise constant-bounds array/vector operations in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* GCC vector extensions */
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
    char data[];  /* zero-length array */
};

/* Test functions covering different paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Memory target with constant index */
    
    /* Two-element range with designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count = 11 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars - count = 11 */
    
    /* Path: MEM_P(target) true, count > 2, larger element type */
    short shorts[50] = {[10 ... 20] = 100};  /* 11 shorts - count = 11 */
    
    /* Use the arrays to prevent optimization */
    printf("arr1[3]=%d, arr2[3]=%d\n", arr1[3], arr2[3]);
    printf("buf1[15]=%c, shorts[15]=%d\n", buf1[15], shorts[15]);
}

void test_non_mem_target_paths(void) {
    /* Path: non-MEM_P(target) - results in registers */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Register targets with constant indexing */
    int reg1 = arr[2];                     /* Single element to register */
    int reg2 = arr[3] + arr[4];            /* Expression result to register */
    int reg3 = arr[5] * arr[6] - arr[7];   /* Complex expression to register */
    
    /* Conditional with constant indices */
    int cond = 1;
    int reg4 = (cond ? arr[2] : arr[3]);   /* Conditional result to register */
    
    printf("reg1=%d, reg2=%d, reg3=%d, reg4=%d\n", reg1, reg2, reg3, reg4);
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[2];           /* Constant index 2 */
    int elem2 = vec2[3];           /* Constant index 3 */
    
    /* Vector operations that might trigger constant bounds checking */
    v4si vec3 = vec1 + vec2;
    int elem3 = vec3[1];           /* Constant index 1 */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    int elem4 = masked[1];         /* Constant index 1 */
    
    /* Small element type vector */
    v16c char_vec = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    char c1 = char_vec[10];        /* Constant index 10 */
    
    printf("vec1[2]=%d, vec2[3]=%d, masked[1]=%d, char_vec[10]=%d\n", 
           elem1, elem2, elem4, c1);
}

void test_constant_string_indexing(void) {
    /* String literal with constant indices */
    char c1 = "hello world"[4];     /* Constant index 4 -> 'o' */
    char c2 = "test string"[6];     /* Constant index 6 -> 't' */
    
    /* Multiple constant string accesses */
    const char *str = "constant";
    char c3 = str[3];               /* Constant index 3 -> 's' */
    char c4 = str[5];               /* Constant index 5 -> 'a' */
    
    printf("c1='%c', c2='%c', c3='%c', c4='%c'\n", c1, c2, c3, c4);
}

void test_nested_and_complex_accesses(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int idx_arr[5] = {1, 3, 5, 7, 9};
    
    /* Nested array access with constant inner index */
    int x1 = arr[idx_arr[2]];       /* arr[5] = 50, idx_arr[2] is constant 5 */
    
    /* Complex expression with multiple constant indices */
    int x2 = arr[2] + arr[3] * arr[4] - arr[5];
    
    /* Array slicing through pointer */
    struct ArrayStruct s;
    int *slice = &s.data[5];        /* Constant start index 5 */
    slice[0] = 100;                 /* Equivalent to s.data[5] = 100 */
    slice[1] = 200;                 /* Equivalent to s.data[6] = 200 */
    
    printf("x1=%d, x2=%d, slice[0]=%d\n", x1, x2, slice[0]);
}

void test_loop_with_constant_bounds(void) {
    int arr[10];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {   /* Constant bound 5 */
        arr[i] = i * 10;
    }
    
    /* Nested loops with constant bounds */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {   /* Constant bound 3 */
        for (int j = 0; j < 3; j++) { /* Constant bound 3 */
            matrix[i][j] = i * 3 + j;
        }
    }
    
    printf("arr[4]=%d, matrix[2][2]=%d\n", arr[4], matrix[2][2]);
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    int x = 2;
    
    /* Switch with constant indices in cases */
    switch (x) {
        case 0: 
            printf("case 0: %d\n", arr[1]);  /* Constant index 1 */
            break;
        case 1: 
            printf("case 1: %d\n", arr[2]);  /* Constant index 2 */
            break;
        case 2: 
            printf("case 2: %d\n", arr[3]);  /* Constant index 3 */
            break;
        default:
            printf("default: %d\n", arr[4]); /* Constant index 4 */
    }
}

void test_builtin_constant_p(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Force constant evaluation with __builtin_constant_p */
    if (__builtin_constant_p(arr[5])) {
        printf("arr[5] is constant: %d\n", arr[5]);
    }
    
    /* Test with constant index */
    if (__builtin_constant_p("string"[3])) {
        printf("'string'[3] is constant: %c\n", "string"[3]);
    }
}

int main(void) {
    printf("=== Testing constant bounds array/vector operations ===\n\n");
    
    test_mem_target_paths();
    printf("\n");
    
    test_non_mem_target_paths();
    printf("\n");
    
    test_vector_operations();
    printf("\n");
    
    test_constant_string_indexing();
    printf("\n");
    
    test_nested_and_complex_accesses();
    printf("\n");
    
    test_loop_with_constant_bounds();
    printf("\n");
    
    test_switch_with_array_indexing();
    printf("\n");
    
    test_builtin_constant_p();
    
    return 0;
}
