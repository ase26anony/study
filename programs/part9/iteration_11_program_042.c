/* Test program to cover constant-bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */

/* Single element access - count = 1 */
__attribute__((noinline))
static int test_small_count_1(int *arr) {
    int sum = 0;
    /* Access exactly one element with constant index */
    sum += arr[5];  /* lo=5, hi=5, count=1 */
    return sum;
}

/* Two adjacent elements - count = 2 */
__attribute__((noinline))
static int test_small_count_2(int *arr) {
    int sum = 0;
    /* Access two adjacent elements with constant indices */
    sum += arr[2];  /* lo=2, hi=3, count=2 */
    sum += arr[3];
    return sum;
}

/* Two-element struct access */
typedef struct { int a; int b; } two_int_t;
__attribute__((noinline))
static int test_small_struct(two_int_t *arr) {
    /* Access struct members - effectively two elements */
    return arr[3].a + arr[3].b;  /* Single struct, two ints */
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Fixed-size array slice in loop */
__attribute__((noinline))
static int test_larger_constant_mem(int *arr) {
    int sum = 0;
    /* Constant bounds: i from 2 to 9 inclusive, count=8 */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Multiple constant-sized slices with different element types */
__attribute__((noinline))
static int test_mixed_sizes(char *carr, short *sarr, int *iarr) {
    int sum = 0;
    
    /* char array: 20 elements, total size = 20*8 = 160 bits */
    for (int i = 0; i < 20; ++i) {
        sum += carr[i];
    }
    
    /* short array: 10 elements, total size = 10*16 = 160 bits */
    for (int i = 5; i < 15; ++i) {
        sum += sarr[i];
    }
    
    /* int array: 5 elements, total size = 5*32 = 160 bits */
    for (int i = 10; i < 15; ++i) {
        sum += iarr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */

/* Vector shuffle with constant indices */
__attribute__((noinline))
static v4si test_vector_shuffle(v4si a, v4si b) {
    /* Create constant-bounded vector section via shuffle */
    return __builtin_shufflevector(a, b, 0, 1, 4, 5);
}

/* Vector compound literal with constant indices */
__attribute__((noinline))
static v4si test_vector_constructor(v4si v) {
    /* Extract constant-bounded section via constructor */
    return (v4si){v[0], v[1], v[2], v[3]};
}

/* Vector permutation with constant mask */
__attribute__((noinline))
static v4si test_vector_permute(v4si a) {
    /* Permute with constant indices */
    v4si mask = {3, 2, 1, 0};
    return __builtin_shuffle(a, mask);
}

/* ========== SCENARIO 4: Conditional constant bounds ========== */

/* Use volatile to prevent constant propagation from removing branches */
__attribute__((noinline))
static int test_conditional_bounds(int *arr) {
    int sum = 0;
    
    /* The compiler sees both bounds possibilities during analysis */
    if (g_volatile) {
        /* First constant bound set */
        for (int i = 0; i < 5; ++i) {
            sum += arr[i];
        }
    } else {
        /* Second constant bound set */
        for (int i = 5; i < 15; ++i) {
            sum += arr[i];
        }
    }
    
    return sum;
}

/* Nested loops with constant bounds */
__attribute__((noinline))
static int test_nested_constant_bounds(int arr[10][10]) {
    int sum = 0;
    /* Outer loop: 2 to 7 (6 iterations) */
    for (int i = 2; i < 8; ++i) {
        /* Inner loop: 3 to 8 (6 iterations) */
        for (int j = 3; j < 9; ++j) {
            sum += arr[i][j];
        }
    }
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize test arrays */
    int int_arr[100];
    char char_arr[100];
    short short_arr[100];
    two_int_t struct_arr[10];
    int matrix[10][10];
    
    /* Initialize with some data */
    for (int i = 0; i < 100; ++i) {
        int_arr[i] = i % 37;
        char_arr[i] = (i * 3) % 127;
        short_arr[i] = (i * 5) % 32767;
    }
    
    for (int i = 0; i < 10; ++i) {
        struct_arr[i].a = i * 2;
        struct_arr[i].b = i * 2 + 1;
        for (int j = 0; j < 10; ++j) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Initialize vectors */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Execute all test scenarios */
    checksum += test_small_count_1(int_arr);
    checksum += test_small_count_2(int_arr);
    checksum += test_small_struct(struct_arr);
    checksum += test_larger_constant_mem(int_arr);
    checksum += test_mixed_sizes(char_arr, short_arr, int_arr);
    
    /* Vector operations */
    v4si shuffled = test_vector_shuffle(vec1, vec2);
    checksum += shuffled[0] + shuffled[1] + shuffled[2] + shuffled[3];
    
    v4si constructed = test_vector_constructor(vec1);
    checksum += constructed[0] + constructed[1];
    
    v4si permuted = test_vector_permute(vec1);
    checksum += permuted[0] + permuted[3];
    
    checksum += test_conditional_bounds(int_arr);
    checksum += test_nested_constant_bounds(matrix);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
