/* Test program to cover constant bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing test code */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

/* ==================== SCENARIO 1: Small count (count <= 2) ==================== */
static int __attribute__((noinline))
test_small_count_memory_ref(void)
{
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Single element access - count = 1 */
    if (g_volatile) {
        sum += arr[5];      /* Lower bound = 5, upper bound = 5, count = 1 */
    } else {
        sum += arr[10];     /* Different constant index */
    }
    
    /* Two adjacent elements - count = 2 */
    if (g_volatile) {
        sum += arr[20] + arr[21];  /* lo=20, hi=21, count=2 */
    } else {
        sum += arr[30] + arr[31];  /* Alternative */
    }
    
    /* Using struct for 2-element access */
    struct two_ints { int a; int b; };
    struct two_ints s;
    s.a = arr[40];
    s.b = arr[41];          /* Still two constant-bounded accesses */
    sum += s.a + s.b;
    
    return sum;
}

/* ==================== SCENARIO 2: Larger constant-sized memory access ==================== */
static int __attribute__((noinline))
test_larger_constant_sized_access(void)
{
    int arr[100];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Loop with constant bounds: lo=2, hi=9, count=8 */
    /* TYPE_SIZE(int) = 32 bits, total = 32 * 8 = 256 bits (fits in uhwi) */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    
    /* Another constant slice with different bounds */
    /* lo=15, hi=34, count=20, total bits = 32 * 20 = 640 */
    for (int i = 15; i < 35; ++i) {
        sum -= arr[i];
    }
    
    /* Use char array for smaller element size */
    char carr[200];
    for (int i = 0; i < 200; i++) {
        carr[i] = (char)(i % 100);
    }
    
    /* char: TYPE_SIZE = 8 bits, count=50, total=400 bits */
    for (int i = 25; i < 75; ++i) {
        sum += carr[i];
    }
    
    return sum;
}

/* ==================== SCENARIO 3: Non-memory vector operations ==================== */
static v4si __attribute__((noinline))
test_non_memory_vector_ref(void)
{
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a direct memory reference */
    /* This creates a VEC_PERM_EXPR or similar in GCC's tree representation */
    v4si shuffled;
    if (g_volatile) {
        /* Constant indices: 0, 2, 1, 3 */
        shuffled = __builtin_shufflevector(v1, v2, 0, 2, 1, 3);
    } else {
        /* Alternative constant indices */
        shuffled = __builtin_shufflevector(v1, v2, 3, 2, 1, 0);
    }
    
    /* Vector compound literal with constant indices */
    v4si constructed;
    if (g_volatile) {
        /* Constant element selection */
        constructed = (v4si){v1[0], v1[1], v2[0], v2[1]};
    } else {
        constructed = (v4si){v1[3], v1[2], v2[3], v2[2]};
    }
    
    /* Vector blend/select with constant mask */
    v4si mask = {0, -1, 0, -1};  /* All constant */
    v4si blended = __builtin_shufflevector(v1, v2, 
        (mask[0] < 0) ? 4 : 0,
        (mask[1] < 0) ? 5 : 1,
        (mask[2] < 0) ? 6 : 2,
        (mask[3] < 0) ? 7 : 3);
    
    return shuffled + constructed + blended;
}

/* ==================== SCENARIO 4: Mixed array/vector with volatile bounds ==================== */
static int __attribute__((noinline))
test_mixed_volatile_bounds(void)
{
    int arr[100];
    float farr[50];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) arr[i] = i;
    for (int i = 0; i < 50; i++) farr[i] = i * 1.5f;
    
    /* Use volatile to prevent constant propagation but keep bounds constant */
    volatile int v = g_volatile;
    
    /* The ternary ensures compiler sees both bounds during analysis */
    int lo = v ? 5 : 10;
    int hi = v ? 15 : 20;
    
    /* But we'll force one path at runtime */
    if (!v) {
        /* This creates constant bounds lo=10, hi=20, count=11 */
        for (int i = 10; i < 21; ++i) {
            sum += arr[i];
        }
    }
    
    /* Vector version with float */
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    if (v) {
        /* Access single element - count = 1 */
        sum += (int)fvec[0];
    } else {
        /* Access two elements - count = 2 */
        sum += (int)(fvec[1] + fvec[2]);
    }
    
    return sum;
}

/* ==================== SCENARIO 5: Multi-dimensional array constant sections ==================== */
static int __attribute__((noinline))
test_multi_dim_constant_sections(void)
{
    int arr[10][20];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr[i][j] = i * 20 + j;
        }
    }
    
    /* Constant slice in 2D array */
    /* This should create MEM_REF with constant bounds for the inner dimension */
    for (int i = 2; i < 6; ++i) {
        /* Constant bounds on j: lo=3, hi=8, count=6 */
        for (int j = 3; j < 9; ++j) {
            sum += arr[i][j];
        }
    }
    
    /* Single row with constant bounds */
    if (g_volatile) {
        /* Access elements 5..14 of row 7 */
        for (int j = 5; j < 15; ++j) {
            sum -= arr[7][j];
        }
    }
    
    return sum;
}

/* ==================== MAIN FUNCTION ==================== */
int main(void)
{
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_memory_ref();
    checksum += test_larger_constant_sized_access();
    
    v4si vec_result = test_non_memory_vector_ref();
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_volatile_bounds();
    checksum += test_multi_dim_constant_sections();
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
