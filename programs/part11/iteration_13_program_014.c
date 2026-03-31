/* Test program to exercise constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

/* Structures with arrays */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];  /* Flexible array member */
};

/* Test functions for different code paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    struct ArrayStruct s1;
    s1.data[5] = 42;           /* Single element - count = 1 */
    s1.data[6] = s1.data[7];   /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf[100] = {[10 ... 20] = 'x'};  /* 11 chars, count > 2 */
    memset(&buf[30], 'y', 5);             /* Constant count = 5 */
    
    /* Path: MEM_P(target) = true, count > 2, larger element type */
    int arr[50] = {[15 ... 25] = 99};     /* 11 ints, count > 2 */
    
    /* Designated initializers with constant ranges */
    int init_arr[10] = {[2 ... 5] = 42, [7 ... 9] = 77};
}

void test_non_mem_target_paths(void) {
    /* Path: MEM_P(target) = false (results in registers) */
    int arr[20] = {1,2,3,4,5,6,7,8,9,10};
    
    /* Results likely go to registers */
    int reg1 = arr[2] + arr[3];           /* Constant indices */
    int reg2 = arr[4] * arr[5] - arr[6];
    int reg3 = (arr[7] > arr[8]) ? arr[9] : arr[10];
    
    /* Complex expression with constant indexing */
    int nested = arr[arr[2]];             /* arr[3] = 4 */
    
    /* Conditional with constant indices */
    int cond = (reg1 > 10) ? arr[11] : arr[12];
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[2];      /* Constant index 2 */
    int elem2 = vec2[0];      /* Constant index 0 */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 & mask;
    
    /* Small element vector */
    v16qi char_vec = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = char_vec[5];    /* Constant index */
    char c2 = char_vec[10];   /* Constant index */
}

void test_constant_string_ops(void) {
    /* String literal with constant indexing */
    char c1 = "Hello World!"[4];      /* 'o' */
    char c2 = "Constant String"[8];   /* 't' */
    
    /* Array from string literal */
    const char *str = "Test String";
    char arr[20];
    
    /* Copy with constant bounds */
    for (int i = 0; i < 5; i++) {    /* Constant bound 5 */
        arr[i] = str[i];
    }
    
    /* Constant index in switch */
    int idx = 2;
    switch(idx) {
        case 0: arr[0] = 'a'; break;
        case 1: arr[1] = 'b'; break;
        case 2: arr[2] = 'c'; break;  /* Constant index in case */
        case 3: arr[3] = 'd'; break;
    }
}

void test_loop_unrolling(void) {
    int arr[10];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {    /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Nested loop with constant bounds */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {    /* Constant bound */
        for (int j = 0; j < 3; j++) { /* Constant bound */
            matrix[i][j] = i + j;
        }
    }
}

void test_builtin_constants(void) {
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        arr[5] = 100;
    }
    
    /* Check constant index access */
    if (__builtin_constant_p("test"[2])) {
        arr[6] = 200;
    }
}

void test_mixed_operations(void) {
    /* Combine multiple patterns in single expressions */
    struct ArrayStruct s;
    v4si vec = {10, 20, 30, 40};
    
    /* Mixed memory and register targets */
    int temp = s.data[3] + vec[1];    /* MEM_P + vector element */
    s.data[4] = temp * 2;
    
    /* Array slice through pointer */
    int *slice = &s.data[5];          /* Constant start index */
    slice[0] = 1;                     /* count = 1 */
    slice[1] = 2;                     /* count = 2 when combined? */
    
    /* Multiple constant ranges */
    int multi[20] = {
        [0 ... 4] = 1,     /* 5 elements */
        [5 ... 9] = 2,     /* 5 elements */
        [10 ... 14] = 3,   /* 5 elements */
        [15 ... 19] = 4    /* 5 elements */
    };
}

int main(void) {
    printf("Testing constant bounds coverage...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_constant_string_ops();
    test_loop_unrolling();
    test_builtin_constants();
    test_mixed_operations();
    
    printf("Tests completed.\n");
    return 0;
}
