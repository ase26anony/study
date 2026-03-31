/* Test program for constant-bounds array/vector operations targeting expr.cc lines 7691-7700 */

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
    char data[];  /* Zero-length array */
};

/* Test functions for different scenarios */
void test_mem_target_paths(void) {
    /* MEM_P(target) true path - targeting memory locations */
    int arr1[20];
    
    /* Single element - count = 1 */
    arr1[5] = 42;  /* Constant index, targets memory */
    
    /* Two elements - count = 2 */
    arr1[3] = 10;
    arr1[4] = 20;  /* Two-element range */
    
    /* Designated initializer with constant range - count = 4 */
    int arr2[10] = {[2 ... 5] = 100};  /* Constant bounds: 2 to 5 */
    
    /* Larger range with char type - triggers type size calculation */
    char buf1[100] = {[10 ... 25] = 'x'};  /* 16 elements, char size 1 */
    
    /* Even larger range to ensure count > 2 */
    char buf2[200] = {[30 ... 80] = 'y'};  /* 51 elements */
}

void test_non_mem_target_paths(void) {
    /* Non-MEM_P(target) path - results likely go to registers */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Expression results to registers */
    int x = arr[2] + arr[3];  /* Constant indices, result in register */
    int y = arr[4] * arr[5];  /* Another register result */
    
    /* Complex expression with constant indexing */
    int z = arr[arr[2]];  /* Nested: outer index from array, inner constant */
    
    /* Conditional with constant indices */
    int cond = 1;
    int w = (cond ? arr[6] : arr[7]);  /* Both branches constant indices */
}

void test_vector_operations(void) {
    /* Vector operations with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem = vec1[2];  /* Constant index 2 */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;  /* Constant vector mask */
    
    /* Char vector with many elements */
    v16c char_vec = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c = char_vec[10];  /* Constant index */
}

void test_struct_operations(void) {
    /* Array in struct with constant bounds */
    struct ArrayStruct s;
    
    /* Constant indexing into struct array */
    s.data[5] = 100;
    s.buffer[10] = 'A';
    
    /* Pointer to sub-range with constant start */
    int *p = &s.data[3];  /* Constant start index 3 */
    char *q = &s.buffer[20];  /* Constant start index 20 */
    
    /* Multiple elements in struct array */
    s.data[6] = 200;
    s.data[7] = 300;  /* Two-element range */
}

void test_constant_strings(void) {
    /* String literal with constant indexing */
    char c1 = "Hello World!"[4];  /* Constant index 4 = 'o' */
    char c2 = "Test String"[0];   /* Constant index 0 = 'T' */
    
    /* Multiple string accesses */
    const char *str = "Constant";
    char a = str[1];  /* 'o' */
    char b = str[2];  /* 'n' */
    char d = str[3];  /* 's' */
}

void test_loops_with_constant_bounds(void) {
    int arr[10];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Nested loop with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {      /* Constant bound */
        for (int j = 0; j < 5; j++) {  /* Constant bound */
            matrix[i][j] = i + j;
        }
    }
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int x = 2;
    
    /* Switch with constant array indices */
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
            x = arr[4];  /* Constant index 4 */
    }
}

void test_builtin_constant_p(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[2])) {
        /* This branch might be taken at compile time */
        int x = arr[3];  /* Constant index 3 */
    }
    
    /* Check constant bounds */
    if (__builtin_constant_p(arr[4])) {
        int y = arr[1];  /* Constant index 1 */
    }
}

void test_mixed_operations(void) {
    /* Combine multiple patterns in complex expressions */
    struct ArrayStruct s;
    int arr[20];
    
    /* Mixed memory and register operations */
    s.data[5] = arr[10] + arr[11];  /* Right side in registers, left in memory */
    
    /* Chain of operations */
    int x = arr[2];
    arr[3] = x + 1;
    arr[4] = arr[3] * 2;
    
    /* Array section copy with constant bounds */
    int src[10] = {0,1,2,3,4,5,6,7,8,9};
    int dst[10];
    
    /* Copy range 2-5 (4 elements) */
    for (int i = 2; i <= 5; i++) {  /* Constant bounds */
        dst[i] = src[i];
    }
}

int main(void) {
    /* Execute all test functions to trigger various code paths */
    test_mem_target_paths();        /* MEM_P(target) paths with various counts */
    test_non_mem_target_paths();    /* Non-MEM_P(target) paths */
    test_vector_operations();       /* Vector extensions */
    test_struct_operations();       /* Struct array accesses */
    test_constant_strings();        /* String literal indexing */
    test_loops_with_constant_bounds(); /* Loop unrolling opportunities */
    test_switch_with_array_indexing(); /* Switch cases with array indices */
    test_builtin_constant_p();      /* Builtin constant checks */
    test_mixed_operations();        /* Complex mixed expressions */
    
    return 0;
}
