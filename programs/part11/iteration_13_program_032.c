/* Test program to exercise constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

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

/* Test functions for different code paths */
void test_mem_p_target() {
    /* MEM_P(target) true path - array elements in memory */
    int arr1[100];
    
    /* Single element access - count = 1 */
    arr1[10] = 42;  /* Line 7691-7700: const_bounds_p true, count=1 */
    
    /* Two element range - count = 2 */
    arr1[20] = 1;
    arr1[21] = 2;   /* Could trigger count <= 2 path */
    
    /* Designated initializer with constant range */
    int arr2[50] = {[5 ... 7] = 99};  /* count = 3, MEM_P true */
    
    /* Larger range with char elements - count > 2, small type size */
    char buf1[200] = {[30 ... 45] = 'A'};  /* count = 16, char size = 1 */
    
    /* Even larger range to ensure count > 2 */
    int arr3[100] = {[10 ... 25] = 777};  /* count = 16, int size = 4 */
}

void test_non_mem_p_target() {
    /* Non-MEM_P(target) path - results in registers */
    int arr[50] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Expression favoring register allocation */
    int x = arr[2] + arr[3] * arr[4];  /* Results likely in register */
    
    /* Multiple operations with constant indices */
    int y = arr[arr[5]];  /* Nested access with constant inner index */
    
    /* Conditional with constant indices */
    int z = (x > 0 ? arr[6] : arr[7]);
    
    /* Complex expression */
    int w = arr[8] + (arr[9] << 2) - arr[10] / 4;
    
    /* Prevent unused variable warnings */
    (void)x; (void)y; (void)z; (void)w;
}

void test_vector_operations() {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem = vec1[2];  /* Constant index 2 */
    
    /* Vector operations that might use registers */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    /* Vector with char elements - small type size */
    v16c char_vec = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    char c = char_vec[10];  /* Constant index, char type */
    
    /* Prevent unused variable warnings */
    (void)elem; (void)vec3; (void)vec4; (void)c;
}

void test_constant_string_indexing() {
    /* String literal with constant indices */
    char c1 = "Hello, World!"[7];  /* Constant index 7 = 'W' */
    char c2 = "Test String"[5];    /* Constant index 5 = 'S' */
    
    /* Array initialized from string */
    char arr[] = "Constant bounds test";
    char c3 = arr[10];  /* Constant index 10 */
    
    /* Prevent unused variable warnings */
    (void)c1; (void)c2; (void)c3;
}

void test_struct_array_access() {
    /* Structure with array member */
    struct ArrayStruct s = {0};
    
    /* Constant index into struct array */
    s.data[5] = 100;
    s.data[6] = 200;  /* Two-element pattern */
    
    /* Pointer to sub-range with constant start */
    int *p1 = &s.data[10];  /* Constant start index 10 */
    int *p2 = &s.data[15];  /* Constant start index 15 */
    
    /* Char array in struct with range */
    s.buffer[20] = 'X';
    s.buffer[21] = 'Y';
    s.buffer[22] = 'Z';  /* Three-element range, char type */
    
    /* Prevent unused variable warnings */
    (void)p1; (void)p2;
}

void test_loop_with_constant_bounds() {
    int arr[20];
    
    /* Loop with constant bound - might be unrolled */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 10;  /* Constant bound 5 */
    }
    
    /* Nested loop with constant bounds */
    int matrix[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i + j;  /* Constant bounds 4 */
        }
    }
}

void test_switch_with_array_indexing() {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int index = 3;
    
    /* Switch with constant indices in cases */
    switch (index) {
        case 0: 
            printf("%d\n", arr[1]);  /* Constant index 1 */
            break;
        case 1:
            printf("%d\n", arr[2]);  /* Constant index 2 */
            break;
        case 2:
            printf("%d\n", arr[3]);  /* Constant index 3 */
            break;
        case 3:
            printf("%d\n", arr[4]);  /* Constant index 4 */
            break;
        default:
            printf("%d\n", arr[0]);  /* Constant index 0 */
    }
}

void test_builtin_constant_p() {
    int arr[10] = {0};
    
    /* Force constant evaluation with __builtin_constant_p */
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        arr[5] = 1;
    }
    
    /* Test with constant index */
    if (__builtin_constant_p("test"[2])) {
        arr[6] = 2;
    }
}

void test_vector_masks() {
    /* Vector with constant mask */
    v4si a = {1, 2, 3, 4};
    v4si mask = {0, -1, 0, -1};  /* Constant vector mask */
    v4si result = a * mask;      /* Vector conditional operation */
    
    /* Prevent unused variable warning */
    (void)result;
}

void test_mixed_operations() {
    /* Combine multiple patterns in complex expressions */
    struct ArrayStruct s = {0};
    int arr[30];
    
    /* Complex expression with multiple constant indices */
    int x = s.data[5] + arr[10] * ("test"[0] - arr[15]);
    
    /* Nested array access with intermediate constant */
    int idx = 8;
    int y = arr[arr[idx]];  /* Outer index from variable, inner from array */
    
    /* Conditional with array accesses on both sides */
    int z = (x > y) ? arr[12] : arr[13];
    
    /* Loop with array initialization using constant bounds */
    for (int i = 10; i < 15; i++) {
        arr[i] = i * 2;  /* Constant range 10-14 */
    }
    
    /* Prevent unused variable warnings */
    (void)x; (void)y; (void)z;
}

int main() {
    printf("Testing constant bounds coverage for expr.cc lines 7691-7700\n");
    
    /* Execute all test functions */
    test_mem_p_target();
    test_non_mem_p_target();
    test_vector_operations();
    test_constant_string_indexing();
    test_struct_array_access();
    test_loop_with_constant_bounds();
    test_switch_with_array_indexing();
    test_builtin_constant_p();
    test_vector_masks();
    test_mixed_operations();
    
    printf("All tests completed (coverage occurs during compilation)\n");
    return 0;
}
