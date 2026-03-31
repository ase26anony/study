/* Test program to exercise constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Vector extensions for GCC */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

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
    
    /* Two-element range in designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count = 11 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars - count = 11 */
    
    /* Path: MEM_P(target) = true, count > 2, larger element type */
    int arr3[50] = {[10 ... 15] = 99};     /* 6 ints - count = 6 */
}

void test_non_mem_target_paths(void) {
    /* Path: MEM_P(target) = false (results in registers) */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* These likely go to registers */
    int reg1 = arr[2] + arr[3];      /* Constant indices 2 and 3 */
    int reg2 = arr[4] * arr[5];      /* Constant indices 4 and 5 */
    int reg3 = arr[1] - arr[6];      /* Constant indices 1 and 6 */
    
    /* Complex expression with constant indexing */
    int reg4 = arr[arr[2]];          /* Nested: outer index from arr[2] = 2 */
    int reg5 = (reg1 > 0) ? arr[7] : arr[8];  /* Conditional with constant indices */
    
    /* Use results to avoid dead code elimination */
    printf("Reg results: %d %d %d %d %d\n", reg1, reg2, reg3, reg4, reg5);
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
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    int elem4 = masked[1];    /* Constant index 1 */
    
    /* Small element vector */
    v16qi vchar = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                   'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    char celem = vchar[10];   /* Constant index 10 */
    
    printf("Vector elements: %d %d %d %d %c\n", elem1, elem2, elem3, elem4, celem);
}

void test_string_and_struct_operations(void) {
    /* Constant string indexing */
    char c1 = "hello world"[6];      /* Constant index 6 = 'w' */
    char c2 = "test string"[3];      /* Constant index 3 = 't' */
    
    /* Array slicing through struct */
    struct ArrayStruct s;
    int *slice1 = &s.data[5];        /* Constant start index 5 */
    char *slice2 = &s.buffer[10];    /* Constant start index 10 */
    
    /* Initialize slices with constant bounds */
    for (int i = 0; i < 5; i++) {    /* Constant bound 5 */
        slice1[i] = i * 10;
    }
    
    /* Designated initializer with range in struct member */
    struct ArrayStruct s2 = {
        .data = {[2 ... 7] = 100},   /* 6 elements - count = 6 */
        .buffer = {[10 ... 25] = 'z'} /* 16 elements - count = 16 */
    };
    
    printf("String chars: %c %c\n", c1, c2);
    printf("Struct slice[2]: %d\n", s2.data[2]);
}

void test_constant_bounds_loops(void) {
    /* Loops with constant bounds that might be unrolled */
    int arr1[10];
    
    /* Small constant bound loop */
    for (int i = 0; i < 3; i++) {    /* Constant bound 3 */
        arr1[i] = i * 2;
    }
    
    /* Medium constant bound loop */
    int arr2[20];
    for (int i = 0; i < 8; i++) {    /* Constant bound 8 */
        arr2[i] = i * 3;
    }
    
    /* Nested loops with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {    /* Constant bound 5 */
        for (int j = 0; j < 5; j++) { /* Constant bound 5 */
            matrix[i][j] = i * j;
        }
    }
    
    /* Switch with array indexing */
    int arr3[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int idx = 3;
    int result;
    
    switch (idx) {
        case 0: result = arr3[1]; break;  /* Constant index 1 */
        case 1: result = arr3[2]; break;  /* Constant index 2 */
        case 2: result = arr3[3]; break;  /* Constant index 3 */
        case 3: result = arr3[4]; break;  /* Constant index 4 */
        default: result = arr3[0]; break; /* Constant index 0 */
    }
    
    printf("Switch result: %d\n", result);
}

void test_compiler_builtins(void) {
    /* Use __builtin_constant_p to force constant evaluation */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    if (__builtin_constant_p(arr[5])) {
        printf("arr[5] is constant: %d\n", arr[5]);
    }
    
    /* Test with constant index */
    if (__builtin_constant_p("string"[3])) {
        printf("string[3] is constant: %c\n", "string"[3]);
    }
    
    /* Mixed constant and non-constant */
    int idx = 2;
    int val1 = arr[idx];      /* Non-constant index */
    int val2 = arr[2];        /* Constant index */
    
    printf("Mixed: %d (non-const idx) vs %d (const idx)\n", val1, val2);
}

void test_multi_dimensional_constant(void) {
    /* Multi-dimensional arrays with constant indices */
    int matrix[10][10];
    
    /* Row access with constant index */
    int *row3 = matrix[3];    /* Constant index 3 */
    
    /* Element access with constant indices */
    matrix[2][3] = 42;        /* Constant indices 2 and 3 */
    matrix[5][6] = matrix[2][3] * 2;
    
    /* Partial row initialization with constant range */
    int row4[10] = {[2 ... 5] = 99};  /* 4 elements - count = 4 */
    
    /* 2D slice */
    for (int i = 0; i < 4; i++) {     /* Constant bound 4 */
        for (int j = 0; j < 4; j++) { /* Constant bound 4 */
            matrix[i + 1][j + 1] = i * 10 + j;
        }
    }
}

int main(void) {
    printf("Testing constant bounds coverage for expr.cc lines 7691-7700\n\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_string_and_struct_operations();
    test_constant_bounds_loops();
    test_compiler_builtins();
    test_multi_dimensional_constant();
    
    printf("\nAll tests completed (compile-time coverage is the goal)\n");
    return 0;
}
