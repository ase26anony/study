/* Test program to exercise constant-bounds array/vector operations in GCC's expr.cc */
#include <stddef.h>
#include <string.h>

/* GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Structures with arrays */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];  /* Flexible array member */
};

/* Test functions to cover different paths */
void test_mem_targets(void) {
    /* MEM_P(target) true path - targeting memory locations */
    int arr1[20];
    
    /* Single element - count = 1 */
    arr1[5] = 42;                     /* Line 7697: count <= 2 path */
    
    /* Two elements - count = 2 */
    arr1[6] = 10;                     /* Also count <= 2 */
    arr1[7] = 20;
    
    /* Designated initializer with constant range - count > 2 */
    int arr2[30] = {[10 ... 15] = 99}; /* 6 elements, count > 2 */
    
    /* Small element type with count > 2 */
    char buf1[100] = {[20 ... 30] = 'A'}; /* 11 chars, size=1, count>2 */
    
    /* Larger element type with count > 2 */
    short buf2[50] = {[5 ... 10] = 1234}; /* 6 shorts, size=2, count>2 */
}

void test_register_targets(void) {
    /* Non-MEM_P(target) path - results likely go to registers */
    int arr[20] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Simple constant indexing to registers */
    int r1 = arr[2];                  /* Single element to register */
    int r2 = arr[3] + arr[4];         /* Expression result to register */
    
    /* Conditional with constant indices */
    int cond = 1;
    int r3 = cond ? arr[5] : arr[6];  /* Both branches constant indices */
    
    /* Nested array access with constant inner index */
    int idx_arr[10] = {3,3,3,3,3,3,3,3,3,3};
    int r4 = arr[idx_arr[2]];         /* Outer index from array, inner constant */
    
    /* Vector operations - results may go to registers */
    v4si vec1 = {1,2,3,4};
    v4si vec2 = {5,6,7,8};
    v4si vec3 = vec1 + vec2;          /* Vector operation to register */
    
    /* Constant vector indexing */
    int vecelem = vec1[2];            /* Constant index 2 from vector */
}

void test_vector_operations(void) {
    /* Vector extensions with constant bounds */
    v4si a = {10, 20, 30, 40};
    v4si b = {1, 2, 3, 4};
    
    /* Constant indexing in vector */
    int x = a[2];                     /* Constant index 2 */
    int y = b[3];                     /* Constant index 3 */
    
    /* Vector conditional with constant mask */
    v4si mask = {0, -1, 0, -1};       /* Constant mask vector */
    v4si result = a * mask;           /* Element-wise multiplication */
    
    /* Character vectors with constant bounds */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = chars[5];               /* Constant index 5 */
    char c2 = chars[10];              /* Constant index 10 */
    
    /* Short vectors */
    v8hi shorts = {100,200,300,400,500,600,700,800};
    short s1 = shorts[3];             /* Constant index 3 */
    short s2 = shorts[6];             /* Constant index 6 */
}

void test_struct_operations(void) {
    /* Array slicing in structs */
    struct ArrayStruct s;
    
    /* Constant indexing into struct array */
    s.data[5] = 100;                  /* Constant index 5 */
    s.data[6] = 200;                  /* Constant index 6 */
    
    /* Pointer to sub-range with constant start */
    int *p1 = &s.data[2];             /* Constant start index 2 */
    int *p2 = &s.data[10];            /* Constant start index 10 */
    
    /* Multiple elements in struct array */
    s.buffer[10] = 'X';               /* Single char */
    s.buffer[11] = 'Y';               /* Two chars total */
    s.buffer[12] = 'Z';               /* Three chars - count > 2 */
    
    /* Constant string indexing */
    const char *str = "constant string";
    char ch1 = str[3];                /* Constant index 3 */
    char ch2 = str[10];               /* Constant index 10 */
}

void test_constant_bounds_loops(void) {
    /* Loops with constant bounds that might be unrolled */
    int arr[10];
    
    /* Small constant loop - might trigger count <= 2 if unrolled as 2 ops */
    for (int i = 0; i < 2; i++) {
        arr[i] = i * 10;
    }
    
    /* Medium constant loop - count > 2 */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 20;
    }
    
    /* Larger loop with char elements */
    char buf[20];
    for (int i = 0; i < 10; i++) {
        buf[i] = 'A' + i;
    }
}

void test_switch_array_indexing(void) {
    /* Switch with constant array indexing */
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int selector = 2;
    int result;
    
    switch (selector) {
        case 0:
            result = arr[1];          /* Constant index 1 */
            break;
        case 1:
            result = arr[2];          /* Constant index 2 */
            break;
        case 2:
            result = arr[3];          /* Constant index 3 */
            break;
        default:
            result = arr[4];          /* Constant index 4 */
    }
}

void test_builtin_constant(void) {
    /* Using __builtin_constant_p with array bounds */
    int arr[20] = {[5] = 100, [10] = 200};
    
    if (__builtin_constant_p(arr[5])) {
        /* Force constant evaluation */
        int x = arr[5] + 1;
    }
    
    if (__builtin_constant_p(arr[10])) {
        int y = arr[10] * 2;
    }
}

void test_mixed_operations(void) {
    /* Complex expressions combining multiple patterns */
    struct ArrayStruct s1, s2;
    int temp[20];
    
    /* Mixed struct and array access */
    s1.data[5] = s2.data[3] + 10;     /* Both constant indices */
    
    /* Chain of constant-index operations */
    temp[0] = s1.data[1];
    temp[1] = s1.data[2];
    temp[2] = s1.data[3];             /* Multiple constant accesses */
    
    /* Vector and scalar mix */
    v4si v = {1,2,3,4};
    int sum = v[0] + v[1] + v[2] + v[3];  /* All constant indices */
    
    /* String literal with constant indices in expression */
    int len = strlen("hello");        /* String literal access */
    char mid = "hello"[2];            /* Direct constant indexing */
}

int main(void) {
    /* Execute all test functions to cover different code paths */
    test_mem_targets();               /* Covers MEM_P(target) paths */
    test_register_targets();          /* Covers non-MEM_P(target) paths */
    test_vector_operations();         /* Covers vector extensions */
    test_struct_operations();         /* Covers struct array access */
    test_constant_bounds_loops();     /* Covers loop unrolling cases */
    test_switch_array_indexing();     /* Covers switch with array indexing */
    test_builtin_constant();          /* Covers __builtin_constant_p */
    test_mixed_operations();          /* Covers complex expressions */
    
    return 0;
}
