/* Test program to cover constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

/* Scenario 1: Small element count (count <= 2) - MEM_P path */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Single element access - count = 1 */
    if (g_volatile) {
        sum += arr[5];  /* Constant lower and upper bound = 5 */
    } else {
        sum += arr[10]; /* Different constant bound */
    }
    
    /* Two adjacent elements - count = 2 */
    if (g_volatile) {
        sum += arr[20] + arr[21];  /* lo=20, hi=21, count=2 */
    } else {
        sum += arr[30] + arr[31];  /* lo=30, hi=31, count=2 */
    }
    
    /* Using small struct for 2-element access */
    struct two_ints { int a; int b; } s;
    s.a = arr[40];
    s.b = arr[41];  /* lo=40, hi=41, count=2 */
    sum += s.a + s.b;
    
    /* Vector type with 2 elements */
    v2si v2;
    for (int i = 0; i < 2; i++) {
        v2[i] = arr[50 + i];  /* lo=50, hi=51, count=2 */
    }
    sum += v2[0] + v2[1];
    
    return sum;
}

/* Scenario 2: Larger constant-sized memory access - MEM_P path */
__attribute__((noinline))
static int test_larger_constant_memory(void) {
    char char_arr[256];
    int int_arr[64];
    float float_arr[32];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) char_arr[i] = i;
    for (int i = 0; i < 64; i++) int_arr[i] = i * 3;
    for (int i = 0; i < 32; i++) float_arr[i] = i * 1.5f;
    
    /* char array: TYPE_SIZE = 8 bits, count = 8, total = 64 bits */
    for (int i = 10; i < 18; ++i) {  /* lo=10, hi=17, count=8 */
        sum += char_arr[i];
    }
    
    /* int array: TYPE_SIZE = 32 bits, count = 4, total = 128 bits */
    for (int i = 20; i < 24; ++i) {  /* lo=20, hi=23, count=4 */
        sum += int_arr[i];
    }
    
    /* float array: TYPE_SIZE = 32 bits, count = 8, total = 256 bits */
    for (int i = 5; i < 13; ++i) {  /* lo=5, hi=12, count=8 */
        sum += (int)float_arr[i];
    }
    
    /* Mixed bounds using conditional */
    int lo = g_volatile ? 2 : 3;
    int hi = g_volatile ? 10 : 15;
    /* The bounds themselves are constant within each path */
    if (g_volatile) {
        for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
            sum += char_arr[i];
        }
    } else {
        for (int i = 3; i < 15; ++i) {  /* lo=3, hi=14, count=12 */
            sum += char_arr[i];
        }
    }
    
    return sum;
}

/* Scenario 3: Non-memory reference cases (!MEM_P) */
__attribute__((noinline))
static v4si test_non_memory_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si result;
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    result = __builtin_shufflevector(v1, v2, 0, 2, 1, 3);  /* Constant indices */
    
    /* Vector compound literal with constant indices */
    v4si slice = (v4si){v1[0], v1[1], v1[2], v1[3]};  /* All constant indices */
    
    /* Vector operation that yields constructor */
    v4si sum = result + slice;
    
    /* Another shuffle with different constant bounds */
    v4si perm = __builtin_shufflevector(v1, v2, 3, 2, 1, 0);
    
    return sum + perm;
}

/* Scenario 4: Mixed vector and array with constant bounds */
__attribute__((noinline))
static int test_mixed_vector_array(void) {
    v4si vectors[10];
    int arr[100];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        vectors[i] = (v4si){i*4, i*4+1, i*4+2, i*4+3};
    }
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Access vector elements with constant indices */
    for (int i = 0; i < 4; i++) {  /* lo=0, hi=3, count=4 */
        sum += vectors[2][i];  /* Constant vector index 2, element indices 0-3 */
    }
    
    /* Array section within vector elements */
    v4si v = vectors[3];
    sum += v[0] + v[1] + v[2] + v[3];  /* All constant indices */
    
    /* Conditional constant bounds */
    int start = g_volatile ? 0 : 1;
    int end = g_volatile ? 2 : 3;
    
    if (g_volatile) {
        for (int i = 0; i < 2; i++) {  /* lo=0, hi=1, count=2 */
            sum += vectors[4][i];
        }
    } else {
        for (int i = 1; i < 3; i++) {  /* lo=1, hi=2, count=2 */
            sum += vectors[4][i];
        }
    }
    
    return sum;
}

/* Scenario 5: Multi-dimensional array with constant bounds */
__attribute__((noinline))
static int test_multi_dim_array(void) {
    int matrix[10][20];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Constant row, constant column range */
    for (int j = 5; j < 15; j++) {  /* lo=5, hi=14, count=10 */
        sum += matrix[3][j];
    }
    
    /* Constant column, varying row with constant bounds */
    for (int i = 2; i < 8; i++) {  /* lo=2, hi=7, count=6 */
        sum += matrix[i][10];
    }
    
    return sum;
}

/* Main function that exercises all scenarios */
int main(void) {
    int checksum = 0;
    
    printf("Testing constant bounds analysis coverage...\n");
    
    /* Test all scenarios */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_memory();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_vector_array();
    checksum += test_multi_dim_array();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
