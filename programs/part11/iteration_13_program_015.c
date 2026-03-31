#include <stdio.h>
#include <string.h>

/* GCC vector extensions for constant indexing */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* Structures with arrays for constant bounds access */
struct ArrayStruct {
    int data[20];
    char buffer[50];
};

struct FlexStruct {
    int count;
    char data[];  /* Zero-length array for special handling */
};

/* Test function 1: Designated initializers with constant ranges */
void test_designated_init() {
    /* Single element constant index */
    int arr1[10] = {[5] = 100};
    
    /* Two-element constant range (count <= 2) */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* Larger constant range with char type (count > 2, small element size) */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Even larger range for type size calculation */
    char buf2[200] = {[50 ... 100] = 'y'};  /* 51 elements */
    
    /* Mixed initialization */
    int arr3[15] = {[2 ... 5] = 42, [10 ... 12] = 99};
    
    /* Use the arrays to avoid optimization */
    printf("arr1[5] = %d\n", arr1[5]);
    printf("buf1[15] = %c\n", buf1[15]);
}

/* Test function 2: Vector extensions with constant indexing */
void test_vector_ops() {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant index access - likely goes to register (non-MEM_P) */
    int x = a[2];  /* Constant index 2 */
    int y = b[3];  /* Constant index 3 */
    
    /* Vector operations with constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si res1 = a * mask;  /* Element-wise multiplication with constant mask */
    
    /* Vector conditional with constant indices */
    v4si cmp = a > b;
    int cmp_result = cmp[1];  /* Constant index access to comparison result */
    
    /* Use results */
    printf("Vector x=%d, y=%d, cmp=%d\n", x, y, cmp_result);
}

/* Test function 3: Array slicing in structs with constant bounds */
void test_struct_array_slicing() {
    struct ArrayStruct s = {0};
    
    /* Initialize with constant bounds */
    for (int i = 0; i < 5; i++) {  /* Constant loop bound */
        s.data[i] = i * 10;
    }
    
    /* Pointer to sub-array with constant start index */
    int *p1 = &s.data[2];  /* Constant index 2 */
    int *p2 = &s.data[5];  /* Constant index 5 */
    
    /* Access through pointer with constant offset */
    int val1 = p1[0];  /* Equivalent to s.data[2] */
    int val2 = p2[3];  /* Equivalent to s.data[8] */
    
    /* String literal with constant indexing */
    char c1 = "hello"[2];  /* Constant index 2 */
    char c2 = "world"[4];  /* Constant index 4 */
    
    printf("Struct slicing: val1=%d, val2=%d, c1=%c, c2=%c\n", val1, val2, c1, c2);
}

/* Test function 4: Complex expressions with constant bounds */
void test_complex_expressions() {
    int arr[20] = {0};
    
    /* Initialize array */
    for (int i = 0; i < 20; i++) {
        arr[i] = i;
    }
    
    /* Nested array access with constant inner index */
    int x1 = arr[arr[2]];  /* arr[2] = 2, so arr[2] = 2 */
    
    /* Conditional array access with constant indices */
    int cond = 1;
    int y1 = (cond ? arr[2] : arr[3]);  /* Both indices constant */
    
    /* Switch with array indexing */
    int idx = 1;
    int result;
    switch (idx) {
        case 0: result = arr[1]; break;  /* Constant index 1 */
        case 1: result = arr[2]; break;  /* Constant index 2 */
        case 2: result = arr[3]; break;  /* Constant index 3 */
        default: result = arr[0]; break; /* Constant index 0 */
    }
    
    /* Array access in arithmetic expression */
    int z1 = arr[5] * 2 + arr[6] / 3;  /* Constant indices 5 and 6 */
    
    printf("Complex: x1=%d, y1=%d, result=%d, z1=%d\n", x1, y1, result, z1);
}

/* Test function 5: Memory vs register targeting */
void test_memory_vs_register() {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* MEM_P(target) likely true - direct array element access */
    arr[3] = 42;      /* Target is memory location */
    arr[4] = arr[5];  /* Both targets are memory */
    
    /* Non-MEM_P(target) likely - expression result to register */
    int reg1 = arr[2] + arr[3];  /* Result likely in register */
    int reg2 = arr[6] * arr[7];  /* Result likely in register */
    
    /* Mixed */
    int temp = arr[1];  /* Load to register */
    arr[8] = temp;      /* Store from register to memory */
    
    /* Constant index in pointer arithmetic */
    int *ptr = arr;
    int val1 = *(ptr + 2);  /* Constant offset 2 */
    *(ptr + 3) = 100;       /* Constant offset 3, target is memory */
    
    printf("Memory/Register: reg1=%d, reg2=%d, val1=%d\n", reg1, reg2, val1);
}

/* Test function 6: Compiler-specific extensions */
void test_compiler_extensions() {
    int arr[10] = {0};
    
    /* __builtin_constant_p with array bounds */
    if (__builtin_constant_p(arr[5])) {
        printf("arr[5] is constant at compile time\n");
    }
    
    /* Force constant evaluation */
    int x = __builtin_constant_p(5) ? arr[2] : arr[3];
    
    /* Vector shuffle with constant indices */
    v4si v1 = {10, 20, 30, 40};
    v4si v2;
    
    /* Simulate shuffle - GCC doesn't have direct vector shuffle intrinsic */
    /* but we can use union to access elements */
    union {
        v4si vec;
        int elems[4];
    } u = {.vec = v1};
    
    /* Create new vector with constant reordering */
    v4si shuffled = {u.elems[2], u.elems[0], u.elems[3], u.elems[1]};
    
    printf("Compiler extensions: shuffled[0]=%d\n", shuffled[0]);
}

/* Test function 7: Different element types and sizes */
void test_element_types() {
    /* Test with different element sizes */
    char char_arr[100] = {[10 ... 30] = 'A'};  /* 21 chars */
    short short_arr[50] = {[5 ... 15] = 255};  /* 11 shorts */
    int int_arr[30] = {[2 ... 8] = 1000};      /* 7 ints */
    long long_arr[20] = {[3 ... 7] = 9999};    /* 5 longs */
    
    /* Access with constant indices */
    char c = char_arr[20];      /* Constant index */
    short s = short_arr[10];    /* Constant index */
    int i = int_arr[5];         /* Constant index */
    long l = long_arr[4];       /* Constant index */
    
    /* Array of structs */
    struct Point {
        int x, y;
    } points[10] = {[2 ... 5] = {1, 2}};  /* 4 structs */
    
    /* Constant index into struct array */
    int point_x = points[3].x;  /* Constant index 3 */
    int point_y = points[3].y;  /* Constant index 3 */
    
    printf("Types: c=%c, s=%d, i=%d, l=%ld, point=(%d,%d)\n", 
           c, s, i, l, point_x, point_y);
}

/* Test function 8: Boundary cases for count */
void test_boundary_cases() {
    /* count = 1 */
    int arr1[10] = {[5] = 1};
    
    /* count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* count = 3 (just above threshold if threshold is 2) */
    int arr3[10] = {[1] = 1, [2] = 2, [3] = 3};
    
    /* count = 10 */
    int arr4[20] = {[0 ... 9] = 42};
    
    /* Very large count with small elements */
    char big_buf[1000] = {[100 ... 500] = 'Z'};  /* 401 chars */
    
    /* Use the arrays */
    printf("Boundaries: arr1[5]=%d, arr2[3]=%d, arr3[2]=%d, arr4[9]=%d, buf[200]=%c\n",
           arr1[5], arr2[3], arr3[2], arr4[9], big_buf[200]);
}

int main() {
    printf("Testing constant bounds array/vector operations...\n\n");
    
    test_designated_init();
    printf("---\n");
    
    test_vector_ops();
    printf("---\n");
    
    test_struct_array_slicing();
    printf("---\n");
    
    test_complex_expressions();
    printf("---\n");
    
    test_memory_vs_register();
    printf("---\n");
    
    test_compiler_extensions();
    printf("---\n");
    
    test_element_types();
    printf("---\n");
    
    test_boundary_cases();
    printf("---\n");
    
    printf("All tests completed.\n");
    
    return 0;
}
