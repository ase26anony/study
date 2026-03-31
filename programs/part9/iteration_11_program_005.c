/* Test program to cover constant-bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Small vector for count <= 2 case */
typedef int v2si __attribute__((vector_size(8)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100] = {0};
    v2si small_vec = {1, 2};
    int result = 0;
    
    /* Single element access - count = 1 */
    if (g_volatile) {
        arr[5] = 10;           /* Constant lower and upper bound = 5 */
        result += arr[5];
    } else {
        arr[10] = 20;          /* Different constant bound */
        result += arr[10];
    }
    
    /* Two adjacent elements - count = 2 */
    /* Using struct to force adjacent memory access */
    struct two_ints { int a; int b; } pair;
    pair.a = arr[2];           /* lo_index = 2, hi_index = 3 */
    pair.b = arr[3];
    result += pair.a + pair.b;
    
    /* Vector element access - still count <= 2 */
    int* vptr = (int*)&small_vec;
    result += vptr[0] + vptr[1];  /* Access both elements */
    
    return result;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_bounds(void) {
    int arr[100];
    char char_arr[200];
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) arr[i] = i;
    for (int i = 0; i < 200; i++) char_arr[i] = i % 100;
    
    /* Access 8 ints - count = 8, total bits = 8 * 32 = 256 */
    /* Use volatile condition to prevent constant propagation */
    int start = g_volatile ? 10 : 20;
    for (int i = start; i < start + 8; ++i) {  /* hi_index = start+7 */
        result += arr[i];
    }
    
    /* Access 16 chars - count = 16, total bits = 16 * 8 = 128 */
    /* Different constant bounds */
    int char_start = g_volatile ? 5 : 15;
    for (int i = char_start; i < char_start + 16; ++i) {
        result += char_arr[i];
    }
    
    /* Fixed bounds with compile-time constants */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        arr[i] = arr[i] * 2;
        result += arr[i];
    }
    
    return result;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si result;
    
    /* Vector shuffle with constant indices - not a memory reference */
    /* This creates VEC_PERM_EXPR or CONSTRUCTOR nodes */
    result = __builtin_shufflevector(a, b, 0, 1, 2, 3);  /* Constant indices */
    
    /* Vector compound literal with constant indices */
    v4si slice = (v4si){a[0], a[1], a[2], a[3]};  /* All constant indices */
    
    /* Conditional to use different constant bounds */
    if (g_volatile) {
        /* Extract specific elements with constant indices */
        v4si perm = __builtin_shufflevector(a, b, 1, 3, 0, 2);
        result += perm;
    } else {
        /* Different constant indices */
        v4si perm = __builtin_shufflevector(a, b, 3, 2, 1, 0);
        result += perm;
    }
    
    return result + slice;
}

/* ========== SCENARIO 4: Mixed array section with struct ========== */
__attribute__((noinline))
static int test_struct_array_section(void) {
    struct element {
        int x;
        int y;
        int z;
    } arr[50];
    
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        arr[i].x = i;
        arr[i].y = i * 2;
        arr[i].z = i * 3;
    }
    
    /* Access struct members with constant bounds */
    /* This creates array section with count = 3 * element_count */
    int start = g_volatile ? 5 : 10;
    for (int i = start; i < start + 4; ++i) {  /* 4 structs, 12 ints total */
        result += arr[i].x + arr[i].y + arr[i].z;
    }
    
    return result;
}

/* ========== SCENARIO 5: Multi-dimensional array ========== */
__attribute__((noinline))
static int test_multi_dim_array(void) {
    int matrix[10][10];
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Constant bounds in both dimensions */
    int row_start = g_volatile ? 2 : 3;
    int col_start = g_volatile ? 1 : 2;
    
    for (int i = row_start; i < row_start + 3; ++i) {      /* 3 rows */
        for (int j = col_start; j < col_start + 4; ++j) {  /* 4 columns */
            result += matrix[i][j];
        }
    }
    
    return result;
}

/* ========== SCENARIO 6: Vector with constant indices ========== */
__attribute__((noinline))
static v4sf test_vector_constant_indices(void) {
    v4sf vec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf result = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Access vector elements with constant indices */
    /* These may not be MEM_REFs but ARRAY_REFs or others */
    float temp[4];
    
    /* Store with constant indices */
    temp[0] = vec[0];
    temp[1] = vec[1];
    temp[2] = vec[2];
    temp[3] = vec[3];
    
    /* Load with constant indices */
    result = (v4sf){temp[0], temp[1], temp[2], temp[3]};
    
    /* Conditional constant bounds */
    if (g_volatile) {
        result += (v4sf){vec[1], vec[2], vec[3], vec[0]};
    } else {
        result += (v4sf){vec[3], vec[2], vec[1], vec[0]};
    }
    
    return result;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int checksum = 0;
    
    /* Call all test functions */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_bounds();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; i++) checksum += vec_result[i];
    
    checksum += test_struct_array_section();
    checksum += test_multi_dim_array();
    
    v4sf float_vec = test_vector_constant_indices();
    for (int i = 0; i < 4; i++) checksum += (int)float_vec[i];
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
