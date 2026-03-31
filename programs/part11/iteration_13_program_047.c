/* Test program to exercise constant-bounds array/vector operations in GCC expr.cc */
#include <stddef.h>

/* Vector extension types */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8s __attribute__((vector_size(16)));

/* Structures with arrays */
struct S {
    int data[10];
    char buf[20];
};

struct Flex {
    int len;
    char data[];
};

/* Global arrays for memory operations */
int global_arr[100];
char global_buf[256];

/* Test 1: Designated initializers with constant ranges */
void test_designated_init(void) {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[2] = 1, [3] = 2};
    
    /* Range > 2 with char type - triggers type size calculation */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Range > 2 with int type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 elements */
}

/* Test 2: Vector extensions with constant indexing */
void test_vector_ops(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v16c chars = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    
    /* Single element access - register target */
    int x = a[2];  /* Non-MEM_P target likely */
    
    /* Multiple element accesses */
    int y = a[1] + a[3];
    
    /* Vector operations with constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si res = a * mask;
    
    /* Vector conditional with constant indices */
    v4si cond = a > b;
    int z = cond[2] ? a[1] : b[3];
}

/* Test 3: Array slicing in structs */
void test_struct_arrays(void) {
    struct S s = {0};
    struct S *ps = &s;
    
    /* Constant start index for pointer */
    int *p1 = &s.data[2];  /* Constant bound 2 */
    char *p2 = &s.buf[5];  /* Constant bound 5 */
    
    /* Multiple element access from struct */
    s.data[3] = 10;
    s.data[4] = 20;  /* Two elements, count = 2 */
    
    /* Range > 2 in struct */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        s.buf[i] = 'A' + i;
    }
}

/* Test 4: Memory operations (MEM_P target) */
void test_mem_ops(void) {
    /* Single element - MEM_P target, count = 1 */
    global_arr[25] = 100;
    
    /* Two elements - MEM_P target, count = 2 */
    global_arr[30] = 200;
    global_arr[31] = 201;
    
    /* Range > 2 with small elements */
    for (int i = 40; i < 50; i++) {  /* 10 elements */
        global_buf[i] = i;
    }
    
    /* String literal with constant indexing */
    char c1 = "hello world"[6];  /* 'w' */
    const char *str = "constant";
    char c2 = str[3];  /* 's' */
}

/* Test 5: Complex expressions with constant bounds */
void test_complex_exprs(void) {
    int arr[20] = {0};
    
    /* Nested array access with constant inner index */
    arr[arr[2]] = 10;  /* arr[2] is constant 0 after initialization */
    
    /* Conditional with constant indices */
    int cond = 1;
    int x = (cond ? arr[5] : arr[6]);
    
    /* Switch with array indexing */
    switch (cond) {
        case 0: x = arr[1]; break;
        case 1: x = arr[2]; break;
        case 2: x = arr[3]; break;
    }
    
    /* Builtin to force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        x = 1;
    }
}

/* Test 6: Loop with constant bounds (may unroll) */
void test_constant_loops(void) {
    int arr[10];
    
    /* Small loop - might unroll completely */
    for (int i = 0; i < 3; i++) {  /* count = 3 */
        arr[i] = i * 10;
    }
    
    /* Larger loop with char elements */
    char buf[50];
    for (int i = 10; i < 25; i++) {  /* 15 elements */
        buf[i] = 'Z';
    }
}

/* Test 7: Multi-dimensional arrays */
void test_multi_dim(void) {
    int matrix[5][5] = {0};
    
    /* Constant row access */
    int *row = matrix[2];  /* Constant index 2 */
    
    /* Constant element */
    matrix[3][4] = 99;
    
    /* Constant slice in 2D */
    for (int i = 0; i < 3; i++) {
        matrix[1][i] = i;  /* 3 elements in row 1 */
    }
}

/* Test 8: Pointer arithmetic with constant bounds */
void test_pointer_arithmetic(void) {
    int arr[50];
    int *p = arr;
    
    /* Constant offset pointer */
    int *p1 = p + 10;  /* Constant bound 10 */
    int *p2 = &p[15];  /* Constant bound 15 */
    
    /* Constant range copy */
    for (int i = 20; i < 30; i++) {  /* 10 elements */
        p[i] = i * 2;
    }
}

/* Test 9: Mixed operations to trigger different paths */
void test_mixed(void) {
    v4si vec = {10, 20, 30, 40};
    int arr[20];
    struct S s;
    
    /* Mix vector and array ops */
    arr[5] = vec[2];  /* Vector element to memory */
    
    /* Multiple constant index operations in one expression */
    int x = arr[2] + arr[3] + arr[4];  /* Three elements, register target */
    
    /* Struct array with constant range */
    for (int i = 2; i < 8; i++) {  /* 6 elements */
        s.data[i] = i * 100;
    }
    
    /* Conditional with mixed targets */
    int *ptr = (x > 0) ? &arr[10] : &arr[11];
    *ptr = 999;
}

/* Main function to call all tests */
int main(void) {
    test_designated_init();
    test_vector_ops();
    test_struct_arrays();
    test_mem_ops();
    test_complex_exprs();
    test_constant_loops();
    test_multi_dim();
    test_pointer_arithmetic();
    test_mixed();
    
    return 0;
}
