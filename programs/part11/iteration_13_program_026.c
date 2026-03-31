/* Test program to exercise constant-bounds array/vector operations in GCC's expr.cc */
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
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range in designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count = 11, size = 1 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars */
    
    /* Path: MEM_P(target) = true, count > 2, larger element type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 ints - count = 6, size = 4 */
}

void test_non_mem_target_paths(void) {
    /* Path: MEM_P(target) = false (register targets) */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Results likely go to registers */
    int reg1 = arr[2];                    /* Single element to register */
    int reg2 = arr[3] + arr[4];           /* Arithmetic result to register */
    int reg3 = (arr[5] > 0) ? arr[6] : arr[7];  /* Conditional to register */
    
    /* Complex expression favoring register allocation */
    int reg4 = arr[arr[2]] + arr[arr[3]];  /* Nested accesses */
    
    /* Use results to avoid dead code elimination */
    printf("Reg results: %d %d %d %d\n", reg1, reg2, reg3, reg4);
}

void test_vector_operations(void) {
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Constant vector indexing */
    int elem1 = vec1[2];      /* Constant index 2 */
    int elem2 = vec2[0];      /* Constant index 0 */
    
    /* Vector operations that might expand with constant bounds */
    v4si vec3 = vec1 + vec2;
    int elem3 = vec3[3];      /* Constant index 3 */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    
    /* Character vectors with many elements */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c1 = chars[5];       /* Constant index 5 */
    char c2 = chars[10];      /* Constant index 10 */
    
    printf("Vector elements: %d %d %d %c %c\n", elem1, elem2, elem3, c1, c2);
}

void test_struct_array_access(void) {
    /* Array access through struct members */
    struct ArrayStruct s = {0};
    
    /* Constant indexing into struct array members */
    s.data[5] = 100;                 /* Constant index 5 */
    s.buffer[10] = 'A';              /* Constant index 10 */
    
    /* Pointer to sub-range with constant start */
    int *p1 = &s.data[2];            /* Constant start index 2 */
    char *p2 = &s.buffer[5];         /* Constant start index 5 */
    
    /* Multiple element range in struct member */
    s.buffer[20 ... 25] = 'B';       /* 6 chars - count = 6 */
    
    /* Use pointers to avoid optimization */
    *p1 = 50;
    *p2 = 'C';
    
    printf("Struct data: %d %c\n", s.data[5], s.buffer[10]);
}

void test_constant_string_ops(void) {
    /* String literal with constant indexing */
    char c1 = "Hello World!"[4];      /* Constant index 4 -> 'o' */
    char c2 = "Test String"[0];       /* Constant index 0 -> 'T' */
    
    /* Multiple character extraction */
    char str[] = "Constant bounds testing";
    char c3 = str[7];                 /* Constant index 7 -> 'b' */
    char c4 = str[8];                 /* Constant index 8 -> 'o' */
    
    printf("Chars: %c %c %c %c\n", c1, c2, c3, c4);
}

void test_loop_with_constant_bounds(void) {
    /* Loop with compile-time constant bound */
    int arr[10];
    
    /* Should be unrollable with constant bound 5 */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 2;
    }
    
    /* Nested loop with constant bounds */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i + j;
        }
    }
    
    printf("Loop results: %d %d\n", arr[2], matrix[1][2]);
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int x = 2;
    
    /* Switch with constant indices in cases */
    switch (x) {
        case 0:
            printf("Case 0: %d\n", arr[1]);  /* Constant index 1 */
            break;
        case 1:
            printf("Case 1: %d\n", arr[2]);  /* Constant index 2 */
            break;
        case 2:
            printf("Case 2: %d\n", arr[3]);  /* Constant index 3 */
            break;
        default:
            printf("Default: %d\n", arr[4]); /* Constant index 4 */
    }
}

void test_builtin_constant_p(void) {
    int arr[10] = {0};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        printf("arr[5] is constant\n");
    }
    
    /* Conditional with constant array indices */
    int x = (arr[2] == 0) ? arr[3] : arr[4];
    
    (void)x; /* Avoid unused warning */
}

int main(void) {
    printf("Testing constant bounds array/vector operations...\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_struct_array_access();
    test_constant_string_ops();
    test_loop_with_constant_bounds();
    test_switch_with_array_indexing();
    test_builtin_constant_p();
    
    /* Additional complex expression combining multiple patterns */
    {
        struct ArrayStruct s;
        v4si vec = {1, 2, 3, 4};
        
        /* Mixed operations that might trigger different code paths */
        s.data[vec[0]] = "test"[1];  /* Vector index + string index */
        int complex = s.data[1] + vec[2] + ("hello"[3] - 'a');
        
        printf("Complex result: %d\n", complex);
    }
    
    return 0;
}
