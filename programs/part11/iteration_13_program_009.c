/* Test program to exercise constant-bounds array/vector operations in GCC's expr.cc */
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
    char data[];  /* Flexible array member */
};

/* Test functions covering different paths */
void test_mem_p_target(void) {
    /* MEM_P(target) true path - targeting memory locations */
    int arr1[20];
    
    /* Single element - count = 1 */
    arr1[5] = 42;  /* Constant index, targets memory */
    
    /* Two elements - count = 2 */
    arr1[3] = 10;
    arr1[4] = 20;  /* Two-element range */
    
    /* Larger range with small elements - count > 2, type size calculation */
    char buf1[100];
    for (int i = 10; i <= 20; i++) {  /* Constant bounds: 10 to 20 */
        buf1[i] = 'x';  /* 11 elements, char size 1 */
    }
    
    /* Designated initializer with constant range */
    int arr2[10] = {[2 ... 5] = 99};  /* 4 elements, constant range 2-5 */
    
    /* Nested array access with constant inner index */
    int idx = arr1[2];  /* Constant index 2 */
    arr1[idx] = 100;    /* May become constant after propagation */
}

void test_non_mem_p_target(void) {
    /* Non-MEM_P(target) path - results in registers */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Expression favoring register allocation */
    int reg_result = arr[2] + arr[3];  /* Both constant indices, result in register */
    
    /* Conditional with constant indices */
    int cond = 1;
    int y = (cond ? arr[2] : arr[3]);  /* Constant indices in both branches */
    
    /* Complex expression with constant indexing */
    int x = arr[arr[2]];  /* Outer index from array, inner constant */
    
    /* Prevent optimization from removing computations */
    volatile int sink = reg_result + x + y;
    (void)sink;
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem = vec1[2];  /* Constant index 2 */
    
    /* Vector operations that might expand to memory */
    v4si result = vec1 + vec2;
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    
    /* Small vector type with many elements */
    v16c chars = {0};
    chars[5] = 'A';  /* Constant index */
    chars[6] = 'B';  /* Two-element range */
    
    /* Prevent dead code elimination */
    volatile int sink = elem + result[0] + masked[1] + chars[7];
    (void)sink;
}

void test_struct_operations(void) {
    /* Array in struct with constant indexing */
    struct ArrayStruct s;
    
    /* Constant index into struct array */
    s.data[5] = 100;  /* Single element */
    s.data[3] = 200;
    s.data[4] = 300;  /* Two-element range */
    
    /* Pointer to sub-range with constant start */
    int *p = &s.data[2];  /* Constant start index 2 */
    
    /* String literal with constant indexing */
    char c = "hello world"[6];  /* Constant index 6 */
    
    /* Array section copy with constant bounds */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        s.buffer[i] = 'a' + i;
    }
    
    /* Prevent optimization */
    volatile int sink = *p + c;
    (void)sink;
}

void test_switch_and_loops(void) {
    int arr[10] = {0};
    
    /* Switch with constant array indexing */
    int selector = 1;
    switch (selector) {
        case 0: arr[1] = 10; break;  /* Constant index 1 */
        case 1: arr[2] = 20; break;  /* Constant index 2 */
        case 2: arr[3] = 30; break;  /* Constant index 3 */
    }
    
    /* Loop with constant bounds (might be unrolled) */
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

void test_constant_folding(void) {
    /* Force constant evaluation with builtins */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Check if array access is constant */
    if (__builtin_constant_p(arr[5])) {
        /* This branch may be taken during compilation */
        volatile int x = arr[5];
        (void)x;
    }
    
    /* Constant expression with array indexing */
    int y = 2 + arr[3] * 4;  /* arr[3] might become constant 3 */
    
    /* Prevent dead code */
    volatile int sink = y;
    (void)sink;
}

void test_mixed_operations(void) {
    /* Combine multiple patterns in complex expressions */
    struct ArrayStruct s1, s2;
    
    /* Cross-struct array operations with constant indices */
    s1.data[2] = s2.data[3] + 10;
    
    /* Multiple constant ranges */
    int arr[20];
    for (int i = 2; i <= 7; i++) {  /* Range 2-7 (6 elements) */
        arr[i] = i * 10;
    }
    
    /* Mixed element sizes */
    short shorts[50];
    for (int i = 10; i <= 25; i++) {  /* 16 elements, short size 2 */
        shorts[i] = i;
    }
    
    /* Very small type, large count */
    unsigned char bytes[256];
    for (int i = 0; i < 100; i++) {  /* 100 elements, char size 1 */
        bytes[i] = i & 0xFF;
    }
}

int main(void) {
    /* Execute all test functions */
    test_mem_p_target();
    test_non_mem_p_target();
    test_vector_operations();
    test_struct_operations();
    test_switch_and_loops();
    test_constant_folding();
    test_mixed_operations();
    
    return 0;
}
