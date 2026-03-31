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
void test_mem_p_target() {
    /* MEM_P(target) true path - array elements in memory */
    int arr1[100];
    
    /* Single element access - count = 1 */
    arr1[42] = 100;  /* Line 7691-7700: const_bounds_p true, count=1 */
    
    /* Two element range - count = 2 */
    arr1[10] = 1;
    arr1[11] = 2;    /* Could be optimized as range operation */
    
    /* Designated initializer with constant range */
    int arr2[50] = {[10 ... 15] = 99};  /* count=6, MEM_P true */
    
    /* Larger range with char elements - triggers type size calculation */
    char buf1[200] = {[20 ... 40] = 'A'};  /* count=21, char size=1 */
    
    /* Structure member access */
    struct ArrayStruct s1;
    s1.data[5] = 123;  /* Constant index, MEM_P true */
    s1.buffer[10] = 'x';
}

void test_non_mem_p_target() {
    /* Non-MEM_P(target) path - results in registers */
    int arr[100] = {0};
    
    /* Expression favoring register allocation */
    int reg_result = arr[10] + arr[20] * arr[30];  /* Line 7691-7700: !MEM_P */
    
    /* Conditional with constant indices */
    int x = (arr[0] > 0) ? arr[1] : arr[2];
    
    /* Nested array access with constant outer index */
    int nested[10][10] = {{0}};
    int val = nested[3][4];  /* Constant indices */
    
    /* String literal with constant index */
    char c = "constant_string"[7];  /* Constant bounds */
    
    printf("Register results: %d %d %c\n", reg_result, val, c);
}

void test_vector_operations() {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Vector element access with constant index */
    int elem = vec1[2];  /* Line 7691-7700: const_bounds_p true */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;
    int sum = vec3[0] + vec3[3];  /* Constant indices */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    
    /* Small vector of chars */
    v16qi char_vec = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char vchar = char_vec[10];  /* Constant index */
    
    printf("Vector: %d %d %c\n", elem, sum, vchar);
}

void test_constant_bounds_loops() {
    /* Loops with constant bounds that might be unrolled */
    int arr[10];
    
    /* Small constant loop - might trigger count <= 2 logic when unrolled */
    for (int i = 0; i < 2; i++) {
        arr[i] = i * 10;
    }
    
    /* Medium loop - count > 2 */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 20;
    }
    
    /* Designated init with exactly 2 elements */
    int two_elem[10] = {[3] = 100, [4] = 200};  /* Two-element range */
    
    /* Three element range */
    int three_elem[10] = {[5 ... 7] = 300};  /* count=3 */
}

void test_switch_array_indexing() {
    /* Switch with array indexing - constant indices in cases */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int index = 2;
    
    switch (index) {
        case 0:
            printf("Value: %d\n", arr[0]);  /* Constant index */
            break;
        case 1:
            printf("Value: %d\n", arr[1]);  /* Constant index */
            break;
        case 2:
            printf("Value: %d\n", arr[2]);  /* Constant index */
            break;
        default:
            printf("Default: %d\n", arr[9]);  /* Constant index */
    }
}

void test_builtin_constant_p() {
    /* Use __builtin_constant_p to force constant evaluation */
    int arr[10] = {0};
    
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        printf("Constant array access\n");
    }
    
    /* Constant index with __builtin_constant_p */
    int idx = 3;
    if (__builtin_constant_p(idx)) {
        int val = arr[idx];
        printf("Value: %d\n", val);
    }
}

void test_mixed_expressions() {
    /* Complex expressions combining multiple patterns */
    int arr[20];
    
    /* Multiple array accesses in one expression */
    arr[arr[2]] = arr[3] + arr[4] * arr[5];
    
    /* Conditional with different constant indices */
    int x = (arr[0] > 0) ? arr[1] : arr[2];
    
    /* Chain of array accesses */
    int y = arr[arr[arr[1]]];
    
    /* Mixed types with constant bounds */
    struct ArrayStruct s;
    s.data[5] = s.buffer[10] + arr[15];
    
    printf("Mixed: %d %d\n", x, y);
}

void test_small_type_large_count() {
    /* Trigger count > 2 with small element type size */
    char small_arr[1000];
    
    /* Large constant range with char elements */
    for (int i = 0; i < 100; i++) {
        small_arr[i] = i % 256;
    }
    
    /* Designated init with large range */
    char buf[500] = {[100 ... 200] = 'Z'};  /* count=101, char size=1 */
    
    /* Exactly boundary case: count = 3 */
    char three_chars[10] = {[2 ... 4] = 'A'};  /* count=3 */
}

int main() {
    printf("Testing constant bounds array operations...\n");
    
    /* Execute all test functions to cover different paths */
    test_mem_p_target();
    test_non_mem_p_target();
    test_vector_operations();
    test_constant_bounds_loops();
    test_switch_array_indexing();
    test_builtin_constant_p();
    test_mixed_expressions();
    test_small_type_large_count();
    
    /* Additional inline tests in main */
    {
        /* String literal indexing */
        const char *str = "Hello World";
        char chars[5];
        chars[0] = str[0];  /* Constant index */
        chars[1] = str[6];  /* Constant index */
        chars[2] = str[10]; /* Constant index */
        
        /* Pointer arithmetic with constant bounds */
        int array[50];
        int *ptr = &array[10];  /* Constant offset */
        ptr[5] = 42;  /* Constant index from pointer */
        
        /* Multi-dimensional with constant indices */
        int matrix[5][5];
        matrix[2][3] = 99;  /* Both indices constant */
        
        printf("Final checks complete\n");
    }
    
    return 0;
}
