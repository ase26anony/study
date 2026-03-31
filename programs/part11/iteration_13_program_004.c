/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stddef.h>
#include <string.h>

/* Vector extensions for GCC */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8s __attribute__((vector_size(16)));

/* Struct with array for slicing */
struct ArrayStruct {
    int data[20];
    char buffer[100];
};

/* Struct with flexible array member */
struct FlexStruct {
    int count;
    char items[];
};

/* Helper function to force register allocation */
static int use_in_register(int x) {
    return x * 2;
}

/* Test function 1: Designated initializers with constant ranges */
void test_designated_init(void) {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* Larger range with char - count > 2, small element size */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Medium range with int */
    int arr3[20] = {[2 ... 8] = 99};  /* 7 elements */
}

/* Test function 2: Vector operations with constant indices */
void test_vector_ops(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant indexing into vector - non-MEM_P target likely */
    int elem1 = vec1[2];  /* Constant index 2 */
    int elem2 = vec2[0];  /* Constant index 0 */
    
    /* Vector operation with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si res1 = vec1 * mask;  /* Constant vector mask */
    
    /* Vector conditional with constant indices */
    v4si cmp = vec1 > vec2;
    int cmp_elem = cmp[3];  /* Constant index 3 */
    
    /* Character vector with larger count */
    v16c char_vec = {[0 ... 15] = 32};  /* 16 elements */
    char c = char_vec[7];  /* Constant index 7 */
}

/* Test function 3: Array slicing through structs */
void test_struct_slicing(void) {
    struct ArrayStruct s = {0};
    
    /* Pointer to slice with constant start - MEM_P target */
    int *slice1 = &s.data[2];  /* Constant index 2 */
    char *slice2 = &s.buffer[10];  /* Constant index 10 */
    
    /* Multiple element access with constant bounds */
    s.data[5] = 100;  /* Single element - count = 1 */
    s.data[6] = 200;
    s.data[7] = 300;  /* Three separate but could be grouped */
    
    /* Two-element range in struct */
    s.buffer[20] = 'a';
    s.buffer[21] = 'b';  /* count = 2 */
}

/* Test function 4: Complex expressions with constant bounds */
int test_complex_expr(int cond) {
    int arr[20] = {0};
    int brr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Nested array access with constant inner index */
    int x = arr[brr[2]];  /* brr[2] is constant 3 at compile time? */
    
    /* Conditional with constant indices in both branches */
    int y = cond ? arr[5] : arr[6];  /* Both indices constant */
    
    /* Expression favoring register allocation */
    int z = use_in_register(arr[3] + arr[4]);  /* Non-MEM_P target */
    
    /* String literal with constant indexing */
    char c1 = "constant string"[7];  /* Constant index 7 */
    char c2 = "hello"[1];  /* Constant index 1 */
    
    return x + y + z + c1 + c2;
}

/* Test function 5: Loops with constant bounds (may unroll) */
void test_constant_loops(void) {
    int arr[10];
    
    /* Small loop - might unroll to count <= 2? */
    for (int i = 0; i < 2; i++) {
        arr[i] = i * 10;
    }
    
    /* Larger loop with char array - count > 2 */
    char buf[50];
    for (int i = 0; i < 25; i++) {  /* 25 elements */
        buf[i] = i % 26 + 'a';
    }
    
    /* Nested loop with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * j;
        }
    }
}

/* Test function 6: Switch with array indexing */
int test_switch_array(int code) {
    int values[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    switch (code) {
        case 0: return values[1];  /* Constant index 1 */
        case 1: return values[2];  /* Constant index 2 */
        case 2: return values[3];  /* Constant index 3 */
        case 3: return values[4];  /* Constant index 4 */
        default: return values[0]; /* Constant index 0 */
    }
}

/* Test function 7: Builtin constant checks */
void test_builtin_const(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        arr[5] = 1;
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(5) && __builtin_constant_p(8)) {
        for (int i = 5; i <= 8; i++) {  /* count = 4 */
            arr[i] = i * 2;
        }
    }
}

/* Test function 8: Memory vs register targeting */
void test_mem_vs_reg(void) {
    int array[100];
    int local1, local2, local3;
    
    /* MEM_P target: direct array assignment */
    array[10] = 42;           /* Single element */
    array[11] = 43;           /* Another single */
    array[12] = array[11];    /* Memory to memory */
    
    /* Non-MEM_P target: register operations */
    local1 = array[15];       /* Memory to register */
    local2 = array[16] + 5;   /* Arithmetic in register */
    local3 = use_in_register(array[17]);  /* Function arg in register */
    
    /* Mixed: memory destination with register source */
    array[20] = local1 + local2;  /* Register to memory */
}

/* Test function 9: Variable length but constant in context */
void test_contextual_constant(void) {
    static const int bounds[2] = {5, 10};
    int arr[20];
    
    /* Bounds are constant through static array */
    for (int i = bounds[0]; i <= bounds[1]; i++) {
        arr[i] = i * 3;  /* i from 5 to 10, count = 6 */
    }
    
    /* String operations with constant indices */
    char str[] = "test string";
    char *p = &str[3];  /* Constant offset 3 */
    p[0] = 'x';         /* Constant index 0 relative to p */
    p[1] = 'y';         /* Constant index 1, count = 2? */
}

/* Test function 10: Vector with many small elements */
void test_small_elements(void) {
    /* Many char elements - triggers type size * count calculation */
    char large_buf[256] = {
        [0 ... 63] = 'A',    /* 64 elements */
        [64 ... 127] = 'B',  /* 64 elements */
        [128 ... 191] = 'C', /* 64 elements */
        [192 ... 255] = 'D'  /* 64 elements */
    };
    
    /* Short vector with constant indexing */
    v8s short_vec = {[0 ... 7] = 100};  /* 8 elements of 2 bytes each */
    short s = short_vec[4];  /* Constant index 4 */
    
    /* Copy with constant size */
    char dest[50];
    __builtin_memcpy(dest, large_buf + 10, 30);  /* Constant offset 10, size 30 */
}

int main(void) {
    /* Execute all test functions to trigger various code paths */
    test_designated_init();
    test_vector_ops();
    test_struct_slicing();
    
    int result = test_complex_expr(1);
    test_constant_loops();
    result += test_switch_array(2);
    
    test_builtin_const();
    test_mem_vs_reg();
    test_contextual_constant();
    test_small_elements();
    
    return result % 256;  /* Return non-zero to avoid dead code elimination */
}
