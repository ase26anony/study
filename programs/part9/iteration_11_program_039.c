/* Test program to cover constant bounds analysis in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile = 0;

/* Vector types for non-MEM_P scenarios */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Scenario 1: Small element count (count <= 2) - MEM_P path */
__attribute__((noinline))
static int test_small_count_mem(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Single element access - count = 1 */
    sum += arr[5];  /* const_bounds_p: lo=5, hi=5, count=1 */
    
    /* Two adjacent elements - count = 2 */
    sum += arr[10] + arr[11];  /* lo=10, hi=11, count=2 */
    
    /* Use volatile to prevent constant folding */
    int idx = g_volatile ? 20 : 30;
    sum += arr[idx];  /* Variable index to keep array alive */
    
    return sum;
}

/* Scenario 2: Larger constant-sized memory access - MEM_P path */
__attribute__((noinline))
static int test_larger_constant_mem(void) {
    char buffer[256];
    int sum = 0;
    
    /* Initialize buffer */
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)(i % 128);
    }
    
    /* Access 8 adjacent chars - TYPE_SIZE=8 bits, count=8, total=64 bits */
    for (int i = 0; i < 8; i++) {
        sum += buffer[50 + i];  /* lo=50, hi=57, count=8 */
    }
    
    /* Access 4 ints - TYPE_SIZE=32 bits, count=4, total=128 bits */
    int int_arr[64];
    for (int i = 0; i < 64; i++) {
        int_arr[i] = i * 3;
    }
    
    for (int i = 2; i < 6; i++) {  /* lo=2, hi=5, count=4 */
        sum += int_arr[i];
    }
    
    /* Use volatile condition to preserve loop */
    int start = g_volatile ? 10 : 20;
    for (int i = start; i < start + 4; i++) {
        sum += int_arr[i];
    }
    
    return sum;
}

/* Scenario 3: Non-memory reference (!MEM_P) - vector operations */
__attribute__((noinline))
static v4si test_non_mem_vector(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    v4si c = __builtin_shufflevector(a, b, 0, 1, 4, 5);  /* {1, 2, 5, 6} */
    
    /* Vector compound literal with constant indices */
    v4si d = (v4si){a[0], a[1], b[0], b[1]};  /* CONSTRUCTOR node */
    
    /* Vector section with constant bounds */
    v2si e = (v2si){c[0], c[1]};  /* lo=0, hi=1, count=2, !MEM_P */
    
    /* Combine results */
    return c + d + (v4si){e[0], e[1], e[0], e[1]};
}

/* Scenario 4: Mixed array section with struct */
__attribute__((noinline))
static int test_struct_array(void) {
    struct point {
        int x;
        int y;
    } points[50];
    
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        points[i].x = i;
        points[i].y = i * 2;
    }
    
    /* Access 3 adjacent structs - each struct is 2 ints = 64 bits */
    /* Total: 3 * 64 = 192 bits, fits in uhwi */
    for (int i = 10; i < 13; i++) {  /* lo=10, hi=12, count=3 */
        sum += points[i].x + points[i].y;
    }
    
    return sum;
}

/* Scenario 5: Float vector with constant bounds */
__attribute__((noinline))
static float test_float_vector(void) {
    v4sf vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float arr[4];
    
    /* Store vector elements to array with constant indices */
    for (int i = 0; i < 4; i++) {
        arr[i] = vec[i];  /* Each access: lo=hi=i, count=1 */
    }
    
    /* Access 2 adjacent floats */
    float sum = arr[0] + arr[1];  /* lo=0, hi=1, count=2 */
    
    /* Conditional constant bounds */
    int start = g_volatile ? 0 : 2;
    sum += arr[start] + arr[start + 1];  /* May be {0,1} or {2,3} */
    
    return sum;
}

/* Main function that exercises all scenarios */
int main(void) {
    int checksum = 0;
    
    /* Test small count memory accesses */
    checksum += test_small_count_mem();
    
    /* Test larger constant-sized memory accesses */
    checksum += test_larger_constant_mem();
    
    /* Test non-memory vector operations */
    v4si vec_result = test_non_mem_vector();
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Test struct array accesses */
    checksum += test_struct_array();
    
    /* Test float vector accesses */
    float float_result = test_float_vector();
    checksum += (int)float_result;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
