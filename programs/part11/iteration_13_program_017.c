/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stdio.h>
#include <string.h>

/* Vector extension types */
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
    char data[];  /* Zero-length array */
};

/* Test functions for different paths */
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) = true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range in designated initializer */
    int arr2[10] = {[3] = 1, [4] = 2};  /* Two elements - count = 2 */
    
    /* Path: MEM_P(target) = true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars - count = 11 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 chars - count = 11 */
    
    /* Path: MEM_P(target) = true, count > 2, larger element type */
    int arr3[20] = {[5 ... 10] = 99};  /* 6 ints - count = 6 */
}

void test_non_mem_target_paths(void) {
    /* Path: MEM_P(target) = false (register targets) */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Results likely go to registers */
    int reg1 = arr[2];                    /* Single element to register */
    int reg2 = arr[3] + arr[4];           /* Expression result to register */
    int reg3 = (arr[5] > 0) ? arr[6] : arr[7];  /* Conditional to register */
    
    /* Vector operations with constant indices */
    v4si vec1 = {1, 2, 3, 4};
    int vec_elem = vec1[2];  /* Constant index 2 */
    
    /* Prevent optimization out */
    printf("%d %d %d %d\n", reg1, reg2, reg3, vec_elem);
}

void test_constant_string_indexing(void) {
    /* String literals with constant indices */
    char c1 = "hello"[2];      /* 'l' - constant index 2 */
    char c2 = "world"[4];      /* 'd' - constant index 4 */
    
    /* Array slicing from string */
    const char *slice = &"abcdefghij"[3];  /* Constant start index 3 */
    
    printf("%c %c %c\n", c1, c2, slice[0]);
}

void test_vector_operations(void) {
    /* Vector extensions with constant bounds */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector operations that might trigger the logic */
    v4si c = a + b;           /* Whole vector operation */
    int x = a[1] + b[2];      /* Mixed vector/scalar with constant indices */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = a * mask;   /* Constant vector mask */
    
    /* Small vector type with many elements */
    v16c chars = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    char c = chars[10];       /* Constant index 10 */
    
    printf("%d %d %d\n", c[0], x, masked[1]);
}

void test_nested_and_complex_accesses(void) {
    int arr[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    
    /* Nested array access with constant inner index */
    int x = arr[arr[2] / 10];  /* arr[2] = 20, 20/10 = 2, arr[2] = 20 */
    
    /* Complex expression with multiple constant indices */
    int y = arr[3] * arr[4] - arr[5] / arr[6];
    
    /* Conditional with constant indices in both branches */
    int z = (arr[0] > 0) ? arr[1] : arr[2];
    
    /* Array access in loop with constant bounds (might be unrolled) */
    int sum = 0;
    for (int i = 0; i < 5; i++) {  /* Constant bound 5 */
        sum += arr[i];
    }
    
    printf("%d %d %d %d\n", x, y, z, sum);
}

void test_switch_with_array_indexing(void) {
    int arr[10] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    int selector = 2;
    
    /* Switch with constant indices in cases */
    switch (selector) {
        case 0: printf("%d\n", arr[1]); break;  /* Constant index 1 */
        case 1: printf("%d\n", arr[2]); break;  /* Constant index 2 */
        case 2: printf("%d\n", arr[3]); break;  /* Constant index 3 */
        case 3: printf("%d\n", arr[4]); break;  /* Constant index 4 */
        default: printf("%d\n", arr[0]); break; /* Constant index 0 */
    }
}

void test_struct_array_access(void) {
    struct ArrayStruct s = {0};
    
    /* Access array within struct with constant indices */
    s.data[5] = 123;           /* Constant index 5 */
    s.buffer[10] = 'a';        /* Constant index 10 */
    
    /* Pointer to sub-range with constant start */
    int *p = &s.data[3];       /* Constant start index 3 */
    char *q = &s.buffer[5];    /* Constant start index 5 */
    
    /* Multiple element range in struct array */
    for (int i = 0; i < 4; i++) {  /* Constant bound 4 */
        s.data[10 + i] = i * 10;   /* Constant start 10 */
    }
    
    printf("%d %c %d %c\n", p[0], q[0], s.data[10], s.buffer[10]);
}

void test_builtin_constant_p(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Force constant evaluation with __builtin_constant_p */
    if (__builtin_constant_p(arr[2])) {
        printf("arr[2] is constant: %d\n", arr[2]);
    }
    
    /* Test with constant index */
    if (__builtin_constant_p("abc"[1])) {
        printf("String index is constant\n");
    }
}

void test_mixed_scenarios(void) {
    /* Combine multiple patterns in single expressions */
    
    /* Designated initializers with various ranges */
    int arr1[20] = {
        [0] = 1,
        [1 ... 3] = 2,      /* 3 elements */
        [4 ... 9] = 3,      /* 6 elements */
        [10] = 4,
        [11 ... 15] = 5,    /* 5 elements */
    };
    
    /* Different element types with constant ranges */
    short shorts[50] = {[10 ... 20] = 255};      /* 11 shorts */
    char chars[100] = {[30 ... 45] = 'Z'};       /* 16 chars */
    long longs[20] = {[5 ... 8] = 999999L};      /* 4 longs */
    
    /* Vector with constant indexing in expression */
    v8hi vshort = {0, 100, 200, 300, 400, 500, 600, 700};
    int result = vshort[3] + vshort[5] * 2;  /* Constant indices 3 and 5 */
    
    printf("%d %d %ld %d\n", shorts[15], chars[40], longs[6], result);
}

int main(void) {
    printf("Testing expr.cc coverage for lines 7691-7700\n\n");
    
    /* Execute all test functions */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_constant_string_indexing();
    test_vector_operations();
    test_nested_and_complex_accesses();
    test_switch_with_array_indexing();
    test_struct_array_access();
    test_builtin_constant_p();
    test_mixed_scenarios();
    
    printf("\nAll tests completed (compile-time coverage is the goal)\n");
    return 0;
}
