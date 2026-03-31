/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

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

/* Test function 1: Designated initializers with constant ranges */
void test_designated_init(void) {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* Larger range with char - count > 2, small element size */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Even larger range */
    int arr3[50] = {[10 ... 30] = 99};  /* 21 elements */
}

/* Test function 2: Vector operations with constant indices */
void test_vector_ops(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant indexing into vector (register target) */
    int x = a[2];  /* Non-MEM_P target likely */
    
    /* Vector operation with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si res = a * mask;  /* May trigger different path */
    
    /* Character vector with constant indexing */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c = chars[5];
}

/* Test function 3: Array slicing through pointers */
void test_array_slicing(void) {
    struct ArrayStruct s = {0};
    
    /* Constant start index - MEM_P target */
    int *p1 = &s.data[2];  /* Pointer to memory location */
    *p1 = 42;
    
    /* Multiple elements with constant bounds */
    int *p2 = &s.data[5];
    p2[0] = 1;  /* Single element access */
    p2[1] = 2;  /* Two elements total */
    
    /* Larger slice */
    char *buf_ptr = &s.buffer[10];
    for (int i = 0; i < 15; i++) {  /* Constant bound loop */
        buf_ptr[i] = 'A' + i;
    }
}

/* Test function 4: Complex expressions with constant bounds */
int test_complex_expr(int cond) {
    int arr[20] = {0};
    
    /* Initialize with constant range */
    for (int i = 0; i < 10; i++) {  /* Loop with constant bound */
        arr[i] = i * 2;
    }
    
    /* Nested array access with constant inner index */
    int x = arr[arr[2]];  /* arr[2] is constant index 2 */
    
    /* Conditional array access with constant indices */
    int y = (cond ? arr[3] : arr[4]);
    
    /* Switch with constant array indices */
    switch (cond) {
        case 0: return arr[1];
        case 1: return arr[2];
        case 2: return arr[3];
        default: return arr[0];
    }
}

/* Test function 5: String literal indexing */
void test_string_indexing(void) {
    /* Constant indexing into string literal */
    char c1 = "Hello, World!"[7];  /* 'W' */
    
    /* Multiple constant indices */
    char msg[] = "Constant bounds testing";
    char c2 = msg[5];
    char c3 = msg[10];
    
    /* Array of strings with constant indexing */
    const char *strings[] = {"first", "second", "third"};
    char c4 = strings[1][3];  /* 'c' from "second" */
}

/* Test function 6: Mixed operations to trigger different paths */
void test_mixed_operations(void) {
    /* Small element type with count > 2 */
    unsigned char bytes[256] = {[32 ... 95] = 0xFF};  /* 64 elements */
    
    /* Structure with array member */
    struct {
        int header;
        int values[8];
    } data = {0};
    
    /* Access multiple elements with constant indices */
    data.values[0] = 100;
    data.values[1] = 200;  /* count = 2 */
    data.values[2] = 300;  /* Now count = 3 */
    
    /* Force constant evaluation with builtin */
    int test_arr[5] = {1, 2, 3, 4, 5};
    if (__builtin_constant_p(test_arr[2])) {
        /* This branch may be taken at compile time */
        test_arr[3] = 99;
    }
}

/* Test function 7: Memory vs register targets */
void test_mem_vs_reg(void) {
    int array[100] = {0};
    
    /* MEM_P target: direct array element assignment */
    array[10] = 42;  /* Targets memory */
    array[11] = 43;  /* Another memory target */
    
    /* Non-MEM_P target: expression result in register */
    int reg_result = array[20] + array[21];  /* Result likely in register */
    
    /* Mixed: memory target with register source */
    array[30] = reg_result;  /* Memory target with register value */
    
    /* Multiple elements with constant bounds */
    int *ptr = &array[50];
    ptr[0] = array[1];  /* Memory target */
    ptr[1] = array[2];  /* Two elements */
}

/* Test function 8: Edge cases with bounds */
void test_edge_cases(void) {
    /* Exactly 2 elements */
    short two_shorts[10] = {[5] = 100, [6] = 200};
    
    /* Exactly 3 elements with small type */
    char three_chars[10] = {[2] = 'a', [3] = 'b', [4] = 'c'};
    
    /* Large count with 1-byte elements */
    unsigned char large_range[1000] = {[100 ... 299] = 0xAA};  /* 200 elements */
    
    /* Using computed but compile-time constant indices */
    enum { START = 5, END = 15 };
    int ranged[50] = {[START ... END] = 999};  /* 11 elements */
}

/* Main function to execute all tests */
int main(void) {
    /* Execute all test functions */
    test_designated_init();
    test_vector_ops();
    test_array_slicing();
    
    int result = test_complex_expr(1);
    
    test_string_indexing();
    test_mixed_operations();
    test_mem_vs_reg();
    test_edge_cases();
    
    /* Use results to prevent dead code elimination */
    volatile int sink = result;
    
    return 0;
}
