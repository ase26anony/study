/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stdio.h>
#include <string.h>

/* GCC vector extensions for vector operations */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Struct with array for array-in-struct access */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

/* Struct with flexible array member */
struct FlexArray {
    int count;
    char items[];
};

/* Helper function to prevent optimizations */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test function 1: Designated initializers with constant ranges */
void test_designated_init(void) {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two element range - count = 2 */
    int arr2[10] = {[2] = 10, [3] = 20};
    
    /* Larger range with char - count > 2, small element size */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Even larger range */
    char buf2[200] = {[30 ... 80] = 'y'};  /* 51 elements */
    
    use(arr1); use(arr2); use(buf1); use(buf2);
}

/* Test function 2: Vector operations with constant indices */
void test_vector_ops(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant indexing into vector - non-MEM_P target (register) */
    int elem1 = vec1[2];  /* Constant index 2 */
    int elem2 = vec1[0] + vec1[3];  /* Multiple constant indices */
    
    /* Vector operations that might go to memory */
    v4si result;
    result[0] = vec1[1] + vec2[1];  /* Memory target with constant index */
    result[1] = vec1[2] * vec2[2];
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    
    use(&elem1); use(&elem2); use(&result); use(&masked);
}

/* Test function 3: Array slicing in structs */
void test_struct_arrays(void) {
    struct ArrayStruct s = {0};
    
    /* Constant start index for pointer */
    int *p1 = &s.data[2];  /* Constant index 2 */
    int *p2 = &s.data[5];  /* Constant index 5 */
    
    /* Multiple element range in struct array */
    s.data[3] = 100;
    s.data[4] = 200;  /* Two elements - count = 2 */
    
    /* Larger range in char array */
    s.buffer[10] = 'a';
    s.buffer[11] = 'b';
    s.buffer[12] = 'c';  /* Three elements - count = 3 */
    
    use(p1); use(p2);
}

/* Test function 4: Complex expressions with constant bounds */
void test_complex_expr(void) {
    int arr[20] = {0};
    int idx_arr[5] = {2, 3, 4, 5, 6};
    
    /* Nested array access with constant inner index */
    int x = arr[arr[2]];  /* arr[2] is constant index access */
    
    /* Conditional with constant indices */
    int cond = 1;
    int y = cond ? arr[2] : arr[3];
    
    /* Loop with constant bounds (might be unrolled) */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Switch with constant array indices */
    int z = 0;
    switch (arr[0]) {
        case 0: z = arr[1]; break;  /* Constant index 1 */
        case 1: z = arr[2]; break;  /* Constant index 2 */
        case 2: z = arr[3]; break;  /* Constant index 3 */
    }
    
    /* __builtin_constant_p with array access */
    if (__builtin_constant_p(arr[5])) {
        arr[6] = 99;
    }
    
    use(&x); use(&y); use(&z);
}

/* Test function 5: String literal operations */
void test_string_ops(void) {
    /* Constant string indexing */
    char c1 = "hello world"[2];   /* 'l' - constant index 2 */
    char c2 = "test string"[5];   /* 's' - constant index 5 */
    
    /* String copy with constant bounds */
    char dest[20];
    strncpy(dest, "constant", 8);  /* Constant bound 8 */
    
    /* Character array with constant range */
    char chars[10] = {[0 ... 4] = 'A', [5 ... 9] = 'B'};
    
    use(&c1); use(&c2); use(dest); use(chars);
}

/* Test function 6: Mixed operations to trigger different paths */
void test_mixed_ops(void) {
    /* Test count <= 2 path with MEM_P target */
    short shorts[10];
    shorts[3] = 100;      /* Single element */
    shorts[4] = 200;      /* Another single element */
    shorts[5] = 300;      /* Another single element */
    
    /* Test count > 2 with small element size */
    unsigned char bytes[256];
    for (int i = 10; i < 25; i++) {  /* 15 elements */
        bytes[i] = i;
    }
    
    /* Vector with constant indices in expression */
    v8hi vshort = {1, 2, 3, 4, 5, 6, 7, 8};
    int sum = vshort[0] + vshort[1] + vshort[2];  /* Multiple constant indices */
    
    /* Array section copy with constant bounds */
    int src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int dst[10];
    
    /* Copy 3 elements - count = 3 */
    dst[0] = src[2];
    dst[1] = src[3];
    dst[2] = src[4];
    
    /* Copy 2 elements - count = 2 */
    dst[5] = src[6];
    dst[6] = src[7];
    
    use(shorts); use(bytes); use(&sum); use(dst);
}

/* Test function 7: Memory vs register targets */
void test_mem_vs_reg(void) {
    int array[100] = {0};
    
    /* MEM_P(target) true - array element assignment */
    array[10] = 42;           /* Targets memory */
    array[20] = array[15];    /* Source and target both memory */
    
    /* Non-MEM_P(target) - intermediate in register */
    int reg1 = array[5] + array[6];    /* Result likely in register */
    int reg2 = array[7] * 2;           /* Result likely in register */
    
    /* Mixed: memory target with register source */
    array[30] = reg1 + reg2;  /* Target memory, source registers */
    
    /* Constant index chain */
    int chain = array[array[1]];  /* Inner constant, outer variable */
    
    use(&reg1); use(&reg2); use(&chain);
}

/* Main function that runs all tests */
int main(void) {
    printf("Testing constant bounds array/vector operations...\n");
    
    test_designated_init();
    test_vector_ops();
    test_struct_arrays();
    test_complex_expr();
    test_string_ops();
    test_mixed_ops();
    test_mem_vs_reg();
    
    printf("All tests completed (runtime behavior secondary to compilation coverage).\n");
    return 0;
}
