/* Test case for GCC expr.cc constant bounds analysis coverage.
   Specifically targeting lines 7691-7700 in expr.cc.
   
   Compile with: gcc -O2 -fdump-tree-ccp1 -fprofile-arcs -ftest-coverage -o test_expr test_expr.c
   Run with: ./test_expr
*/

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile_zero = 0;

/* Vector types for non-MEM_P scenarios */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */
/* MEM_P(target) && count <= 2 */
static int __attribute__((noinline))
test_small_count_mem(void)
{
    int arr[100] = {0};
    int sum = 0;
    
    /* Single element access - count = 1 */
    if (g_volatile_zero == 0) {
        sum += arr[5];  /* lo=5, hi=5, count=1 */
    } else {
        sum += arr[10]; /* Alternative constant bound */
    }
    
    /* Two adjacent elements - count = 2 */
    if (g_volatile_zero == 0) {
        sum += arr[20] + arr[21];  /* lo=20, hi=21, count=2 */
    } else {
        sum += arr[30] + arr[31];  /* Alternative */
    }
    
    /* Using struct for 2-element access */
    struct two_ints { int a; int b; };
    struct two_ints s = {0};
    if (g_volatile_zero == 0) {
        sum += s.a + s.b;  /* Two elements, but not array syntax */
    }
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
/* MEM_P(target) && count > 2 && total size fits in unsigned HWI */
static int __attribute__((noinline))
test_larger_constant_mem(void)
{
    int arr[100] = {0};
    int sum = 0;
    
    /* Fixed-size array slice: 8 elements * 4 bytes = 32 bytes = 256 bits */
    /* TYPE_SIZE(int) = 32 bits, count = 8, total = 256 bits */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        arr[i] = i;
        sum += arr[i];
    }
    
    /* Different element type: char array, 20 elements * 1 byte = 20 bytes */
    char carr[50] = {0};
    for (int i = 5; i < 25; ++i) {  /* lo=5, hi=24, count=20 */
        carr[i] = (char)i;
        sum += carr[i];
    }
    
    /* Using volatile to prevent loop unrolling from changing bounds analysis */
    volatile int start = 30;
    if (g_volatile_zero == 0) {
        for (int i = start; i < start + 15; ++i) {  /* lo=30, hi=44, count=15 */
            arr[i] = i * 2;
            sum += arr[i];
        }
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
/* !MEM_P(target) - vector operations that return values, not memory refs */
static v4si __attribute__((noinline))
test_non_mem_vector(void)
{
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si result;
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    /* This is not a MEM_P reference */
    if (g_volatile_zero == 0) {
        /* Shuffle: take elements 0,1,2,3 from v1 (constant bounds) */
        result = __builtin_shufflevector(v1, v1, 0, 1, 2, 3);
    } else {
        /* Alternative shuffle */
        result = __builtin_shufflevector(v1, v2, 0, 1, 6, 7);
    }
    
    /* Vector compound literal with constant indices */
    v4si slice;
    if (g_volatile_zero == 0) {
        /* Constant bounds: elements 0 through 2 */
        slice = (v4si){v1[0], v1[1], v1[2], 0};  /* lo=0, hi=2, count=3 */
    } else {
        slice = (v4si){v1[1], v1[2], v1[3], 0};  /* lo=1, hi=3, count=3 */
    }
    
    /* Vector operation that creates CONSTRUCTOR nodes */
    v4si mask = {0, 1, 2, 3};
    v4si blended;
    for (int i = 0; i < 4; ++i) {
        blended[i] = (mask[i] < 2) ? v1[i] : v2[i];  /* Conditional with constant bounds */
    }
    
    return result + slice + blended;
}

/* ========== SCENARIO 4: Mixed vector memory and non-memory ========== */
static int __attribute__((noinline))
test_mixed_vector_scenarios(void)
{
    v4si vec_arr[10];
    int sum = 0;
    
    /* Initialize vector array */
    for (int i = 0; i < 10; ++i) {
        vec_arr[i] = (v4si){i*4, i*4+1, i*4+2, i*4+3};
    }
    
    /* MEM_P case: accessing vector elements from memory */
    if (g_volatile_zero == 0) {
        /* Access 2 elements from vector in memory */
        sum += vec_arr[2][0] + vec_arr[2][1];  /* lo=0, hi=1, count=2, MEM_P */
    } else {
        sum += vec_arr[3][2] + vec_arr[3][3];  /* Alternative */
    }
    
    /* Non-MEM_P case: vector shuffle result */
    v4si shuffled = __builtin_shufflevector(vec_arr[1], vec_arr[2], 0, 1, 4, 5);
    sum += shuffled[0] + shuffled[3];
    
    /* Larger constant slice from vector array in memory */
    for (int i = 0; i < 4; ++i) {  /* lo=0, hi=3, count=4 */
        sum += vec_arr[5][i];
    }
    
    return sum;
}

/* ========== SCENARIO 5: Multi-dimensional array with constant bounds ========== */
static int __attribute__((noinline))
test_multi_dim_constant_bounds(void)
{
    int matrix[10][20] = {{0}};
    int sum = 0;
    
    /* Constant bounds in both dimensions */
    for (int i = 2; i < 6; ++i) {          /* lo=2, hi=5, count=4 */
        for (int j = 3; j < 8; ++j) {      /* lo=3, hi=7, count=5 */
            matrix[i][j] = i * j;
            sum += matrix[i][j];
        }
    }
    
    /* Partial row access with constant bounds */
    if (g_volatile_zero == 0) {
        for (int j = 10; j < 15; ++j) {    /* lo=10, hi=14, count=5 */
            sum += matrix[8][j];
        }
    }
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void)
{
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_mem();
    checksum += test_larger_constant_mem();
    
    v4si vec_result = test_non_mem_vector();
    for (int i = 0; i < 4; ++i) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_vector_scenarios();
    checksum += test_multi_dim_constant_bounds();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
