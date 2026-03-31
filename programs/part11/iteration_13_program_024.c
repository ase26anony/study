/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>
#include <string.h>

/* Vector extensions for GCC */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8s __attribute__((vector_size(16)));

/* Struct with array member for constant bounds access */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

/* Struct with flexible array member */
struct FlexStruct {
    int count;
    char items[];
};

/* Test function 1: Designated initializers with constant ranges */
void test_designated_init(void) {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[2] = 1, [3] = 2};
    
    /* Range > 2 with char type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Range > 2 with int type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 elements */
    
    /* Multiple ranges */
    int arr4[30] = {[0 ... 4] = 1, [10 ... 14] = 2, [20 ... 24] = 3};
}

/* Test function 2: Vector operations with constant indices */
void test_vector_ops(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant index access - non-MEM_P target (register) */
    int elem1 = vec1[2];  /* Constant index 2 */
    
    /* Vector operations with constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si res1 = vec1 * mask;  /* Constant mask */
    
    /* Vector conditional with constant indices */
    v4si cmp = vec1 > vec2;
    int cmp_result = cmp[1];  /* Constant index */
    
    /* Larger vector with char elements */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = chars[5];  /* Constant index */
    char c2 = chars[10]; /* Constant index */
}

/* Test function 3: Array slicing in structs */
void test_struct_array_slicing(void) {
    struct ArrayStruct s = {0};
    
    /* Constant start index - count = 1 */
    int *p1 = &s.data[5];
    *p1 = 100;
    
    /* Constant start index - count = 2 (implicit through two assignments) */
    s.data[2] = 10;
    s.data[3] = 20;
    
    /* String literal with constant indices */
    char c1 = "constant_string"[7];
    char c2 = "another"[3];
    
    /* Array section copy with constant bounds */
    int src[10] = {0,1,2,3,4,5,6,7,8,9};
    memcpy(&s.data[5], &src[2], 3 * sizeof(int));  /* 3 elements */
}

/* Test function 4: Complex expressions with constant bounds */
int test_complex_expressions(int cond) {
    int arr[20] = {[0 ... 19] = 0};
    
    /* Nested array access with constant inner index */
    arr[0] = 5;
    int x = arr[arr[0]];  /* arr[5] */
    
    /* Conditional array access with constant indices */
    int y = cond ? arr[2] : arr[3];
    
    /* Array access in arithmetic expression */
    int z = arr[4] + arr[5] * arr[6];
    
    /* Switch with constant array indices */
    switch(cond) {
        case 0: return arr[1];
        case 1: return arr[2];
        case 2: return arr[3];
        default: return arr[4];
    }
}

/* Test function 5: Loop with constant bounds (may be unrolled) */
void test_constant_loops(void) {
    int arr[10];
    
    /* Small loop - might trigger count <= 2 if unrolled */
    for (int i = 0; i < 2; i++) {
        arr[i] = i * 10;
    }
    
    /* Medium loop - count > 2 */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 5;
    }
    
    /* Loop with char array - small element size */
    char chars[20];
    for (int i = 0; i < 8; i++) {
        chars[i] = 'A' + i;
    }
}

/* Test function 6: Builtin constant checks */
void test_builtin_constants(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        arr[5] = 1;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(5) && __builtin_constant_p(8)) {
        for (int i = 5; i <= 8; i++) {
            arr[i] = i;
        }
    }
}

/* Test function 7: Mixed operations targeting different paths */
void test_mixed_operations(void) {
    struct ArrayStruct s1, s2;
    
    /* MEM_P target path with count = 1 */
    s1.data[7] = 42;
    
    /* MEM_P target path with count = 2 */
    s1.data[8] = 43;
    s1.data[9] = 44;
    
    /* MEM_P target path with count > 2 and small element size */
    for (int i = 0; i < 10; i++) {
        s1.buffer[i] = 'a' + i;
    }
    
    /* Non-MEM_P target (register result) */
    int sum = s1.data[1] + s1.data[2] + s1.data[3];
    
    /* Vector to memory with constant indices */
    v4si vec = {1, 2, 3, 4};
    int *dest = &s2.data[5];
    dest[0] = vec[0];
    dest[1] = vec[1];
    dest[2] = vec[2];
}

/* Test function 8: String operations with constant indices */
void test_string_ops(void) {
    const char *str = "Hello, World!";
    
    /* Multiple constant index accesses */
    char a = str[0];
    char b = str[7];
    char c = str[12];
    
    /* String compare with constant offsets */
    int cmp1 = strncmp(str + 1, "ello", 4);
    int cmp2 = strncmp(str + 7, "World", 5);
    
    /* Character array with constant range initialization */
    char message[50] = {[0 ... 9] = '-', [10 ... 19] = '*', [20 ... 29] = '+'};
}

/* Main function combining all tests */
int main(void) {
    /* Execute all test functions */
    test_designated_init();
    test_vector_ops();
    test_struct_array_slicing();
    
    int result = test_complex_expressions(1);
    
    test_constant_loops();
    test_builtin_constants();
    test_mixed_operations();
    test_string_ops();
    
    /* Additional inline tests for coverage */
    {
        /* Direct array access patterns */
        int direct_arr[100];
        
        /* Single element - MEM_P target, count = 1 */
        direct_arr[50] = 1000;
        
        /* Two adjacent elements - MEM_P target, count = 2 */
        direct_arr[60] = 2000;
        direct_arr[61] = 2001;
        
        /* Range > 2 with byte elements */
        char byte_arr[256];
        for (int i = 100; i <= 110; i++) {
            byte_arr[i] = i % 256;
        }
        
        /* Register-targeting operations */
        int reg1 = direct_arr[10];
        int reg2 = direct_arr[11];
        int reg_sum = reg1 + reg2;
        int reg_prod = direct_arr[12] * direct_arr[13];
        
        /* Constant index chain */
        int idx = direct_arr[1];
        int val = direct_arr[idx];  /* idx is runtime, but array access has constant addressing */
    }
    
    return 0;
}
