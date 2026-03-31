/* Test program to cover constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile_zero = 0;
static volatile int g_volatile_one = 1;

/* Vector types for non-MEM_P scenarios */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

/* Scenario 1: Small element count (count <= 2) - MEM_P path */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100] = {0};
    int sum = 0;
    
    /* Single element access - count = 1 */
    arr[5] = 42;
    sum += arr[5];
    
    /* Two adjacent elements - count = 2 */
    arr[10] = 1;
    arr[11] = 2;
    sum += arr[10] + arr[11];
    
    /* Use volatile to prevent constant folding */
    int idx = g_volatile_zero ? 20 : 30;
    arr[idx] = 3;
    arr[idx + 1] = 4;
    sum += arr[idx] + arr[idx + 1];
    
    return sum;
}

/* Scenario 2: Larger constant-sized access - MEM_P path */
__attribute__((noinline))
static int test_larger_constant_memory(void) {
    int arr[100] = {0};
    int sum = 0;
    
    /* Access 10 elements - TYPE_SIZE(int) * 10 fits in uhwi */
    for (int i = 2; i < 12; ++i) {
        arr[i] = i * 2;
        sum += arr[i];
    }
    
    /* Different element type (char) with larger count */
    char carr[256] = {0};
    for (int i = 0; i < 100; ++i) {
        carr[i] = i % 128;
        sum += carr[i];
    }
    
    /* Use volatile in bounds to prevent loop unrolling from changing analysis */
    int start = g_volatile_zero ? 0 : 50;
    for (int i = start; i < start + 20; ++i) {
        arr[i] = i;
        sum += arr[i];
    }
    
    return sum;
}

/* Scenario 3: Non-memory reference (!MEM_P) - vector operations */
__attribute__((noinline))
static v4si test_non_memory_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    v4si shuffled = __builtin_shufflevector(v1, v2, 0, 1, 4, 5);
    
    /* Vector compound literal with constant indices */
    v4si constructed = (v4si){v1[0], v1[1], v2[0], v2[1]};
    
    /* Vector section using GCC vector extension syntax */
    typedef int v2si __attribute__((vector_size(8)));
    v2si low_half = __builtin_convertvector((v4si){v1[0], v1[1], 0, 0}, v2si);
    
    /* Combine results */
    return shuffled + constructed + __builtin_convertvector(low_half, v4si);
}

/* Scenario 4: Mixed array section with struct elements */
__attribute__((noinline))
static int test_struct_array_sections(void) {
    struct point {
        int x;
        int y;
    } points[10];
    
    int sum = 0;
    
    /* Access struct array elements with constant bounds */
    for (int i = 0; i < 5; ++i) {
        points[i].x = i * 2;
        points[i].y = i * 3;
        sum += points[i].x + points[i].y;
    }
    
    /* Two-element access pattern */
    points[6].x = 100;
    points[7].x = 200;
    sum += points[6].x + points[7].x;
    
    return sum;
}

/* Scenario 5: Vector memory access with constant bounds */
__attribute__((noinline))
static int test_vector_memory_access(void) {
    v4si vectors[10];
    int sum = 0;
    
    /* Initialize vectors */
    for (int i = 0; i < 10; ++i) {
        vectors[i] = (v4si){i, i+1, i+2, i+3};
    }
    
    /* Access vector elements with constant bounds */
    for (int i = 0; i < 4; ++i) {
        sum += vectors[0][i];
    }
    
    /* Access multiple vector elements */
    sum += vectors[1][0] + vectors[1][1];
    
    /* Use volatile to control which elements are accessed */
    int base = g_volatile_one ? 2 : 3;
    for (int i = base; i < base + 2; ++i) {
        for (int j = 0; j < 4; ++j) {
            sum += vectors[i][j];
        }
    }
    
    return sum;
}

/* Scenario 6: Multi-dimensional array with constant bounds */
__attribute__((noinline))
static int test_multi_dimensional_array(void) {
    int matrix[10][10];
    int sum = 0;
    
    /* Constant bounds in both dimensions */
    for (int i = 2; i < 8; ++i) {
        for (int j = 2; j < 8; ++j) {
            matrix[i][j] = i * j;
            sum += matrix[i][j];
        }
    }
    
    /* Single row access */
    for (int j = 0; j < 5; ++j) {
        matrix[9][j] = j * 10;
        sum += matrix[9][j];
    }
    
    return sum;
}

/* Main function that exercises all scenarios */
int main(void) {
    int checksum = 0;
    
    /* Execute all test scenarios */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_memory();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; ++i) {
        checksum += vec_result[i];
    }
    
    checksum += test_struct_array_sections();
    checksum += test_vector_memory_access();
    checksum += test_multi_dimensional_array();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
