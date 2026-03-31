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

/* Test functions covering different paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range in designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count = 11 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars - count = 11 */
    
    /* Path: MEM_P(target) true, count > 2, larger element type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 ints - count = 6 */
}

void test_non_mem_target_paths(void) {
    /* Path: non-MEM_P(target) - results in registers */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* These likely go to registers */
    int reg1 = arr[2] + arr[3];      /* Constant indices */
    int reg2 = arr[4] * arr[5];      /* Constant indices */
    int reg3 = (arr[6] > arr[7]) ? arr[6] : arr[7];  /* Conditional with constant indices */
    
    /* Use the results to avoid dead code elimination */
    printf("Register results: %d %d %d\n", reg1, reg2, reg3);
}

void test_vector_operations(void) {
    /* Vector extensions with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[2];      /* Constant index 2 */
    int elem2 = vec2[1];      /* Constant index 1 */
    
    /* Vector operations that might trigger the logic */
    v4si vec3 = vec1 + vec2;
    int elem3 = vec3[3];      /* Constant index 3 */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    int elem4 = masked[1];    /* Constant index 1 */
    
    printf("Vector elements: %d %d %d %d\n", elem1, elem2, elem3, elem4);
}

void test_string_and_struct_operations(void) {
    /* Constant string indexing */
    char c1 = "hello world"[4];      /* Constant index 4 */
    char c2 = "test string"[7];      /* Constant index 7 */
    
    /* Array slicing in structs */
    struct ArrayStruct s;
    int *slice1 = &s.data[5];        /* Constant start index 5 */
    char *slice2 = &s.buffer[10];    /* Constant start index 10 */
    
    /* Initialize slices with constant bounds */
    for (int i = 0; i < 3; i++) {    /* Loop with constant bound 3 */
        slice1[i] = i * 10;
        slice2[i] = 'a' + i;
    }
    
    printf("String chars: %c %c\n", c1, c2);
    printf("Struct slice: %d %c\n", slice1[1], slice2[1]);
}

void test_complex_expressions(void) {
    int arr[20] = {0};
    
    /* Nested array access with constant inner index */
    arr[0] = 5;
    int x = arr[arr[0]];  /* arr[5] - both indices involve constants */
    
    /* Switch with constant array indexing */
    int idx = 2;
    int result;
    switch (idx) {
        case 0: result = arr[1]; break;  /* Constant index 1 */
        case 1: result = arr[2]; break;  /* Constant index 2 */
        case 2: result = arr[3]; break;  /* Constant index 3 */
        default: result = arr[0]; break; /* Constant index 0 */
    }
    
    /* Conditional with constant array indices */
    int y = (x > 10) ? arr[4] : arr[5];  /* Constant indices 4 and 5 */
    
    /* Loop with constant bounds (might be unrolled) */
    for (int i = 0; i < 4; i++) {  /* Constant bound 4 */
        arr[10 + i] = i * 2;       /* Constant base index 10 */
    }
    
    printf("Complex results: %d %d %d\n", x, result, y);
}

void test_constant_folding_paths(void) {
    /* Use __builtin_constant_p to force constant evaluation */
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    if (__builtin_constant_p(arr[3])) {  /* Constant index 3 */
        printf("arr[3] is constant-foldable\n");
    }
    
    /* Designated initializer with larger constant range */
    int big_arr[50] = {[10 ... 30] = 255};  /* 21 elements - count = 21 */
    
    /* Mixed small and large element types */
    short shorts[100] = {[20 ... 40] = 100};  /* 21 shorts - count = 21 */
    long longs[20] = {[5 ... 10] = 999};      /* 6 longs - count = 6 */
    
    /* Very small element type with large count */
    unsigned char bytes[256] = {[32 ... 127] = 0xAA};  /* 96 bytes - count = 96 */
}

int main(void) {
    printf("Testing constant bounds paths in expr.cc\n\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_string_and_struct_operations();
    test_complex_expressions();
    test_constant_folding_paths();
    
    printf("\nAll tests completed (coverage occurs during compilation)\n");
    return 0;
}
