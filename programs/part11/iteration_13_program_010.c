/* Test program to exercise constant-bounds array/vector operations in GCC's expr.cc */
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
    char data[];  /* Zero-length array */
};

/* Test functions for different scenarios */
void test_mem_target_paths(void) {
    /* MEM_P(target) true path - targeting memory locations */
    int arr1[20];
    
    /* Single element - count = 1 */
    arr1[5] = 42;  /* Constant index, targets memory */
    
    /* Two elements - count = 2 */
    arr1[3] = 10;
    arr1[4] = 20;  /* Two-element range with constant indices */
    
    /* Designated initializer with constant range - count > 2 */
    int arr2[30] = {[10 ... 15] = 99};  /* 6 elements, constant bounds */
    
    /* Larger range with char elements - triggers type size calculation */
    char buf1[100] = {[20 ... 35] = 'A'};  /* 16 chars, size 1 each */
    
    /* Even larger range with small type */
    char buf2[200] = {[50 ... 100] = 'B'};  /* 51 chars */
}

void test_non_mem_target_paths(void) {
    /* Non-MEM_P(target) path - results likely go to registers */
    int arr[20] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Register-targeting operations with constant indices */
    int x = arr[2] + arr[3];  /* Sum likely in register */
    int y = arr[5] * arr[6];  /* Product likely in register */
    
    /* Complex expression with constant indexing */
    int z = arr[arr[2]] + arr[arr[3]];  /* Nested constant indexing */
    
    /* Conditional with constant indices */
    int w = (x > 0) ? arr[1] : arr[2];
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem = vec1[2];  /* Constant index 2 */
    
    /* Vector operations that might trigger the logic */
    v4si vec3 = vec1 + vec2;
    int sum = vec3[0] + vec3[1] + vec3[2] + vec3[3];
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    
    /* Char vector with many elements */
    v16qi char_vec = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c = char_vec[10];  /* Constant index */
}

void test_struct_array_access(void) {
    struct ArrayStruct s;
    
    /* Constant indexing into struct array members */
    s.data[5] = 100;  /* Single element */
    s.data[10] = 200;
    s.data[11] = 300;  /* Two-element range */
    
    /* Pointer to sub-range with constant start */
    int *p = &s.data[3];  /* Constant offset */
    p[0] = 1;
    p[1] = 2;  /* Two elements from constant base */
    
    /* Larger range in struct buffer */
    memset(&s.buffer[10], 'X', 15);  /* 15 chars from constant offset */
}

void test_string_operations(void) {
    /* String literal with constant indexing */
    char c1 = "Hello World!"[6];  /* Constant index 6 = 'W' */
    
    /* Multiple constant string indices */
    char msg[] = "Constant bounds testing";
    char c2 = msg[5];   /* 'a' */
    char c3 = msg[10];  /* 'b' */
    char c4 = msg[15];  /* 'd' */
    
    /* Build string with constant ranges */
    char template[50];
    template[0] = '[';
    for (int i = 1; i < 10; i++) {  /* Loop with constant bound */
        template[i] = 'A' + i;
    }
    template[10] = ']';
    template[11] = '\0';
}

void test_constant_bounds_loops(void) {
    int arr[20];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 8; i++) {  /* Constant bound 8 */
        arr[i] = i * 2;
    }
    
    /* Nested loops with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int x = 3;
    
    /* Switch with constant array indices */
    switch (x) {
        case 0: x = arr[1]; break;  /* Constant index 1 */
        case 1: x = arr[2]; break;  /* Constant index 2 */
        case 2: x = arr[3]; break;  /* Constant index 3 */
        case 3: x = arr[4]; break;  /* Constant index 4 */
        default: x = arr[0];
    }
    
    /* Another switch with different constant indices */
    switch (x % 3) {
        case 0: x = arr[5] + arr[6]; break;  /* Two constant indices */
        case 1: x = arr[7] - arr[8]; break;  /* Two constant indices */
        case 2: x = arr[9] * 2; break;       /* Single constant index */
    }
}

void test_builtin_constant_p(void) {
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        /* This path might be taken during compilation */
        int x = arr[5] * 2;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(arr[9])) {
        int y = arr[9] + 100;
    }
}

void test_mixed_operations(void) {
    /* Combine multiple patterns in complex expressions */
    struct ArrayStruct s1, s2;
    int arr[30];
    
    /* Complex expression with multiple constant indices */
    int result = s1.data[5] + 
                 arr[10] * 
                 (s2.data[3] > 0 ? arr[15] : arr[20]);
    
    /* Nested array accesses with constant and variable indices */
    int idx = 7;
    int val = arr[arr[idx]] + arr[5];  /* arr[idx] might be constant-folded */
    
    /* Multiple memory targets with constant bounds */
    for (int i = 0; i < 4; i++) {  /* Small constant bound */
        s1.data[i * 2] = i;      /* Constant stride */
        s2.data[i * 2 + 1] = i * 10;
    }
}

int main(void) {
    printf("Testing constant bounds array/vector operations...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_struct_array_access();
    test_string_operations();
    test_constant_bounds_loops();
    test_switch_with_array_indexing();
    test_builtin_constant_p();
    test_mixed_operations();
    
    printf("All tests completed (coverage occurs during compilation).\n");
    return 0;
}
