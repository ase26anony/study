#include <stddef.h>

/* Vector extensions for GCC */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* Struct with array for memory access patterns */
struct ArrayStruct {
    int data[20];
    char buffer[100];
};

/* Flexible array struct */
struct FlexStruct {
    int count;
    char data[];
};

/* Test function for different constant bound scenarios */
void test_constant_bounds(void) {
    /* 1. Designated initializers with constant ranges */
    int arr1[10] = {[2 ... 5] = 42};  /* count = 4, const bounds */
    int arr2[10] = {[3] = 1, [4] = 2}; /* count = 2, const bounds */
    int arr3[10] = {[7] = 99};         /* count = 1, const bounds */
    
    /* 2. Vector extensions with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    int vec_elem = vec1[2];  /* constant index 2 */
    
    v16c vec2 = {0};
    char vec_char = vec2[5];  /* constant index 5 */
    
    /* 3. Array slicing in structs */
    struct ArrayStruct s;
    int *slice1 = &s.data[2];   /* constant start index */
    int *slice2 = &s.data[5];   /* constant start index */
    char *slice3 = &s.buffer[10]; /* constant start index */
    
    /* 4. Constant string indexing */
    char c1 = "hello"[2];      /* constant index 2 */
    char c2 = "test"[0];       /* constant index 0 */
    
    /* 5. For MEM_P(target) true path - memory operations */
    arr1[3] = 42;              /* memory target with constant index */
    s.data[5] = 100;           /* struct field with constant index */
    
    /* 6. For count <= 2 path */
    int arr4[10] = {[3] = 1, [4] = 2};  /* exactly 2 elements */
    int single = arr4[5];                /* single element access */
    
    /* 7. For count > 2 path with small element types */
    char buf[100] = {[10 ... 20] = 'x'};  /* 11 elements, char size 1 */
    char buf2[50] = {[5 ... 15] = 'a'};   /* 11 elements */
    
    /* 8. For non-MEM_P(target) path - register operations */
    int reg1 = arr1[2] + arr1[3];      /* result likely in register */
    int reg2 = vec1[0] * vec1[1];      /* vector elements to register */
    
    /* 9. Complex expressions with constant bounds */
    int x = arr1[arr2[3]];             /* nested with inner constant */
    int y = (vec_elem > 0) ? arr1[2] : arr1[3]; /* conditional with constants */
    
    /* 10. Loop with constant bounds (may be unrolled) */
    for (int i = 0; i < 5; i++) {
        arr1[i] = i * 2;
    }
    
    /* 11. Switch with constant array indexing */
    int idx = 2;
    switch (idx) {
        case 0: x = arr1[1]; break;
        case 1: x = arr1[2]; break;
        case 2: x = arr1[3]; break;
        default: x = arr1[0];
    }
    
    /* 12. Vector operations with constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si res = vec1 * mask;  /* vector operation with constant mask */
    
    /* 13. Using __builtin_constant_p with array bounds */
    if (__builtin_constant_p(arr1[5])) {
        /* Force constant evaluation */
        int test = arr1[5];
    }
    
    /* 14. Multi-dimensional array with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i + j;
        }
    }
    int diag = matrix[2][2];  /* constant indices */
    
    /* 15. Pointer arithmetic with constant bounds */
    int *ptr = arr1;
    int val1 = *(ptr + 3);    /* constant offset */
    int val2 = ptr[4];        /* constant index */
    
    /* Prevent unused variable warnings */
    (void)vec_elem;
    (void)vec_char;
    (void)slice1;
    (void)slice2;
    (void)slice3;
    (void)c1;
    (void)c2;
    (void)single;
    (void)reg1;
    (void)reg2;
    (void)x;
    (void)y;
    (void)res;
    (void)diag;
    (void)val1;
    (void)val2;
}

/* Additional test with larger constant ranges */
void test_large_ranges(void) {
    /* Large constant range with char (small element size) */
    char large_buf[256] = {[50 ... 100] = 'z'};  /* 51 elements */
    
    /* Medium range with int */
    int medium_arr[50] = {[10 ... 30] = 255};  /* 21 elements */
    
    /* Very small range */
    int tiny_arr[10] = {[5 ... 6] = 42};  /* 2 elements */
    
    /* Single element with constant index */
    int single_arr[10];
    single_arr[7] = 99;
    
    /* Prevent unused warnings */
    (void)large_buf;
    (void)medium_arr;
    (void)tiny_arr;
    (void)single_arr;
}

/* Test with different optimization contexts */
static inline int inline_test(int *arr) {
    /* Inline function with constant indexing */
    return arr[2] + arr[3];  /* May use registers */
}

/* Main function to execute all tests */
int main(void) {
    int test_arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Execute all test patterns */
    test_constant_bounds();
    test_large_ranges();
    
    /* Test inline function with constant indices */
    int result = inline_test(test_arr);
    
    /* Additional constant-bound operations in main */
    char str[] = "constant";
    for (int i = 0; i < 8; i++) {  /* constant bound 8 */
        str[i] = str[i] + 1;
    }
    
    /* Array section copy with constant bounds */
    int src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int dst[10];
    for (int i = 2; i < 7; i++) {  /* constant bounds 2..6 */
        dst[i] = src[i];
    }
    
    /* Vector operation with constant indices */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    int sum = c[0] + c[1] + c[2] + c[3];  /* all constant indices */
    
    return result + sum;
}
