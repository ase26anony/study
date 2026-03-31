/* Test program to exercise constant bounds array/vector operations
   targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* Vector extensions for GCC */
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

/* Test functions for different scenarios */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, various count values */
    
    /* Single element - count = 1 */
    int arr1[10];
    arr1[5] = 42;  /* Constant index, memory target */
    
    /* Two elements - count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Designated init with constant indices */
    
    /* Multiple elements - count > 2 with small type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements, char size = 1 */
    
    /* Multiple elements - count > 2 with larger type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 elements, int size = 4 */
    
    /* Nested array access with constant outer index */
    int matrix[5][5];
    matrix[2][3] = arr1[arr2[3]];  /* Mixed constant and variable indices */
}

void test_register_target_paths(void) {
    /* Path: MEM_P(target) = false (result in register) */
    
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Results likely go to registers */
    int reg1 = arr[2] + arr[3];  /* Constant indices, arithmetic result in register */
    int reg2 = arr[4] * arr[5];
    
    /* Conditional with constant indices */
    int cond = 1;
    int reg3 = (cond ? arr[2] : arr[3]);  /* Both branches have constant indices */
    
    /* Function return with constant indexing */
    reg3 += arr[arr[2]];  /* Nested: outer index from array, inner constant */
}

void test_vector_operations(void) {
    /* Vector extension tests */
    
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[2];  /* Constant index 2 */
    vec1[3] = 42;         /* Constant index 3, memory target */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;  /* Result likely in vector register */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;  /* Constant mask vector */
    
    /* Small element vector */
    v16c char_vec = {[0 ... 15] = 1};
    char_vec[10] = 5;  /* Constant index, char element */
}

void test_constant_bounds_loops(void) {
    /* Loops with constant bounds that might be unrolled */
    
    int arr[10];
    
    /* Small constant loop - might trigger count <= 2 after unrolling? */
    for (int i = 0; i < 2; i++) {
        arr[i] = i * 10;
    }
    
    /* Larger constant loop */
    for (int i = 0; i < 5; i++) {
        arr[i + 2] = i * 20;  /* Constant offset */
    }
}

void test_switch_array_access(void) {
    /* Switch with constant array indexing */
    
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int x = 2;
    
    switch (x) {
        case 0:
            x = arr[1];  /* Constant index 1 */
            break;
        case 1:
            x = arr[2];  /* Constant index 2 */
            break;
        case 2:
            x = arr[3];  /* Constant index 3 */
            break;
        default:
            x = arr[0];  /* Constant index 0 */
    }
}

void test_struct_array_access(void) {
    /* Array access through structures */
    
    struct ArrayStruct s = {0};
    
    /* Constant indexing into struct array member */
    s.data[5] = 100;  /* Constant index 5 */
    s.buffer[10] = 'A';  /* Constant index 10, char type */
    
    /* Pointer to sub-range with constant start */
    int *p = &s.data[2];  /* Constant offset 2 */
    *p = 50;
    
    /* Multiple element range in struct */
    for (int i = 0; i < 3; i++) {
        s.buffer[i + 5] = 'a' + i;  /* Constant base offset 5 */
    }
}

void test_builtin_constant(void) {
    /* Using __builtin_constant_p to force constant evaluation */
    
    int arr[10] = {0};
    
    if (__builtin_constant_p(arr[5])) {
        /* This might influence compiler's constant analysis */
        arr[5] = 1;
    }
    
    /* Constant index in builtin */
    if (__builtin_constant_p(5)) {
        arr[5] = 2;
    }
}

void test_string_literals(void) {
    /* String literal indexing */
    
    char c1 = "hello"[2];  /* Constant index 2 -> 'l' */
    char c2 = "world"[4];  /* Constant index 4 -> 'd' */
    
    /* String literal in expression */
    int len = "test"[3] - 'a';  /* Constant index 3 -> 't' */
    
    /* Modify array based on string literal index */
    char buffer[10];
    buffer[0] = "ABCD"[1];  /* Constant index 1 -> 'B' */
}

void test_mixed_operations(void) {
    /* Complex expressions combining multiple patterns */
    
    int arr[20] = {[0 ... 19] = 0};
    struct ArrayStruct s = {0};
    
    /* Mixed memory/register targets with constant bounds */
    int temp = arr[5] + s.data[3];  /* Both memory accesses, result in register */
    
    /* Chain of constant-index operations */
    arr[arr[2]] = s.data[4] + temp;  /* Nested constant indices */
    
    /* Conditional with different array types */
    int cond = arr[1] > 0;
    char result = cond ? s.buffer[5] : "fixed"[2];  /* Struct array vs string literal */
    
    /* Loop with constant bounds and struct access */
    for (int i = 0; i < 4; i++) {
        s.data[i + 6] = arr[i + 1] * 2;  /* Constant offsets in both arrays */
    }
}

int main(void) {
    /* Execute all test functions to trigger various code paths */
    
    test_mem_target_paths();
    test_register_target_paths();
    test_vector_operations();
    test_constant_bounds_loops();
    test_switch_array_access();
    test_struct_array_access();
    test_builtin_constant();
    test_string_literals();
    test_mixed_operations();
    
    return 0;
}
