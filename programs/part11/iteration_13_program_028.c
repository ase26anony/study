/* Test program for constant-bounds array/vector operations targeting expr.cc lines 7691-7700 */

#include <stddef.h>

/* GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* Structures with arrays */
struct S1 {
    int data[10];
    int other;
};

struct S2 {
    char buf[100];
    int count;
};

/* Flexible array member structure */
struct flex {
    int len;
    char data[];
};

/* Test functions for different scenarios */
void test_mem_target_true() {
    /* MEM_P(target) true path - array elements in memory */
    int arr1[20];
    
    /* Single element access - count = 1 */
    arr1[5] = 42;  /* Constant index 5 */
    
    /* Two element range - count = 2 */
    arr1[3] = 10;
    arr1[4] = 20;
    
    /* Larger range with char elements - count > 2, small type size */
    char buf1[100];
    for (int i = 10; i <= 20; i++) {  /* Constant bounds 10-20, count=11 */
        buf1[i] = 'x';
    }
    
    /* Designated initializer with constant range */
    int arr2[10] = {[2 ... 5] = 42};  /* Constant range 2-5, count=4 */
}

void test_non_mem_target() {
    /* Non-MEM_P(target) path - results likely in registers */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Expression with constant indexing, result in register */
    int x = arr[2] + arr[3];  /* Constant indices 2 and 3 */
    
    /* More complex expression */
    int y = arr[arr[4]];  /* Outer index from array, inner constant 4 */
    
    /* Conditional with constant indices */
    int z = (x > 0 ? arr[1] : arr[2]);  /* Constant indices in both branches */
    
    /* Vector operations - results often in registers */
    v4si v1 = {1,2,3,4};
    v4si v2 = {5,6,7,8};
    v4si v3 = v1 + v2;  /* Vector addition, result in register */
    int elem = v3[2];   /* Constant index 2 into vector */
}

void test_vector_extensions() {
    /* Vector operations with constant indexing */
    v4si a = {1,2,3,4};
    v4si b = {5,6,7,8};
    
    /* Constant vector mask */
    v4si mask = {0, -1, 0, -1};
    v4si res = a * mask;  /* Vector operation with constant mask */
    
    /* Constant index access */
    int x = a[2];  /* Constant index 2 */
    int y = b[3];  /* Constant index 3 */
    
    /* Character vector with many elements */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c = chars[10];  /* Constant index 10 */
}

void test_struct_array_access() {
    /* Array slicing in structs with constant bounds */
    struct S1 s1 = {{0}};
    struct S2 s2 = {{0}};
    
    /* Constant start index into struct array */
    int *p1 = &s1.data[2];  /* Constant index 2 */
    char *p2 = &s2.buf[10]; /* Constant index 10 */
    
    /* Access range in struct array */
    for (int i = 3; i <= 7; i++) {  /* Constant bounds 3-7, count=5 */
        s1.data[i] = i * 2;
    }
    
    /* Larger range with char elements in struct */
    for (int i = 20; i <= 40; i++) {  /* Constant bounds 20-40, count=21 */
        s2.buf[i] = 'A' + (i % 26);
    }
}

void test_constant_string_indexing() {
    /* String literal with constant indexing */
    const char *str = "Hello, World!";
    
    char c1 = str[2];   /* Constant index 2 */
    char c2 = str[7];   /* Constant index 7 */
    char c3 = "test"[1]; /* Direct string literal indexing, constant index 1 */
    
    /* Array initialized with string */
    char arr[] = "Constant bounds test";
    char d1 = arr[5];   /* Constant index 5 */
    char d2 = arr[10];  /* Constant index 10 */
}

void test_switch_array_indexing() {
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int x = 2;
    
    /* Switch with constant array indices */
    switch (x) {
        case 0: x = arr[1]; break;  /* Constant index 1 */
        case 1: x = arr[2]; break;  /* Constant index 2 */
        case 2: x = arr[3]; break;  /* Constant index 3 */
        case 3: x = arr[4]; break;  /* Constant index 4 */
        default: x = arr[0]; break; /* Constant index 0 */
    }
}

void test_builtin_constant_p() {
    int arr[10] = {0};
    
    /* Force constant evaluation with __builtin_constant_p */
    if (__builtin_constant_p(arr[5])) {  /* Constant index 5 */
        /* This branch may be taken at compile time */
    }
    
    /* Test with constant bounds */
    if (__builtin_constant_p(arr[2] + arr[3])) {
        /* Another constant expression test */
    }
}

void test_mixed_operations() {
    /* Combine multiple patterns in complex expressions */
    int arr[20];
    
    /* Nested array access with constant inner index */
    int idx = 5;
    arr[arr[2]] = 10;  /* Inner constant index 2 */
    
    /* Multiple constant index operations in one expression */
    int val = arr[1] + arr[2] * arr[3] - arr[4];
    
    /* Loop with constant bounds that might be unrolled */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * i;
    }
    
    /* Two-dimensional array with constant indices */
    int matrix[5][5];
    matrix[2][3] = 42;  /* Constant indices 2 and 3 */
    matrix[1][4] = matrix[0][2];  /* Constant indices in both sides */
}

/* Main function that calls all test cases */
int main() {
    test_mem_target_true();      /* Covers MEM_P(target) true path */
    test_non_mem_target();       /* Covers non-MEM_P(target) path */
    test_vector_extensions();    /* Vector operations */
    test_struct_array_access();  /* Struct array slicing */
    test_constant_string_indexing(); /* String indexing */
    test_switch_array_indexing(); /* Switch with array indices */
    test_builtin_constant_p();   /* Builtin constant checks */
    test_mixed_operations();     /* Combined operations */
    
    return 0;
}
