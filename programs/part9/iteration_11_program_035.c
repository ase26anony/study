/* Test program to cover constant-bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void)
{
    int arr[100] = {0};
    int sum = 0;
    
    /* Single element access - count = 1 */
    if (g_volatile) {
        arr[5] = 42;           /* Constant lower and upper bound = 5 */
    } else {
        arr[3 + 2] = 43;       /* Constant expression bounds */
    }
    sum += arr[5];
    
    /* Two adjacent elements - count = 2 */
    typedef struct { int a; int b; } two_ints;
    two_ints pair;
    pair.a = arr[10];          /* Access element 10 */
    pair.b = arr[11];          /* Access element 11 - total count = 2 */
    sum += pair.a + pair.b;
    
    /* Vector with 2 elements */
    v2si vec2 = {1, 2};
    int* pvec = (int*)&vec2;
    sum += pvec[0] + pvec[1];  /* Access both elements */
    
    /* Array section with exactly 2 elements */
    for (int i = 20; i < 22; ++i) {  /* lo=20, hi=21, count=2 */
        arr[i] = i * 2;
        sum += arr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_sized(void)
{
    int arr[100] = {0};
    float farr[50] = {0.0f};
    char carr[200] = {0};
    int sum = 0;
    
    /* Integer array: 8 elements * 32 bits = 256 bits (fits in uhwi) */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        arr[i] = i * 3;
        sum += arr[i];
    }
    
    /* Float array: 16 elements * 32 bits = 512 bits */
    for (int i = 0; i < 16; ++i) {  /* lo=0, hi=15, count=16 */
        farr[i] = i * 1.5f;
        sum += (int)farr[i];
    }
    
    /* Char array: 100 elements * 8 bits = 800 bits */
    for (int i = 50; i < 150; ++i) {  /* lo=50, hi=149, count=100 */
        carr[i] = i % 128;
        sum += carr[i];
    }
    
    /* Mixed with volatile to prevent constant propagation */
    int start = g_volatile ? 5 : 10;
    for (int i = start; i < start + 20; ++i) {  /* Still constant bounds after analysis */
        arr[i] = i * 2;
        sum += arr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
__attribute__((noinline))
static int test_non_memory_vector(void)
{
    v4si vec = {1, 2, 3, 4};
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    int sum = 0;
    
    /* Vector constructor with constant indices - not a memory reference */
    v4si vec_slice = (v4si){vec[0], vec[1], vec[2], vec[3]};  /* All constant indices */
    sum += vec_slice[0] + vec_slice[1];
    
    /* Vector shuffle with constant indices */
    v4si shuffled = __builtin_shufflevector(vec, vec, 3, 2, 1, 0);  /* Constant indices */
    sum += shuffled[0] + shuffled[3];
    
    /* Vector compound literal with computed but constant indices */
    int idx1 = 1 + 0;  /* Constant expression */
    int idx2 = 2 * 1;  /* Constant expression */
    v4si mixed = (v4si){vec[idx1], vec[idx2], vec[3], vec[0]};
    sum += mixed[0] + mixed[1];
    
    /* Float vector operations */
    v4sf fslice = (v4sf){fvec[0], fvec[1], fvec[2], fvec[3]};
    sum += (int)(fslice[0] + fslice[1]);
    
    /* Vector extraction with constant indices */
    typedef int extract_type __attribute__((ext_vector_type(4)));
    extract_type ev = {10, 20, 30, 40};
    sum += ev.x + ev.z;  /* Constant component access */
    
    return sum;
}

/* ========== SCENARIO 4: Complex constant bounds with conditions ========== */
__attribute__((noinline))
static int test_complex_bounds(void)
{
    int arr[200] = {0};
    int sum = 0;
    
    /* Use volatile to create conditional constant bounds */
    volatile int cond = g_volatile;
    
    /* Both branches have constant bounds, just different values */
    if (cond) {
        /* Access 4 elements */
        for (int i = 0; i < 4; ++i) {  /* lo=0, hi=3, count=4 */
            arr[i] = i * 10;
            sum += arr[i];
        }
    } else {
        /* Access 6 elements */
        for (int i = 5; i < 11; ++i) {  /* lo=5, hi=10, count=6 */
            arr[i] = i * 5;
            sum += arr[i];
        }
    }
    
    /* Nested loops with constant bounds */
    for (int i = 1; i < 5; ++i) {      /* Outer: lo=1, hi=4, count=4 */
        for (int j = 10; j < 15; ++j) { /* Inner: lo=10, hi=14, count=5 */
            arr[i * 20 + j] = i + j;
            sum += arr[i * 20 + j];
        }
    }
    
    /* Multi-dimensional array with constant bounds */
    int matrix[10][10];
    for (int i = 2; i < 8; ++i) {      /* lo=2, hi=7, count=6 */
        for (int j = 3; j < 9; ++j) {  /* lo=3, hi=8, count=6 */
            matrix[i][j] = i * j;
            sum += matrix[i][j];
        }
    }
    
    return sum;
}

/* ========== SCENARIO 5: Edge cases and mixed types ========== */
__attribute__((noinline))
static int test_edge_cases(void)
{
    /* Test with smallest possible count (1) */
    {
        int arr[10] = {0};
        arr[5] = 100;  /* Single element, count=1 */
        int sum = arr[5];
        
        /* Exactly 2 elements using different types */
        short sarr[20];
        sarr[10] = 50;  /* First element */
        sarr[11] = 60;  /* Second element, count=2 */
        sum += sarr[10] + sarr[11];
        
        return sum;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void)
{
    int checksum = 0;
    
    /* Call all test functions */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_sized();
    checksum += test_non_memory_vector();
    checksum += test_complex_bounds();
    checksum += test_edge_cases();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
