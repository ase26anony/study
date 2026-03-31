/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stdio.h>
#include <string.h>

/* GCC vector extensions for vector operations */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8s __attribute__((vector_size(16)));

/* Struct with array for array slicing */
struct ArrayStruct {
    int data[20];
    char buffer[100];
};

/* Struct with flexible array member */
struct FlexStruct {
    int count;
    char items[];
};

/* Test function 1: Designated initializers with constant ranges */
void test_designated_initializers(void) {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* Multiple elements (count > 2) with char type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Multiple elements with short type */
    short sarr[50] = {[5 ... 15] = 99};  /* 11 elements */
    
    /* Mixed initializations */
    int arr3[20] = {[0 ... 4] = 1, [10 ... 14] = 2, [19] = 3};
    
    /* Use the arrays to prevent optimization */
    printf("Designated: %d %d %c %d\n", arr1[5], arr2[3], buf1[15], sarr[10]);
}

/* Test function 2: Vector operations with constant indices */
void test_vector_operations(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v16c vec_char = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    
    /* Constant indexing into vectors - likely goes to register */
    int elem1 = vec1[2];  /* Non-MEM_P(target) path */
    int elem2 = vec1[1] + vec2[3];  /* Register operations */
    
    /* Vector operations with constant masks */
    v4si mask1 = {0, -1, 0, -1};
    v4si mask2 = {-1, 0, -1, 0};
    v4si res1 = vec1 * mask1;  /* Mixed constant/vector */
    v4si res2 = vec2 & mask2;
    
    /* Character vector with constant indexing */
    char c1 = vec_char[5];
    char c2 = vec_char[10];
    
    printf("Vector: %d %d %d %c %c\n", elem1, elem2, res1[1], c1, c2);
}

/* Test function 3: Array slicing through structs */
void test_struct_array_slicing(void) {
    struct ArrayStruct s = {0};
    
    /* Initialize with constant bounds */
    for (int i = 0; i < 5; i++) {  /* Constant bound loop */
        s.data[i] = i * 10;
    }
    
    /* Constant index access - MEM_P(target) true */
    s.data[3] = 100;
    s.data[4] = 200;
    
    /* Pointer to slice with constant start */
    int *slice1 = &s.data[2];  /* Start at index 2 */
    int *slice2 = &s.data[5];  /* Start at index 5 */
    
    /* Access through pointer with constant offset */
    slice1[0] = 50;  /* Equivalent to s.data[2] */
    slice1[1] = 60;  /* Equivalent to s.data[3] */
    
    /* Character buffer with constant range */
    memset(&s.buffer[10], 'A', 15);  /* 15 chars starting at index 10 */
    
    printf("Struct slice: %d %d %c\n", slice1[0], s.data[3], s.buffer[12]);
}

/* Test function 4: String literal constant indexing */
void test_string_indexing(void) {
    const char *str = "Hello, World!";
    
    /* Constant indices on string literal */
    char c1 = "Hello"[2];      /* 'l' */
    char c2 = "World"[4];      /* 'd' */
    char c3 = str[7];          /* 'W' */
    
    /* Array of strings with constant indexing */
    const char *strings[] = {"one", "two", "three", "four"};
    char c4 = strings[2][1];   /* 'h' from "three" */
    
    printf("String: %c %c %c %c\n", c1, c2, c3, c4);
}

/* Test function 5: Complex expressions with constant bounds */
void test_complex_expressions(void) {
    int matrix[5][5] = {0};
    int arr[20] = {0};
    
    /* Initialize with constant bounds */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 5 + j;
        }
    }
    
    /* Nested array access with constant indices */
    int val1 = matrix[2][3];
    int val2 = matrix[arr[2] % 3][4];  /* Mixed constant/variable */
    
    /* Conditional array access with constant indices */
    int x = 1;
    int val3 = (x > 0) ? arr[5] : arr[6];
    int val4 = (x == 0) ? matrix[1][2] : matrix[3][4];
    
    /* Switch with array indexing */
    switch (x) {
        case 0:
            val3 = arr[1];
            break;
        case 1:
            val3 = arr[2];
            break;
        case 2:
            val3 = arr[3];
            break;
    }
    
    /* __builtin_constant_p with array bounds */
    if (__builtin_constant_p(arr[5])) {
        printf("arr[5] is constant at compile time\n");
    }
    
    /* Multi-dimensional constant slicing */
    int *row = matrix[2];  /* Constant row index */
    int elem = row[3];     /* Constant column index */
    
    printf("Complex: %d %d %d %d\n", val1, val2, val3, elem);
}

/* Test function 6: Small element types with large ranges */
void test_small_element_ranges(void) {
    /* Character arrays with various ranges */
    char small_range[5] = {[1 ... 2] = 'a'};      /* count = 2 */
    char medium_range[50] = {[10 ... 25] = 'b'};  /* count = 16 */
    char large_range[200] = {[0 ... 99] = 'c'};   /* count = 100 */
    
    /* Short arrays */
    short shorts[100] = {[20 ... 40] = 255};      /* count = 21 */
    
    /* Mixed access patterns */
    small_range[0] = small_range[3];  /* Single element access */
    medium_range[15] = medium_range[20] + 1;
    
    /* Pointer arithmetic with constant bounds */
    char *p1 = &large_range[10];
    char *p2 = &large_range[90];
    int count = p2 - p1;  /* Constant difference: 80 */
    
    printf("Small elements: %c %c %d %d\n", 
           small_range[1], medium_range[20], shorts[30], count);
}

/* Test function 7: Register vs memory targets */
void test_register_memory_targets(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Operations that likely target registers */
    int reg_target1 = arr[2] + arr[3];      /* Non-MEM_P(target) */
    int reg_target2 = arr[4] * arr[5];      /* Non-MEM_P(target) */
    int reg_target3 = arr[6] | arr[7];      /* Non-MEM_P(target) */
    
    /* Operations that target memory */
    arr[0] = reg_target1;                   /* MEM_P(target) true */
    arr[1] = reg_target2 + reg_target3;     /* MEM_P(target) true */
    
    /* Mixed in expression */
    int complex = (arr[2] + arr[3]) * (arr[4] - arr[5]);
    arr[8] = complex;                       /* MEM_P(target) true */
    
    printf("Register/memory: %d %d %d\n", reg_target1, reg_target2, arr[8]);
}

/* Main function combining all tests */
int main(void) {
    printf("Testing constant bounds array/vector operations...\n");
    
    test_designated_initializers();
    test_vector_operations();
    test_struct_array_slicing();
    test_string_indexing();
    test_complex_expressions();
    test_small_element_ranges();
    test_register_memory_targets();
    
    printf("All tests completed.\n");
    return 0;
}
