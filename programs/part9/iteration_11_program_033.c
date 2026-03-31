/* Test program for constant-bounds analysis in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

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
    
    /* Using struct for 2-element access */
    struct two_ints { int a; int b; };
    struct two_ints s;
    s.a = arr[40];
    s.b = arr[41];  /* lo=40, hi=41, count=2 */
    sum += s.a + s.b;
    
    /* Vector type with 2 elements */
    v2si v2;
    v2[0] = arr[50];
    v2[1] = arr[51];  /* lo=50, hi=51, count=2 */
    sum += v2[0] + v2[1];
    
    return sum;
}

/* Scenario 2: Larger constant-sized memory access - MEM_P path */
__attribute__((noinline))
static int test_larger_constant_mem(void) {
    int arr[200];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 200; i++) {
        arr[i] = i * 3;
    }
    
    /* Loop with constant bounds - count = 8 */
    for (int i = 10; i < 18; i++) {  /* lo=10, hi=17, count=8 */
        sum += arr[i];
    }
    
    /* Multiple constant-bounded sections */
    if (g_volatile) {
        /* Section 1: count = 5 */
        for (int i = 20; i < 25; i++) {  /* lo=20, hi=24, count=5 */
            sum += arr[i];
        }
    } else {
        /* Section 2: count = 6 */
        for (int i = 30; i < 36; i++) {  /* lo=30, hi=35, count=6 */
            sum += arr[i];
        }
    }
    
    /* Different element type (char) with known size */
    char carr[1000];
    for (int i = 0; i < 1000; i++) {
        carr[i] = i % 256;
    }
    
    /* char array with larger count but small total size */
    int char_sum = 0;
    for (int i = 100; i < 200; i++) {  /* lo=100, hi=199, count=100 */
        char_sum += carr[i];           /* TYPE_SIZE = 8 bits, total = 800 bits */
    }
    sum += char_sum;
    
    /* Mixed constant expressions in bounds */
    const int base = 5;
    for (int i = base; i < base + 12; i++) {  /* lo=5, hi=16, count=12 */
        sum += arr[i];
    }
    
    return sum;
}

/* Scenario 3: Non-memory reference cases - !MEM_P path */
__attribute__((noinline))
static v4si test_non_mem_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    v4si shuffled = __builtin_shufflevector(v1, v2, 0, 2, 1, 3);
    
    /* Vector compound literal with constant indices */
    v4si constructed = (v4si){v1[0], v1[2], v2[1], v2[3]};
    
    /* Vector section using shuffle */
    v4si section;
    if (g_volatile) {
        /* First two elements from v1 */
        section = __builtin_shufflevector(v1, v1, 0, 1, 0, 1);
    } else {
        /* Last two elements from v2 */
        section = __builtin_shufflevector(v2, v2, 2, 3, 2, 3);
    }
    
    /* Combine results */
    v4si result = shuffled + constructed + section;
    return result;
}

/* Scenario 4: Float vector with constant bounds */
__attribute__((noinline))
static float test_float_vector(void) {
    v4sf fv1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fv2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    float sum = 0.0f;
    
    /* Constant-bounded vector element accesses */
    if (g_volatile) {
        sum += fv1[0] + fv1[1];  /* count = 2 */
    } else {
        sum += fv1[2] + fv1[3];  /* count = 2 */
    }
    
    /* Vector shuffle with float */
    v4sf fshuffled = __builtin_shufflevector(fv1, fv2, 1, 3, 0, 2);
    sum += fshuffled[0] + fshuffled[2];
    
    return sum;
}

/* Scenario 5: Byte vector with many elements but small total size */
__attribute__((noinline))
static int test_byte_vector(void) {
    v16qi bv1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    v16qi bv2 = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    
    int sum = 0;
    
    /* Access multiple byte elements - count up to 16 */
    for (int i = 0; i < 8; i++) {  /* lo=0, hi=7, count=8 */
        sum += bv1[i];
    }
    
    /* Shuffle with constant indices */
    v16qi shuffled = __builtin_shufflevector(bv1, bv2, 
        0, 2, 4, 6, 8, 10, 12, 14,  /* Even indices from bv1 */
        1, 3, 5, 7, 9, 11, 13, 15); /* Odd indices from bv2 */
    
    /* Access shuffled result */
    for (int i = 0; i < 4; i++) {  /* lo=0, hi=3, count=4 */
        sum += shuffled[i * 2];
    }
    
    return sum;
}

/* Scenario 6: Multi-dimensional array with constant bounds */
__attribute__((noinline))
static int test_multi_dim_array(void) {
    int arr2d[10][20];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr2d[i][j] = i * 20 + j;
        }
    }
    
    /* Constant bounds in both dimensions */
    for (int i = 2; i < 6; i++) {        /* lo=2, hi=5, count=4 */
        for (int j = 5; j < 15; j++) {   /* lo=5, hi=14, count=10 */
            sum += arr2d[i][j];
        }
    }
    
    /* Different constant bounds based on volatile */
    int start = g_volatile ? 0 : 1;
    int end = g_volatile ? 3 : 4;
    
    for (int i = start; i < end; i++) {  /* Constant bounds after propagation */
        sum += arr2d[i][0];
    }
    
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
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    /* Test float vector */
    checksum += (int)test_float_vector();
    
    /* Test byte vector */
    checksum += test_byte_vector();
    
    /* Test multi-dimensional array */
    checksum += test_multi_dim_array();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
