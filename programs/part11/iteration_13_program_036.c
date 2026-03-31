/* Test program to exercise constant bounds checking in GCC's expr.cc */
#include <stddef.h>

/* Vector types for GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8s __attribute__((vector_size(16)));

/* Struct with array for memory access patterns */
struct ArrayStruct {
    int data[20];
    char buffer[100];
};

/* Flexible array struct */
struct FlexArray {
    int count;
    char items[];
};

/* Global arrays for memory targeting */
int global_arr[50] = {0};
char global_buf[200] = {0};

/* Test 1: Designated initializers with constant ranges */
void test_designated_init(void) {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* Multiple elements with range - count > 2 */
    int arr3[10] = {[2 ... 5] = 99};
    
    /* Char array with large range - triggers type size calculation */
    char buf1[100] = {[10 ... 20] = 'x'};
    
    /* Mixed initialization */
    int arr4[15] = {0, 1, 2, [5 ... 9] = 5, [12] = 12};
}

/* Test 2: Vector operations with constant indices */
void test_vector_ops(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing - non-MEM_P target (register) */
    int elem1 = vec1[2];  /* Constant index 2 */
    int elem2 = vec1[0] + vec1[3];  /* Multiple constant indices */
    
    /* Vector operations that might go to memory */
    v4si vec3;
    vec3[1] = 10;  /* MEM_P target with single element */
    vec3[2] = 20;
    
    /* Vector with char elements for size calculation */
    v16c char_vec = {0};
    char_vec[5] = 'a';
    char_vec[6] = 'b';
    char_vec[7] = 'c';  /* Three elements, each size 1 */
    
    /* Vector conditional with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
}

/* Test 3: Array slicing through structs */
void test_struct_arrays(void) {
    struct ArrayStruct s = {0};
    
    /* Constant start index for pointer */
    int *slice1 = &s.data[2];  /* Constant index 2 */
    char *slice2 = &s.buffer[10];  /* Constant index 10 */
    
    /* Access ranges within struct array */
    s.data[3] = 100;  /* Single element - MEM_P target */
    s.data[4] = 200;  /* Another single element */
    
    /* Two-element range in struct */
    s.buffer[20] = 'a';
    s.buffer[21] = 'b';
    
    /* Larger range in struct */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        s.data[10 + i] = i;
    }
}

/* Test 4: Complex expressions with constant bounds */
int test_complex_expr(int cond) {
    int local_arr[20] = {0};
    
    /* Nested array access with constant inner index */
    int x = local_arr[local_arr[2]];  /* Inner index 2 is constant */
    
    /* Conditional with constant indices in both branches */
    int y = (cond ? local_arr[3] : local_arr[4]);
    
    /* Switch with constant array indexing */
    switch (cond) {
        case 0: return local_arr[1];
        case 1: return local_arr[2];
        case 2: return local_arr[3];
        default: return local_arr[0];
    }
    
    /* Array access in arithmetic expression */
    return local_arr[5] + local_arr[6] * 2;
}

/* Test 5: Memory vs register targeting */
void test_memory_targets(void) {
    int arr[30] = {0};
    
    /* MEM_P target cases (array elements) */
    arr[5] = 42;  /* Single element memory write */
    arr[6] = arr[7];  /* Memory-to-memory with constant indices */
    
    /* Non-MEM_P target (register result) */
    int reg1 = arr[8] + arr[9];  /* Result likely in register */
    int reg2 = arr[10] * 2;      /* Another register result */
    
    /* Mixed: memory source, register computation */
    int reg3 = arr[11] + arr[12] + arr[13];
    
    /* Global array access - definitely memory */
    global_arr[15] = 100;
    int from_global = global_arr[16];
}

/* Test 6: String literal constant indexing */
void test_string_indexing(void) {
    /* Direct constant indexing */
    char c1 = "hello world"[4];  /* Constant index 4 */
    
    /* Multiple constant indices */
    char c2 = "test string"[0];
    char c3 = "another"[3];
    
    /* In expression */
    int sum = "abc"[0] + "def"[1];  /* 'a' + 'e' */
    
    /* With pointer arithmetic */
    const char *str = "constant";
    char c4 = *(str + 2);  /* Constant offset 2 */
}

/* Test 7: Loop with constant bounds (may unroll) */
void test_constant_loops(void) {
    int arr[10];
    
    /* Small loop - might unroll completely */
    for (int i = 0; i < 3; i++) {  /* Constant bound 3 */
        arr[i] = i * 10;
    }
    
    /* Larger loop - might partially unroll */
    for (int i = 0; i < 8; i++) {  /* Constant bound 8 */
        arr[i] = arr[i] + 1;
    }
    
    /* Nested loops with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * j;
        }
    }
}

/* Test 8: Compiler builtins and extensions */
void test_builtins(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        arr[5] = 1;
    }
    
    /* Constant index with sizeof */
    size_t offset = sizeof(arr[2]);  /* Constant index 2 */
    
    /* Offsetof with array in struct */
    struct Test {
        int x;
        int arr[10];
        int y;
    } t;
    
    size_t arr_offset = offsetof(struct Test, arr[3]);  /* Constant index 3 */
}

/* Test 9: Multi-dimensional array constant indexing */
void test_multi_dim(void) {
    int matrix[5][5] = {0};
    
    /* Constant indices in 2D */
    matrix[2][3] = 42;  /* Single element */
    
    /* Row slice */
    int *row = matrix[2];  /* Constant first index */
    
    /* Multiple constant indices */
    int val = matrix[1][2] + matrix[3][4];
    
    /* 3D array */
    int cube[3][3][3];
    cube[1][1][1] = 100;  /* All indices constant */
}

/* Test 10: Variable arrays with constant bounds (C99 VLA) */
void test_vla_constant_bounds(void) {
    const int n = 10;  /* Compile-time constant */
    int vla[n];        /* Actually a regular array since n is constant */
    
    /* Constant indexing into VLA */
    vla[5] = 50;
    vla[6] = 60;
    
    /* Loop with constant bound */
    for (int i = 0; i < n; i++) {
        vla[i] = i * 2;
    }
}

/* Main function to call all tests */
int main(void) {
    test_designated_init();
    test_vector_ops();
    test_struct_arrays();
    
    int result = test_complex_expr(1);
    test_memory_targets();
    test_string_indexing();
    test_constant_loops();
    test_builtins();
    test_multi_dim();
    test_vla_constant_bounds();
    
    /* Use results to prevent dead code elimination */
    return result + global_arr[0];
}
