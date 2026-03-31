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
void test_mem_target_paths(void) {
    /* Path: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Memory to memory with constant index */
    
    /* Two-element range in designated initializer */
    int arr2[10] = {[2] = 100, [3] = 200};  /* Two separate elements */
    
    /* Two-element range in struct */
    struct ArrayStruct s1 = {.data = {[5] = 10, [6] = 20}};
    
    /* Path: MEM_P(target) true, count > 2, small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 chars, count > 2 */
    char buf2[50] = {[5 ... 15] = 0};      /* 11 chars */
    
    /* Path: MEM_P(target) true, count > 2, larger element type */
    int arr3[20] = {[5 ... 10] = 99};      /* 6 ints, count > 2 */
}

void test_non_mem_target_paths(void) {
    /* Path: non-MEM_P(target) - results in registers */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    
    /* Register targets with constant indexing */
    int reg1 = arr[2];                     /* Single load to register */
    int reg2 = arr[3] + arr[4];            /* Arithmetic result in register */
    int reg3 = arr[5] * arr[6];            /* Multiplication result */
    
    /* Conditional with constant indices */
    int x = 1;
    int reg4 = (x > 0) ? arr[7] : arr[8];  /* Both branches constant indices */
    
    /* Nested array access with constant inner index */
    int idx_arr[5] = {2,3,4,1,0};
    int reg5 = arr[idx_arr[2]];            /* Inner constant index 2 */
}

void test_vector_operations(void) {
    /* Vector with constant indexing */
    v4si vec1 = {1, 2, 3, 4};
    int elem1 = vec1[2];                   /* Constant vector index */
    
    /* Vector operations that might go to registers */
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;               /* Vector add, result likely in reg */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si vec4 = vec1 * mask;               /* Constant mask operation */
    
    /* Small vector type with many elements */
    v16c vchar = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    char celem = vchar[10];                /* Constant index into 16-char vector */
}

void test_constant_string_ops(void) {
    /* String literal with constant indexing */
    char c1 = "hello world"[6];            /* Constant index 6 = 'w' */
    char c2 = "test string"[3];            /* Constant index 3 = 't' */
    
    /* Array initialized from string literal */
    char arr[] = "constant string";
    char c3 = arr[8];                      /* Constant index 8 = 's' */
}

void test_loop_unrolling(void) {
    /* Loop with constant bounds that might unroll */
    int arr[10];
    
    /* Small constant loop - might unroll completely */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 2;                    /* Constant bound 5 */
    }
    
    /* Two-element loop */
    for (int i = 8; i < 10; i++) {
        arr[i] = i * 3;                    /* Exactly 2 iterations */
    }
}

void test_switch_cases(void) {
    int arr[10] = {0,10,20,30,40,50,60,70,80,90};
    int x = 2;
    
    switch (x) {
        case 0:
            printf("%d\n", arr[1]);        /* Constant index 1 */
            break;
        case 1:
            printf("%d\n", arr[2]);        /* Constant index 2 */
            break;
        case 2:
            printf("%d\n", arr[3]);        /* Constant index 3 */
            break;
        default:
            printf("%d\n", arr[4]);        /* Constant index 4 */
    }
}

void test_builtin_constant_p(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[2])) {
        int x = arr[2] + 10;               /* Should use constant 3 + 10 */
    }
    
    /* Test with constant index */
    if (__builtin_constant_p(arr[3])) {
        int y = arr[3] * 2;                /* Should use constant 4 * 2 */
    }
}

void test_mixed_expressions(void) {
    /* Complex expressions combining multiple patterns */
    struct ArrayStruct s;
    int arr[20];
    
    /* Mixed memory/register with constant bounds */
    int x = s.data[5] + arr[10];           /* Both constant indices */
    
    /* Chain of constant-index accesses */
    int y = arr[arr[2]];                   /* Outer index from arr[2] */
    
    /* Conditional with struct array access */
    int z = (x > 0) ? s.data[3] : s.data[4];
    
    /* Array slice via pointer with constant bounds */
    int *slice = &arr[5];                  /* Constant start index */
    slice[0] = 100;                        /* Constant offset 0 */
    slice[1] = 200;                        /* Constant offset 1 */
    
    /* Two-element range via pointer */
    int *p = &arr[10];
    p[0] = 1;  /* count = 1 */
    p[1] = 2;  /* count = 2 when considered together? */
}

void test_designated_ranges(void) {
    /* Various designated initializer ranges */
    
    /* Single element */
    int a1[10] = {[5] = 100};
    
    /* Two-element explicit range */
    int a2[10] = {[2 ... 3] = 50};         /* count = 2 */
    
    /* Three-element range */
    int a3[10] = {[6 ... 8] = 99};         /* count = 3 > 2 */
    
    /* Large range with char type */
    char c1[100] = {[20 ... 40] = 'A'};    /* count = 21 > 2 */
    
    /* Multiple ranges */
    int a4[20] = {[1 ... 3] = 1, [10 ... 12] = 2};
}

int main(void) {
    printf("Testing constant bounds coverage...\n");
    
    /* Execute all test patterns */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_constant_string_ops();
    test_loop_unrolling();
    test_switch_cases();
    test_builtin_constant_p();
    test_mixed_expressions();
    test_designated_ranges();
    
    printf("All tests completed (runtime behavior is secondary).\n");
    return 0;
}
