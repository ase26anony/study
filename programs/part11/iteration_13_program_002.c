/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* Test 1: Designated initializers with constant ranges */
void test_designated_init() {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* Multiple elements with range - count > 2 */
    int arr3[10] = {[2 ... 5] = 99};
    
    /* Char array with large range - small element size */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Mixed initialization */
    int arr4[20] = {0, 1, 2, [10 ... 15] = 10, [18] = 20};
}

/* Test 2: Vector extensions with constant indexing */
#ifdef __GNUC__
void test_vector_ops() {
    typedef int v4si __attribute__((vector_size(16)));
    typedef char v16c __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v16c b = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
              'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    
    /* Constant index access - likely goes to register */
    int x1 = a[2];      /* Non-MEM_P target path */
    char c1 = b[10];    /* Non-MEM_P target path */
    
    /* Memory target with constant index */
    a[1] = 99;          /* MEM_P target path, count = 1 */
    b[5] = 'z';         /* MEM_P target path, count = 1 */
    
    /* Vector operations with constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si res1 = a * mask;  /* Constant vector operation */
    
    /* Vector shuffle with constant indices */
    v4si shuffled = __builtin_shuffle(a, (v4si){3, 2, 1, 0});
}
#endif

/* Test 3: Array slicing in structs */
struct ArrayStruct {
    int data[20];
    char buffer[50];
    short values[30];
};

void test_struct_arrays() {
    struct ArrayStruct s = {0};
    
    /* Constant start index for pointer */
    int *p1 = &s.data[5];           /* Constant bound, count unknown */
    char *p2 = &s.buffer[10];       /* Constant bound, small elements */
    short *p3 = &s.values[15];      /* Constant bound, medium elements */
    
    /* Direct memory access with constant indices */
    s.data[3] = 100;                /* MEM_P target, count = 1 */
    s.buffer[25] = 'A';             /* MEM_P target, count = 1 */
    s.values[10] = 500;             /* MEM_P target, count = 1 */
    
    /* Two-element range in struct array */
    s.data[6] = 1;
    s.data[7] = 2;                  /* Two adjacent elements */
}

/* Test 4: Complex expressions with constant bounds */
void test_complex_expressions(int cond) {
    int arr[20] = {0};
    int lookup[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Nested array access with constant inner index */
    int x1 = arr[lookup[3]];        /* Outer index from array, inner constant */
    
    /* Conditional array access with constant indices */
    int y1 = (cond ? arr[2] : arr[3]);  /* Both branches constant indices */
    
    /* Array access in arithmetic expression */
    int z1 = arr[5] + arr[6] * 2;   /* Multiple memory accesses */
    
    /* Pointer arithmetic with constant bounds */
    int *ptr = arr + 5;             /* Constant offset */
    int val = *(ptr + 2);           /* Constant offset from pointer */
    
    /* String literal with constant indexing */
    char c1 = "constant string"[7]; /* Constant index into string literal */
    char c2 = "short"[2];           /* Small string, constant index */
}

/* Test 5: Loops with constant bounds (may be unrolled) */
void test_constant_loops() {
    int arr1[10];
    char arr2[20];
    
    /* Small loop - might unroll completely */
    for (int i = 0; i < 5; i++) {
        arr1[i] = i * 2;            /* Constant bound 5 */
    }
    
    /* Loop with exactly 2 iterations */
    for (int i = 8; i < 10; i++) {
        arr1[i] = 99;               /* Two iterations */
    }
    
    /* Char array loop - many small elements */
    for (int i = 0; i < 15; i++) {
        arr2[i] = 'a' + i;          /* 15 char elements */
    }
}

/* Test 6: Switch with array indexing */
int test_switch_array(int code) {
    static int table[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    
    switch (code) {
        case 0: return table[1];    /* Constant index 1 */
        case 1: return table[2];    /* Constant index 2 */
        case 2: return table[3];    /* Constant index 3 */
        case 3: return table[4];    /* Constant index 4 */
        default: return table[0];   /* Constant index 0 */
    }
}

/* Test 7: Flexible array member with constant indexing */
struct FlexArray {
    int count;
    char data[];  /* Zero-length array */
};

void test_flex_array(struct FlexArray *fa) {
    /* Assuming fa points to allocated memory with data */
    if (fa && fa->count > 0) {
        /* Constant index access into flexible array */
        char c = fa->data[0];       /* First element */
        if (fa->count > 5) {
            c = fa->data[5];        /* Constant index 5 */
        }
    }
}

/* Test 8: Builtin constant checks */
void test_builtin_const() {
    int arr[10] = {0};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        arr[5] = 1;
    }
    
    /* Check constant index expression */
    int idx = 3;
    if (__builtin_constant_p(idx)) {
        arr[idx] = 2;
    }
}

/* Test 9: Multi-dimensional array with constant indices */
void test_multi_dim() {
    int matrix[5][10] = {0};
    
    /* Constant indices in 2D array */
    matrix[2][3] = 100;             /* MEM_P target */
    int val = matrix[1][5];         /* Could be register or memory */
    
    /* Row slice with constant row index */
    int *row = matrix[2];           /* Constant row index */
    row[4] = 50;                    /* Memory access */
}

/* Test 10: Mixed scenarios to trigger different branches */
void test_mixed_scenarios() {
    /* Scenario 1: count <= 2 with MEM_P target */
    short small_arr[5];
    small_arr[0] = 1;               /* Single element */
    small_arr[1] = 2;               /* Second element - total 2 */
    
    /* Scenario 2: count > 2 with small element size */
    unsigned char bytes[100];
    for (int i = 10; i < 30; i++) { /* 20 elements */
        bytes[i] = i & 0xFF;
    }
    
    /* Scenario 3: Non-MEM_P target (register) */
    int temp[5] = {1, 2, 3, 4, 5};
    int sum = temp[1] + temp[2] + temp[3];  /* Result in register */
    
    /* Scenario 4: Large count with larger elements */
    double doubles[50];
    doubles[10] = 3.14;             /* Single element, larger size */
}

/* Main function to execute all tests */
int main() {
    test_designated_init();
    
    #ifdef __GNUC__
    test_vector_ops();
    #endif
    
    test_struct_arrays();
    test_complex_expressions(1);
    test_constant_loops();
    
    int result = test_switch_array(2);
    
    struct FlexArray *fa = (struct FlexArray*)malloc(sizeof(struct FlexArray) + 20);
    if (fa) {
        fa->count = 20;
        test_flex_array(fa);
        free(fa);
    }
    
    test_builtin_const();
    test_multi_dim();
    test_mixed_scenarios();
    
    return 0;
}
