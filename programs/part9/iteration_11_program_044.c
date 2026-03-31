/* Test case for expr.cc constant bounds analysis coverage */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating test functions */
#define NOINLINE __attribute__((noinline))

/* Vector types for non-memory reference tests */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent constant propagation */
extern volatile int g_volatile_zero;
extern volatile int g_volatile_one;

/* Global arrays for memory reference tests */
static int g_int_array[100];
static float g_float_array[50];
static v4si g_vector_array[20];

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */

/* Single element load/store */
NOINLINE static void test_small_count_single(void)
{
    /* Force compiler to see constant bounds 5..5 (count=1) */
    int lo = g_volatile_zero ? 5 : 5;  /* Always 5, but compiler doesn't know */
    int hi = g_volatile_one ? 5 : 5;   /* Always 5, but compiler doesn't know */
    
    /* Memory reference with count=1 */
    g_int_array[lo] = 42;
    int val = g_int_array[hi];
    g_int_array[0] = val;  /* Use result */
}

/* Two adjacent elements */
NOINLINE static void test_small_count_pair(void)
{
    /* Constant bounds 3..4 (count=2) */
    int start = g_volatile_zero ? 3 : 3;
    
    /* Two-element memory reference */
    g_int_array[start] = 1;
    g_int_array[start + 1] = 2;
    
    /* Small struct-like access */
    typedef struct { int a; int b; } pair_t;
    pair_t *p = (pair_t *)&g_int_array[10];
    p->a = 10;
    p->b = 20;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Fixed-size array slice in loop */
NOINLINE static void test_larger_constant_slice(void)
{
    /* Constant bounds 2..9 (count=8) */
    int lo = g_volatile_zero ? 2 : 2;
    int hi = g_volatile_one ? 9 : 9;
    
    /* Loop with constant bounds - total size = 8 * sizeof(int) * 8 bits */
    for (int i = lo; i <= hi; ++i) {
        g_int_array[i] = i * 2;
    }
}

/* Byte array with many elements but small total size */
NOINLINE static void test_byte_array_slice(void)
{
    char byte_array[200];
    
    /* Constant bounds 10..49 (count=40) */
    /* Total bits = 40 * 8 = 320, fits in unsigned HWI */
    int start = g_volatile_zero ? 10 : 10;
    int end = g_volatile_one ? 49 : 49;
    
    for (int i = start; i <= end; ++i) {
        byte_array[i] = (char)(i % 256);
    }
    
    /* Use result to prevent elimination */
    g_int_array[0] = byte_array[start] + byte_array[end];
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */

/* Vector shuffle with constant indices */
NOINLINE static v4si test_vector_shuffle(void)
{
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant bounds in shuffle: indices 0,2,1,3 */
    /* This creates a VEC_PERM_EXPR, not a memory reference */
    v4si result = __builtin_shufflevector(a, b, 0, 2, 1, 3);
    
    return result;
}

/* Vector compound literal with constant indices */
NOINLINE static v4si test_vector_constructor(void)
{
    v4si v = {10, 20, 30, 40};
    
    /* Constant-bounded section: elements 1..2 (count=2) */
    /* Creates a CONSTRUCTOR node, not MEM_REF */
    v4si slice = (v4si){v[1], v[2], 0, 0};
    
    return slice;
}

/* Vector extraction with constant indices */
NOINLINE static void test_vector_extract(void)
{
    v4si v = g_vector_array[0];
    
    /* Multiple constant-index extractions */
    int a = v[0];  /* Constant index 0 */
    int b = v[2];  /* Constant index 2 */
    int c = v[3];  /* Constant index 3 */
    
    g_int_array[0] = a + b + c;
}

/* ========== SCENARIO 4: Mixed cases with conditional bounds ========== */

/* Conditional constant bounds to exercise both paths */
NOINLINE static void test_conditional_bounds(void)
{
    int flag = g_volatile_zero;
    
    if (flag) {
        /* Path A: small count */
        for (int i = 0; i < 2; ++i) {  /* count=2 */
            g_float_array[i] = i * 1.5f;
        }
    } else {
        /* Path B: larger count but still constant */
        for (int i = 2; i < 10; ++i) {  /* count=8 */
            g_float_array[i] = i * 2.5f;
        }
    }
}

/* Array section with compile-time constant expression bounds */
NOINLINE static void test_constant_expression_bounds(void)
{
    /* Complex but compile-time constant bounds */
    int lo = (sizeof(int) > 2) ? 5 : 10;  /* Compile-time constant */
    int hi = lo + 7;  /* Also compile-time constant: 5+7=12 or 10+7=17 */
    
    /* count = hi - lo + 1 = 8 (constant) */
    for (int i = lo; i <= hi; ++i) {
        g_int_array[i] = i * 3;
    }
}

/* ========== MAIN FUNCTION ========== */

int main(void)
{
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        g_int_array[i] = i;
    }
    for (int i = 0; i < 50; ++i) {
        g_float_array[i] = i * 0.5f;
    }
    for (int i = 0; i < 20; ++i) {
        g_vector_array[i] = (v4si){i*4, i*4+1, i*4+2, i*4+3};
    }
    
    /* Execute all test scenarios */
    test_small_count_single();
    test_small_count_pair();
    test_larger_constant_slice();
    test_byte_array_slice();
    
    v4si v1 = test_vector_shuffle();
    v4si v2 = test_vector_constructor();
    test_vector_extract();
    
    test_conditional_bounds();
    test_constant_expression_bounds();
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 20; ++i) {
        checksum += g_int_array[i];
        checksum += (int)g_float_array[i % 50];
        checksum += g_vector_array[i % 20][0];
    }
    
    checksum += v1[0] + v1[1] + v1[2] + v1[3];
    checksum += v2[0] + v2[1] + v2[2] + v2[3];
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Define volatile variables */
volatile int g_volatile_zero = 0;
volatile int g_volatile_one = 1;
