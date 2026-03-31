/* Test program to cover constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing our test cases */
static volatile int g_volatile = 0;

/* Vector types for non-MEM_P cases */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small count (count <= 2) for MEM_P ========== */
__attribute__((noinline))
static int test_small_count_mem(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* Single element access - count = 1 */
    sum += arr[5];  /* lo=5, hi=5, count=1 */
    
    /* Two adjacent elements - count = 2 */
    sum += arr[10] + arr[11];  /* lo=10, hi=11, count=2 */
    
    /* Use volatile to prevent constant folding */
    int idx = g_volatile ? 20 : 30;
    sum += arr[idx];  /* Variable index, but bounds are constant in the checks */
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized access ========== */
__attribute__((noinline))
static int test_larger_constant_access(void) {
    char buffer[256];
    int sum = 0;
    
    /* Initialize buffer */
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)(i % 128);
    }
    
    /* Access 16 chars - TYPE_SIZE=8 bits, count=16, total=128 bits */
    for (int i = 32; i < 48; i++) {  /* lo=32, hi=47, count=16 */
        sum += buffer[i];
    }
    
    /* Access 4 ints - TYPE_SIZE=32 bits, count=4, total=128 bits */
    int int_arr[50];
    for (int i = 0; i < 50; i++) {
        int_arr[i] = i * 2;
    }
    
    for (int i = 10; i < 14; i++) {  /* lo=10, hi=13, count=4 */
        sum += int_arr[i];
    }
    
    /* Mixed bounds with constant expressions */
    int start = g_volatile ? 5 : 15;
    for (int i = start; i < start + 8; i++) {  /* Constant count=8 */
        sum += int_arr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-MEM_P vector operations ========== */
__attribute__((noinline))
static v4si test_non_mem_vector(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    v4si c = __builtin_shufflevector(a, b, 0, 1, 4, 5);  /* {1, 2, 5, 6} */
    
    /* Vector compound literal with constant indices */
    v4si d = (v4si){a[0], a[1], b[0], b[1]};  /* CONSTRUCTOR node */
    
    /* Vector section with constant bounds */
    v4si e;
    for (int i = 0; i < 2; i++) {  /* lo=0, hi=1, count=2 */
        e[i] = a[i] + b[i];
    }
    for (int i = 2; i < 4; i++) {  /* lo=2, hi=3, count=2 */
        e[i] = a[i] * b[i];
    }
    
    return c + d + e;
}

/* ========== SCENARIO 4: Mixed array/vector with struct ========== */
typedef struct {
    int x;
    int y;
} Point;

__attribute__((noinline))
static int test_struct_array(void) {
    Point points[20];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 20; i++) {
        points[i].x = i;
        points[i].y = i * 2;
    }
    
    /* Access two adjacent structs - count=2 elements of type Point */
    sum += points[5].x + points[5].y;    /* Single element */
    sum += points[6].x + points[6].y;    /* Two adjacent elements */
    
    /* Access struct members with constant indices */
    int idx = g_volatile ? 8 : 12;
    for (int i = idx; i < idx + 3; i++) {  /* lo=idx, hi=idx+2, count=3 */
        sum += points[i].x;
    }
    
    return sum;
}

/* ========== SCENARIO 5: Multi-dimensional array ========== */
__attribute__((noinline))
static int test_multi_dim_array(void) {
    int matrix[10][10];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Constant row slice */
    for (int j = 2; j < 8; j++) {  /* lo=2, hi=7, count=6 */
        sum += matrix[3][j];
    }
    
    /* Constant column slice with volatile condition */
    int row = g_volatile ? 5 : 6;
    for (int i = row; i < row + 4; i++) {  /* lo=row, hi=row+3, count=4 */
        sum += matrix[i][4];
    }
    
    return sum;
}

/* ========== SCENARIO 6: Vector with different element sizes ========== */
__attribute__((noinline))
static int test_vector_sizes(void) {
    v8hi v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v4sf v2 = {1.0f, 2.0f, 3.0f, 4.0f};
    
    int sum = 0;
    
    /* Access short vector elements - TYPE_SIZE=16 bits */
    for (int i = 0; i < 4; i++) {  /* lo=0, hi=3, count=4, total=64 bits */
        sum += v1[i];
    }
    
    /* Access float vector elements - TYPE_SIZE=32 bits */
    for (int i = 2; i < 4; i++) {  /* lo=2, hi=3, count=2, total=64 bits */
        sum += (int)v2[i];
    }
    
    /* Shuffle with constant bounds */
    v8hi v3 = __builtin_shufflevector(v1, v1, 0, 2, 4, 6, 1, 3, 5, 7);
    
    for (int i = 0; i < 8; i++) {
        sum += v3[i];
    }
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_mem();
    checksum += test_larger_constant_access();
    
    v4si vec_result = test_non_mem_vector();
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    checksum += test_struct_array();
    checksum += test_multi_dim_array();
    checksum += test_vector_sizes();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
