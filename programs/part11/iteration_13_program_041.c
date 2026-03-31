#include <stddef.h>

/* Test 1: Designated initializers with constant ranges */
void test_designated_init() {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* Multiple elements - count > 2 */
    int arr3[10] = {[2 ... 5] = 99};
    
    /* Char array with large range - count > 2, small element size */
    char buf1[100] = {[10 ... 20] = 'x'};
    
    /* Mixed initialization */
    int arr4[20] = {0, [5 ... 9] = 5, [15] = 10};
}

/* Test 2: Vector extensions with constant indexing */
#ifdef __GNUC__
void test_vector_extensions() {
    typedef int v4si __attribute__((vector_size(16)));
    typedef char v16c __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v16c b = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
              'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    
    /* Constant indexing into vectors - likely goes to registers */
    int x1 = a[0];  /* Non-MEM_P target */
    int x2 = a[2];  /* Non-MEM_P target */
    
    /* Vector operations with constant masks */
    v4si mask1 = {0, -1, 0, -1};
    v4si res1 = a * mask1;
    
    v4si mask2 = {-1, -1, -1, -1};
    v4si res2 = a + mask2;
    
    /* Char vector with constant indexing */
    char c1 = b[5];
    char c2 = b[15];
}
#endif

/* Test 3: Array slicing in structs */
struct ArrayStruct {
    int data[20];
    char buffer[50];
    long values[10];
};

void test_struct_arrays() {
    struct ArrayStruct s = {0};
    
    /* Constant start index for pointer */
    int *p1 = &s.data[2];      /* count depends on usage */
    char *p2 = &s.buffer[10];  /* char type, small element size */
    long *p3 = &s.values[5];   /* long type, larger element size */
    
    /* Direct memory access with constant indices */
    s.data[3] = 100;      /* MEM_P target, count = 1 */
    s.data[4] = 200;      /* MEM_P target, count = 1 */
    
    /* Two-element range access */
    s.buffer[20] = 'a';
    s.buffer[21] = 'b';   /* MEM_P target, count = 2 */
    
    /* Multi-element range */
    s.buffer[30] = 'x';
    s.buffer[31] = 'y';
    s.buffer[32] = 'z';   /* MEM_P target, count = 3, char type */
}

/* Test 4: Complex expressions with constant bounds */
void test_complex_expressions(int cond) {
    int arr[20] = {0};
    
    /* Nested array access with constant inner index */
    arr[0] = 5;
    int x1 = arr[arr[2]];  /* arr[2] is constant 0 */
    
    /* Conditional array access with constant indices */
    int y = (cond ? arr[2] : arr[3]);
    
    /* Array access in arithmetic expression */
    int z = arr[5] * 2 + arr[6] / 3;
    
    /* Multiple array accesses in one expression */
    int w = arr[1] + arr[2] + arr[3] + arr[4];
}

/* Test 5: Loop with constant bounds (may be unrolled) */
void test_constant_loops() {
    int arr1[5];
    
    /* Small loop - might be unrolled */
    for (int i = 0; i < 5; i++) {
        arr1[i] = i * 2;
    }
    
    /* Two-iteration loop */
    int arr2[10] = {0};
    for (int i = 3; i < 5; i++) {
        arr2[i] = i * 10;
    }
    
    /* Larger loop with char array */
    char buf[20];
    for (int i = 0; i < 10; i++) {
        buf[i] = 'A' + i;
    }
}

/* Test 6: Switch with array indexing */
int test_switch_array(int x) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    
    switch (x) {
        case 0: return arr[1];  /* Constant index 1 */
        case 1: return arr[2];  /* Constant index 2 */
        case 2: return arr[3];  /* Constant index 3 */
        case 3: return arr[4] + arr[5];  /* Two constant indices */
        default: return arr[0];
    }
}

/* Test 7: Flexible array member with constant indexing */
struct flex_array {
    size_t len;
    char data[];
};

void test_flex_array(struct flex_array *fa) {
    /* Assuming fa points to allocated memory with data */
    if (fa && fa->len > 5) {
        char c1 = fa->data[2];  /* Constant index 2 */
        char c2 = fa->data[4];  /* Constant index 4 */
        
        /* Two-element range */
        fa->data[0] = 'x';
        fa->data[1] = 'y';
    }
}

/* Test 8: Builtin constant check */
void test_builtin_constant() {
    int arr[10] = {0};
    
    /* Force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        arr[5] = 100;
    }
    
    /* Check constant index */
    if (__builtin_constant_p(5)) {
        arr[5] = 200;
    }
}

/* Test 9: String literal with constant indexing */
void test_string_literals() {
    const char *str = "Hello, World!";
    
    /* Constant indexing into string literal */
    char c1 = str[0];
    char c2 = str[7];
    char c3 = "Constant"[3];
    
    /* Multiple character extraction */
    char buf[5];
    buf[0] = str[0];
    buf[1] = str[1];
    buf[2] = str[2];  /* Three-element operation */
}

/* Test 10: Multi-dimensional arrays */
void test_multi_dim_arrays() {
    int matrix[5][5] = {0};
    
    /* Constant row, variable column */
    matrix[2][3] = 100;  /* MEM_P target, count = 1 */
    
    /* Constant column, variable row */
    for (int i = 0; i < 5; i++) {
        matrix[i][2] = i * 10;  /* Constant column index 2 */
    }
    
    /* Two-element range in 2D */
    matrix[1][1] = 10;
    matrix[1][2] = 20;
}

/* Test 11: Pointer arithmetic with constant bounds */
void test_pointer_arithmetic() {
    int array[20] = {0};
    int *ptr = array;
    
    /* Constant offset pointer arithmetic */
    int *p1 = ptr + 5;    /* Constant offset 5 */
    int *p2 = &ptr[10];   /* Constant index 10 */
    
    /* Dereference with constant offset */
    int val1 = *(ptr + 3);  /* Non-MEM_P? Depends on context */
    int val2 = ptr[7];      /* Same as above */
    
    /* Multiple dereferences */
    ptr[5] = 50;    /* MEM_P target */
    ptr[6] = 60;    /* MEM_P target */
    ptr[7] = 70;    /* MEM_P target, count = 3 */
}

/* Main function combining all tests */
int main() {
    /* Test designated initializers */
    test_designated_init();
    
#ifdef __GNUC__
    /* Test vector extensions */
    test_vector_extensions();
#endif
    
    /* Test struct arrays */
    test_struct_arrays();
    
    /* Test complex expressions */
    test_complex_expressions(1);
    
    /* Test constant loops */
    test_constant_loops();
    
    /* Test switch with array */
    int r1 = test_switch_array(2);
    
    /* Test flexible array */
    struct flex_array *fa = (struct flex_array*)malloc(sizeof(struct flex_array) + 100);
    if (fa) {
        fa->len = 100;
        test_flex_array(fa);
        free(fa);
    }
    
    /* Test builtin constant */
    test_builtin_constant();
    
    /* Test string literals */
    test_string_literals();
    
    /* Test multi-dimensional arrays */
    test_multi_dim_arrays();
    
    /* Test pointer arithmetic */
    test_pointer_arithmetic();
    
    return 0;
}
