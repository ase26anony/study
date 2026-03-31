/* Test program to cover expr.cc lines 7691-7700 */
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
void test_mem_p_target() {
    /* MEM_P(target) true path - targeting memory locations */
    int arr1[10];
    
    /* Single element - count = 1 */
    arr1[3] = 42;  /* Constant index, targets memory */
    
    /* Two elements - count = 2 */
    arr1[4] = 43;
    arr1[5] = 44;
    
    /* Multiple elements with constant bounds */
    int arr2[100] = {[10 ... 20] = 99};  /* 11 elements, count > 2 */
    
    /* Structure member access */
    struct ArrayStruct s1;
    s1.data[5] = 100;  /* Constant index to struct array member */
    
    /* Pointer dereference with constant offset */
    int *ptr = arr1;
    ptr[2] = 55;  /* Constant index via pointer */
}

void test_non_mem_p_target() {
    /* Non-MEM_P(target) path - results likely go to registers */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Register operations with constant indices */
    int x = arr[2] + arr[3];  /* Sum likely in register */
    int y = arr[4] * arr[5];  /* Product likely in register */
    
    /* Conditional expression with constant indices */
    int z = (x > 0) ? arr[6] : arr[7];
    
    /* Function argument from array with constant index */
    printf("Value: %d\n", arr[8]);
}

void test_vector_operations() {
    /* Vector extensions with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem = vec1[2];  /* Constant index 2 */
    
    /* Vector operations that might trigger the logic */
    v4si result = vec1 + vec2;
    result[1] = 99;  /* Constant index assignment */
    
    /* Character vector with many elements */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c = chars[10];  /* Constant index */
}

void test_string_operations() {
    /* String literal with constant indexing */
    char c1 = "Hello World"[6];  /* Constant index 6 = 'W' */
    
    /* Array initialized with string */
    char strarr[] = "Constant bounds test";
    char c2 = strarr[5];  /* Constant index */
    
    /* Designated initializer with range for string */
    char buf[50] = {[10 ... 20] = 'X', [30 ... 35] = 'Y'};
    buf[0] = 'A';  /* Single element */
}

void test_count_paths() {
    /* Test count <= 2 path */
    int arr1[10] = {[3] = 100};  /* Single element, count = 1 */
    int arr2[10] = {[4] = 200, [5] = 300};  /* Two elements, count = 2 */
    
    /* Test count > 2 path with small element type */
    char small[100] = {[20 ... 40] = 'Z'};  /* 21 elements, count > 2, char size = 1 */
    
    /* Test count > 2 path with larger element type */
    int large[50] = {[10 ... 25] = 999};  /* 16 elements, count > 2, int size > 1 */
}

void test_complex_expressions() {
    int arr[20] = {[0 ... 19] = 0};
    
    /* Nested array access with constant inner index */
    int idx_arr[5] = {2, 3, 4, 5, 6};
    int x = arr[idx_arr[2]];  /* arr[4], constant inner index 2 */
    
    /* Loop with constant bounds (might be unrolled) */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 10;  /* Constant loop bound 5 */
    }
    
    /* Switch with constant array indices */
    int selector = 2;
    int result;
    switch (selector) {
        case 0: result = arr[1]; break;
        case 1: result = arr[2]; break;
        case 2: result = arr[3]; break;  /* Constant index 3 */
        default: result = arr[0]; break;
    }
    
    /* Conditional with array indexing on both sides */
    int a = (selector > 1) ? arr[5] : arr[6];
}

void test_compiler_extensions() {
    int arr[10];
    
    /* __builtin_constant_p with array access */
    if (__builtin_constant_p(arr[5])) {
        /* Force compiler to consider constant bounds */
        arr[5] = 42;
    }
    
    /* Vector with constant mask */
    v4si vec = {1, 2, 3, 4};
    v4si mask = {0, -1, 0, -1};  /* Constant vector mask */
    v4si masked = vec & mask;
    
    /* Multiple constant index operations in one expression */
    int x = arr[1] + arr[2] * arr[3] - arr[4];
}

/* Main function combining all tests */
int main() {
    printf("Testing constant bounds array operations...\n");
    
    /* Execute all test patterns */
    test_mem_p_target();
    test_non_mem_p_target();
    test_vector_operations();
    test_string_operations();
    test_count_paths();
    test_complex_expressions();
    test_compiler_extensions();
    
    /* Additional inline tests in main */
    struct ArrayStruct s;
    int local_arr[30];
    
    /* Mixed operations */
    local_arr[5] = 10;
    local_arr[6] = 20;
    local_arr[7] = 30;  /* Three consecutive elements */
    
    /* Constant bounds in function calls */
    memcpy(&local_arr[10], &local_arr[0], 5 * sizeof(int));  /* Constant size 5 */
    
    /* Designated initializers with multiple ranges */
    int init_arr[50] = {
        [0 ... 9] = 1,
        [10 ... 19] = 2,
        [20 ... 29] = 3,
        [40] = 99,  /* Single element */
        [41 ... 42] = 100  /* Two elements */
    };
    
    printf("Test completed.\n");
    return 0;
}
