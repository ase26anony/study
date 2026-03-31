/* Test program to cover constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing our test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference tests */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Small vector for count <= 2 test */
typedef int v2si __attribute__((vector_size(8)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100] = {0};
    v2si small_vec = {1, 2};
    int sum = 0;
    
    /* Single element access - count = 1 */
    if (g_volatile) {
        arr[5] = 10;  /* Constant lower and upper bound: 5 */
        sum += arr[5];
    } else {
        arr[10] = 20; /* Constant lower and upper bound: 10 */
        sum += arr[10];
    }
    
    /* Two adjacent elements - count = 2 */
    int* ptr = arr;
    ptr[3] = 30;  /* Access elements 3 and 4 */
    ptr[4] = 40;
    sum += ptr[3] + ptr[4];
    
    /* Vector with 2 elements */
    small_vec[0] = 50;
    small_vec[1] = 60;
    sum += small_vec[0] + small_vec[1];
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_sized_access(void) {
    int arr[100] = {0};
    char char_arr[200] = {0};
    int sum = 0;
    
    /* Access 8 ints - count = 8, total size = 8 * 32 = 256 bits */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        arr[i] = i * 10;
        sum += arr[i];
    }
    
    /* Access 32 chars - count = 32, total size = 32 * 8 = 256 bits */
    for (int i = 5; i < 37; ++i) {  /* lo=5, hi=36, count=32 */
        char_arr[i] = i;
        sum += char_arr[i];
    }
    
    /* Mixed with volatile to prevent loop unrolling from changing bounds */
    int start = g_volatile ? 20 : 15;
    for (int i = start; i < start + 16; ++i) {  /* lo=15 or 20, hi=30 or 35, count=16 */
        arr[i] = i * 2;
        sum += arr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector_ops(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a direct memory reference */
    v4si shuffled;
    if (g_volatile) {
        /* Create a constructor with constant indices */
        shuffled = (v4si){a[0], a[1], b[2], b[3]};
    } else {
        /* Different constant indices */
        shuffled = (v4si){b[0], a[1], a[2], b[3]};
    }
    
    /* Vector permute with constant mask */
    v4si permuted = __builtin_shufflevector(a, b, 0, 1, 4, 5);
    
    /* Return combination to prevent elimination */
    return shuffled + permuted;
}

/* ========== SCENARIO 4: Mixed array section with struct ========== */
__attribute__((noinline))
static int test_struct_array_section(void) {
    struct point {
        int x;
        int y;
    } points[50];
    
    int sum = 0;
    
    /* Access struct array elements with constant bounds */
    for (int i = 3; i < 12; ++i) {  /* lo=3, hi=11, count=9 */
        points[i].x = i * 10;
        points[i].y = i * 20;
        sum += points[i].x + points[i].y;
    }
    
    /* Single struct element access */
    points[25].x = 100;
    points[25].y = 200;
    sum += points[25].x + points[25].y;
    
    return sum;
}

/* ========== SCENARIO 5: Multi-dimensional array ========== */
__attribute__((noinline))
static int test_multi_dimensional_array(void) {
    int matrix[10][10];
    int sum = 0;
    
    /* Constant bounds in both dimensions */
    for (int i = 2; i < 8; ++i) {        /* lo=2, hi=7, count=6 */
        for (int j = 3; j < 9; ++j) {    /* lo=3, hi=8, count=6 */
            matrix[i][j] = i * 10 + j;
            sum += matrix[i][j];
        }
    }
    
    /* Single row access */
    for (int j = 0; j < 5; ++j) {        /* lo=0, hi=4, count=5 */
        matrix[9][j] = j * 100;
        sum += matrix[9][j];
    }
    
    return sum;
}

/* ========== SCENARIO 6: Vector with constant indices ========== */
__attribute__((noinline))
static int test_vector_constant_indices(void) {
    v4si vec = {10, 20, 30, 40};
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    int sum = 0;
    
    /* Access vector elements with constant indices */
    sum += vec[0] + vec[1] + vec[2] + vec[3];
    
    /* Conditional constant indices */
    int idx = g_volatile ? 2 : 3;
    sum += vec[idx];  /* This is NOT constant bounded */
    
    /* But this is: */
    sum += vec[g_volatile ? 2 : 1];  /* Both bounds are constants (1 or 2) */
    
    /* Short vector access */
    for (int i = 0; i < 4; ++i) {  /* lo=0, hi=3, count=4 */
        sum += short_vec[i];
    }
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_sized_access();
    
    v4si vec_result = test_non_memory_vector_ops();
    for (int i = 0; i < 4; ++i) {
        checksum += vec_result[i];
    }
    
    checksum += test_struct_array_section();
    checksum += test_multi_dimensional_array();
    checksum += test_vector_constant_indices();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
