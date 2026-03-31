/* Test program to cover expr.cc lines 7691-7700 */
#include <stddef.h>

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
    char data[];
};

/* Test functions to cover different paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;                    /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;           /* Single element access */
    
    /* Two-element range in designated initializer */
    int arr2[10] = {[2] = 10, [3] = 20};  /* Two separate elements */
    
    /* Two consecutive elements */
    struct ArrayStruct s1;
    s1.data[5] = 100;
    s1.data[6] = 200;                /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count = 11 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars - count = 11 */
    
    /* Larger range with char type */
    char buf3[200];
    for (int i = 0; i < 30; i++) {   /* Constant bound 30 */
        buf3[i] = i;
    }
}

void test_register_target_paths(void) {
    /* Path: MEM_P(target) = false (result in register) */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Results likely go to registers */
    int reg1 = arr[2];               /* Single element to register */
    int reg2 = arr[3] + arr[4];      /* Expression result to register */
    int reg3 = arr[5] * 2;           /* Arithmetic result to register */
    
    /* Conditional with constant indices */
    int cond = 1;
    int reg4 = (cond ? arr[6] : arr[7]);  /* Both branches constant indices */
    
    /* Nested array access with constant outer index */
    int reg5 = arr[arr[8] & 0x7];    /* Outer index from array, masked to constant range */
}

void test_vector_operations(void) {
    /* Vector extensions with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem1 = vec1[0];             /* Constant index 0 */
    int elem2 = vec1[2];             /* Constant index 2 */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;         /* Vector add, result likely in register */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si vec4 = vec1 * mask;         /* Vector multiply with constant mask */
    
    /* Character vector with many elements */
    v16c char_vec = {
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15
    };
    char c1 = char_vec[5];           /* Constant index 5 */
    char c2 = char_vec[10];          /* Constant index 10 */
}

void test_constant_string_ops(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[4];      /* Constant index 4 */
    char c2 = "test string"[7];      /* Constant index 7 */
    
    /* Multiple character extraction */
    const char *str = "constant string";
    char buf[5];
    buf[0] = str[0];                 /* Series of constant accesses */
    buf[1] = str[1];
    buf[2] = str[2];
    buf[3] = str[3];
    buf[4] = '\0';
}

void test_switch_array_access(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int selector = 2;
    
    /* Switch with constant array indices */
    switch (selector) {
        case 0:
            selector = arr[1];       /* Constant index 1 */
            break;
        case 1:
            selector = arr[2];       /* Constant index 2 */
            break;
        case 2:
            selector = arr[3];       /* Constant index 3 */
            break;
        case 3:
            selector = arr[4];       /* Constant index 4 */
            break;
        default:
            selector = arr[0];       /* Constant index 0 */
    }
}

void test_builtin_constant_p(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Force constant evaluation with __builtin_constant_p */
    if (__builtin_constant_p(arr[2])) {
        int x = arr[2] + 10;         /* Constant index 2 */
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(arr[4])) {
        int y = arr[4] * 2;          /* Constant index 4 */
    }
}

void test_mixed_operations(void) {
    /* Complex expression combining multiple patterns */
    struct ArrayStruct s;
    int arr[20];
    
    /* Mixed memory and register targets */
    s.data[5] = arr[3] + arr[4];     /* arr[3], arr[4] to registers, then to memory */
    
    /* Multiple constant ranges */
    int init_arr[10] = {
        [0 ... 2] = 1,               /* 3 elements */
        [3 ... 5] = 2,               /* 3 elements */
        [6 ... 9] = 3                /* 4 elements */
    };
    
    /* Nested constant indexing */
    int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int val = matrix[1][2];          /* Constant indices 1 and 2 */
    
    /* Pointer arithmetic with constant bounds */
    int *ptr = &arr[0];
    ptr += 5;                        /* Constant offset 5 */
    *ptr = 100;
}

int main(void) {
    /* Execute all test functions to trigger different code paths */
    test_mem_target_paths();
    test_register_target_paths();
    test_vector_operations();
    test_constant_string_ops();
    test_switch_array_access();
    test_builtin_constant_p();
    test_mixed_operations();
    
    return 0;
}
