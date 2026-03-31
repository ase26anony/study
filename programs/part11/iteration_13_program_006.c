/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stdio.h>
#include <string.h>

/* GCC vector extensions for vector operations */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* Structures with arrays for different access patterns */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];  /* Zero-length array for potential special handling */
};

/* Test functions for different code paths */
void test_mem_p_target_paths(void) {
    /* Path 1: MEM_P(target) true with count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[5] = 99; /* Two elements - count = 2 (in sequence) */
    
    /* Path 2: MEM_P(target) true with count > 2 and small element type */
    char buf[100] = {[10 ... 20] = 'x'};  /* 11 elements, char size = 1 */
    char buf2[50] = {[5 ... 15] = 'y'};   /* 11 elements */
    
    /* Path 3: MEM_P(target) true with count > 2 and larger element type */
    int arr2[50] = {[20 ... 30] = 255};   /* 11 elements, int size = 4 */
    
    /* Designated initializers with constant ranges */
    int init_arr[20] = {
        [0 ... 4] = 1,      /* 5 elements */
        [5 ... 9] = 2,      /* 5 elements */
        [10] = 3,           /* Single element */
        [11 ... 12] = 4     /* 2 elements */
    };
}

void test_non_mem_p_target_paths(void) {
    /* Operations where results likely go to registers */
    int arr[20] = {1,2,3,4,5,6,7,8,9,10};
    
    /* Register targets from array accesses */
    int reg1 = arr[2];                     /* Single element to register */
    int reg2 = arr[3] + arr[4];            /* Expression result to register */
    int reg3 = arr[5] * arr[6] - arr[7];   /* Complex expression to register */
    
    /* Conditional with constant indices */
    int x = 1;
    int reg4 = (x > 0) ? arr[8] : arr[9];  /* Both branches constant indices */
    
    /* Nested array access with constant inner index */
    int idx_arr[5] = {2, 3, 4, 1, 0};
    int reg5 = arr[idx_arr[2]];            /* idx_arr[2] is constant 4 */
    
    /* Use results to avoid dead code elimination */
    printf("Reg results: %d %d %d %d %d\n", reg1, reg2, reg3, reg4, reg5);
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[2];      /* Constant index 2 */
    int elem2 = vec2[1];      /* Constant index 1 */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;  /* Result likely in register */
    v4si vec4 = vec1 * vec2;
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 & mask;
    
    /* Small element vector */
    v16c char_vec = {0};
    char_vec[5] = 'A';        /* Single char access */
    char_vec[6] = char_vec[7] = 'B';  /* Two char access */
    
    /* Use results */
    printf("Vector: %d %d\n", elem1, elem2);
}

void test_constant_string_indexing(void) {
    /* String literal with constant indices */
    char c1 = "hello"[2];      /* 'l' - constant index 2 */
    char c2 = "world"[4];      /* 'd' - constant index 4 */
    
    /* Multiple constant indices in expression */
    int diff = "hello"[4] - "world"[0];  /* 'o' - 'w' */
    
    /* Array slicing via pointer with constant offset */
    char str[] = "abcdefghijklmnop";
    char *slice1 = &str[5];    /* Constant offset 5 */
    char *slice2 = &str[10];   /* Constant offset 10 */
    
    printf("Chars: %c %c diff: %d\n", c1, c2, diff);
}

void test_loop_with_constant_bounds(void) {
    /* Loops that might be unrolled due to constant bounds */
    int arr[10];
    
    /* Small constant bound loop */
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        arr[i] = i * 2;
    }
    
    /* Larger constant bound */
    int arr2[20];
    for (int i = 0; i < 15; i++) {  /* Constant bound 15 */
        arr2[i] = i + 1;
    }
    
    /* Nested loops with constant bounds */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * j;
        }
    }
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int x = 2;
    
    /* Switch with constant indices in cases */
    switch (x) {
        case 0:
            printf("Value: %d\n", arr[1]);  /* Constant index 1 */
            break;
        case 1:
            printf("Value: %d\n", arr[2]);  /* Constant index 2 */
            break;
        case 2:
            printf("Value: %d\n", arr[3]);  /* Constant index 3 */
            break;
        default:
            printf("Value: %d\n", arr[4]);  /* Constant index 4 */
    }
}

void test_struct_array_access(void) {
    struct ArrayStruct s;
    
    /* Constant index access to struct array member */
    s.data[5] = 100;          /* Constant index 5 */
    s.data[6] = s.data[7] = 200;  /* Two elements */
    
    /* Range access in struct array */
    for (int i = 10; i < 15; i++) {  /* Constant range 10-14 */
        s.data[i] = i * 10;
    }
    
    /* Pointer to array slice with constant offset */
    int *p = &s.data[8];      /* Constant offset 8 */
    p[0] = 300;               /* Access through pointer */
    p[1] = 400;               /* Sequential access */
}

void test_builtin_constant_p(void) {
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        printf("arr[5] is constant: %d\n", arr[5]);
    }
    
    /* Test with constant index */
    if (__builtin_constant_p(arr[3])) {
        printf("arr[3] is constant: %d\n", arr[3]);
    }
}

/* Complex expression combining multiple patterns */
int test_complex_expression(void) {
    int arr[20];
    for (int i = 0; i < 20; i++) arr[i] = i;
    
    /* Complex expression with multiple constant-index array accesses */
    int result = arr[2] + arr[5] * arr[8] - arr[12] / (arr[3] + 1);
    
    /* Conditional with array accesses */
    result += (arr[4] > 10) ? arr[6] : arr[7];
    
    /* Nested array access */
    int idx = arr[1];
    result += arr[arr[2]];  /* Inner arr[2] is constant index */
    
    return result;
}

int main(void) {
    printf("Testing various array/vector operations with constant bounds\n");
    
    /* Execute all test patterns */
    test_mem_p_target_paths();
    test_non_mem_p_target_paths();
    test_vector_operations();
    test_constant_string_indexing();
    test_loop_with_constant_bounds();
    test_switch_with_array_indexing();
    test_struct_array_access();
    
    /* Only use __builtin_constant_p if supported */
    #ifdef __GNUC__
    test_builtin_constant_p();
    #endif
    
    int complex_result = test_complex_expression();
    printf("Complex expression result: %d\n", complex_result);
    
    return 0;
}
