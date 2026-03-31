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
    /* Path 1: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[5] = 99; /* Two elements - count = 2 */
    
    /* Path 2: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements, char size = 1 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 elements */
    
    /* Path 3: MEM_P(target) true, count > 2, larger element type */
    int arr2[50] = {[10 ... 25] = 123};    /* 16 elements, int size = 4 */
    
    /* Use designated initializers with constant ranges */
    int arr3[20] = {[2 ... 5] = 42, [10 ... 12] = 99};
}

void test_non_mem_target_paths(void) {
    /* Path where target is not memory (likely register) */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Results likely go to registers */
    int reg1 = arr[2] + arr[3];      /* Constant indices 2 and 3 */
    int reg2 = arr[4] * arr[5];      /* Constant indices 4 and 5 */
    int reg3 = arr[6] - arr[7];      /* Constant indices 6 and 7 */
    
    /* Complex expression with register target */
    int reg4 = (arr[1] > arr[2]) ? arr[3] : arr[4];
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[0];  /* Constant index 0 */
    int elem2 = vec1[2];  /* Constant index 2 */
    
    /* Vector operations that might trigger constant bounds */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 & mask;
    
    /* Small element vector */
    v16qi char_vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    char c1 = char_vec[5];  /* Constant index 5 */
    char c2 = char_vec[10]; /* Constant index 10 */
}

void test_string_operations(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[0];  /* Constant index 0 */
    char c2 = "hello world"[5];  /* Constant index 5 */
    char c3 = "hello world"[10]; /* Constant index 10 */
    
    /* Array from string literal */
    char str[] = "constant string";
    char c4 = str[3];  /* Constant index 3 */
    char c5 = str[8];  /* Constant index 8 */
}

void test_nested_and_conditional_access(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int idx_arr[5] = {1, 3, 5, 7, 9};
    
    /* Nested array access with constant inner index */
    int x1 = arr[idx_arr[2]];  /* idx_arr[2] = 5 (constant index 2) */
    
    /* Conditional array access with constant indices */
    int cond = 1;
    int y1 = (cond ? arr[2] : arr[3]);  /* Both indices constant */
    int y2 = (!cond ? arr[4] : arr[5]); /* Both indices constant */
    
    /* Complex expression combining multiple constant accesses */
    int z = arr[1] + arr[idx_arr[0]] * arr[3] - arr[4];
}

void test_loop_with_constant_bounds(void) {
    int arr[10];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Nested loops with constant bounds */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {      /* Constant bound 3 */
        for (int j = 0; j < 3; j++) {  /* Constant bound 3 */
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int x = 2;
    
    /* Switch with constant array indices in cases */
    switch (x) {
        case 0: 
            printf("%d\n", arr[0]);  /* Constant index 0 */
            break;
        case 1:
            printf("%d\n", arr[1]);  /* Constant index 1 */
            break;
        case 2:
            printf("%d\n", arr[2]);  /* Constant index 2 */
            break;
        default:
            printf("%d\n", arr[9]);  /* Constant index 9 */
    }
}

void test_struct_array_access(void) {
    struct ArrayStruct s;
    
    /* Access array within struct with constant indices */
    s.data[0] = 100;   /* Constant index 0 */
    s.data[5] = 200;   /* Constant index 5 */
    s.data[10] = 300;  /* Constant index 10 */
    
    /* Pointer to sub-array with constant start index */
    int *p1 = &s.data[2];   /* Constant start index 2 */
    int *p2 = &s.data[5];   /* Constant start index 5 */
    
    /* Access through pointer with constant offset */
    p1[0] = 400;  /* Constant offset 0 */
    p1[1] = 500;  /* Constant offset 1 */
    p2[2] = 600;  /* Constant offset 2 */
}

void test_builtin_constant_p(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        printf("arr[5] is constant\n");
    }
    
    /* Test with constant index */
    if (__builtin_constant_p("hello"[2])) {
        printf("String index is constant\n");
    }
}

/* Main function combining all tests */
int main(void) {
    printf("Testing constant bounds coverage...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_string_operations();
    test_nested_and_conditional_access();
    test_loop_with_constant_bounds();
    test_switch_with_array_indexing();
    test_struct_array_access();
    test_builtin_constant_p();
    
    /* Additional combined tests */
    {
        /* Mixed operations in single scope */
        int arr[20];
        char str[] = "test string";
        
        /* Multiple constant-bound operations */
        arr[0] = str[0];
        arr[1] = str[5];
        arr[2] = arr[0] + arr[1];
        
        /* Constant range initialization in local array */
        int local_arr[10] = {[2 ... 4] = 42, [6 ... 8] = 99};
        
        /* Vector operation in main */
        v4si v = {1, 2, 3, 4};
        int sum = v[0] + v[1] + v[2] + v[3];
    }
    
    printf("Tests completed.\n");
    return 0;
}
