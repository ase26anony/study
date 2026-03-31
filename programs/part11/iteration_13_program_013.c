/* Test program to exercise constant bounds checking in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Vector extensions for GCC */
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
    /* Path 1: MEM_P(target) true, count <= 2 */
    int arr1[10];
    arr1[3] = 42;           /* Single element - count = 1 */
    arr1[4] = arr1[3] + 1;  /* Still single element access */
    
    /* Two-element range initialization */
    int arr2[10] = {[3] = 100, [4] = 200};  /* count = 2 */
    
    /* Path 2: MEM_P(target) true, count > 2 with small element type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements, char size = 1 */
    char buf2[50] = {[5 ... 15] = 'y'};    /* 11 elements */
    
    /* Path 3: MEM_P(target) true, count > 2 with larger element type */
    int arr3[20] = {[5 ... 10] = 999};  /* 6 elements, int size = 4 */
}

void test_non_mem_target_paths(void) {
    /* Non-MEM_P(target) path - results go to registers */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Register targets with constant indexing */
    int x = arr[2] + arr[3];      /* Both constant indices */
    int y = arr[4] * arr[5];      /* Multiplication result in register */
    int z = arr[6] - arr[7];      /* Subtraction result */
    
    /* Complex expression with register result */
    int w = (arr[1] << 2) | (arr[2] & 0xFF);
    
    /* Use results to prevent dead code elimination */
    printf("Register results: %d %d %d %d\n", x, y, z, w);
}

void test_vector_operations(void) {
    /* Vector with constant indexing */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant vector element access */
    int elem0 = a[0];  /* Constant index 0 */
    int elem2 = a[2];  /* Constant index 2 */
    int elem3 = a[3];  /* Constant index 3 */
    
    /* Vector operations that might trigger the logic */
    v4si c = a + b;    /* Vector addition */
    v4si d = a * b;    /* Vector multiplication */
    
    /* Vector with constant mask */
    v4si mask = {0, -1, 0, -1};
    v4si masked = a & mask;
    
    /* Small element vector */
    v16c chars = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    char c0 = chars[0];
    char c8 = chars[8];
    
    printf("Vector elements: %d %d %d\n", elem0, elem2, elem3);
}

void test_complex_expressions(void) {
    int arr[20] = {[0 ... 19] = 0};
    
    /* Nested array access with constant inner index */
    arr[0] = 5;
    int x = arr[arr[0]];  /* arr[5] - outer index from array element */
    
    /* Conditional array access with constant indices */
    int cond = 1;
    int y = (cond ? arr[2] : arr[3]);  /* Both branches constant indices */
    
    /* Array access in switch */
    switch(x) {
        case 0: y = arr[1]; break;
        case 1: y = arr[2]; break;
        case 2: y = arr[3]; break;
        default: y = arr[4]; break;
    }
    
    /* Loop with constant bounds (might be unrolled) */
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 2;
    }
    
    /* Designated initializer with range */
    int init_arr[10] = {[2 ... 5] = 42, [7 ... 9] = 99};
    
    /* String literal with constant indexing */
    char str_char = "Hello World!"[6];  /* Constant index 6 = 'W' */
    
    printf("Complex: x=%d y=%d char=%c\n", x, y, str_char);
}

void test_struct_operations(void) {
    struct ArrayStruct s;
    
    /* Access struct array with constant bounds */
    s.data[5] = 100;
    s.data[6] = 200;
    s.data[7] = 300;
    
    /* Pointer to sub-range with constant start */
    int *p = &s.data[2];  /* Constant offset 2 */
    p[0] = 10;  /* Equivalent to s.data[2] = 10 */
    p[1] = 20;  /* Equivalent to s.data[3] = 20 */
    
    /* Multiple element range in struct buffer */
    memset(&s.buffer[10], 'A', 15);  /* 15 chars starting at index 10 */
    
    /* Test with __builtin_constant_p */
    if (__builtin_constant_p(s.data[5])) {
        printf("s.data[5] is constant at compile time\n");
    }
    
    /* Force evaluation of constant bounds */
    int idx = 3;
    if (__builtin_constant_p(idx)) {
        s.data[idx] = 999;
    }
}

void test_mixed_scenarios(void) {
    /* Mix of different element sizes and counts */
    
    /* char array with various ranges */
    char small_range[10] = {[2] = 'a', [3] = 'b'};  /* count = 2 */
    char medium_range[30] = {[5 ... 15] = 'c'};     /* count = 11 */
    char large_range[100] = {[10 ... 50] = 'd'};    /* count = 41 */
    
    /* int array with various ranges */
    int int_small[5] = {[1 ... 2] = 10};      /* count = 2 */
    int int_medium[20] = {[3 ... 8] = 20};    /* count = 6 */
    int int_large[50] = {[5 ... 25] = 30};    /* count = 21 */
    
    /* short array - intermediate size */
    short short_arr[40] = {[10 ... 20] = 255};  /* count = 11, size = 2 */
    
    /* Access patterns that might trigger different branches */
    int arr[100];
    
    /* Single element - count = 1 */
    volatile int *volatile_ptr = arr;
    volatile_ptr[10] = 1234;
    
    /* Two elements - count = 2 */
    arr[20] = 1;
    arr[21] = 2;
    
    /* Many elements with small type - might trigger type size calculation */
    char char_arr[1000];
    for (int i = 100; i < 200; i++) {
        char_arr[i] = i % 256;
    }
}

int main(void) {
    printf("Testing constant bounds coverage...\n");
    
    /* Execute all test scenarios */
    test_mem_target_paths();
    test_non_mem_target_paths();
    test_vector_operations();
    test_complex_expressions();
    test_struct_operations();
    test_mixed_scenarios();
    
    printf("All tests completed.\n");
    return 0;
}
