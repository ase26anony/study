/* Test program to cover constant bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* Array declarations */
static int arr_int[100];
static float arr_float[50];
static char arr_char[200];

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void)
{
    int sum = 0;
    
    /* Single element access - count = 1 */
    arr_int[5] = 42;          /* Store single element */
    sum += arr_int[5];        /* Load single element */
    
    /* Two adjacent elements - count = 2 */
    arr_int[10] = 1;
    arr_int[11] = 2;
    sum += arr_int[10] + arr_int[11];
    
    /* Using struct for 2-element access */
    struct two_ints { int a; int b; } s;
    s.a = arr_int[20];
    s.b = arr_int[21];
    sum += s.a + s.b;
    
    /* Vector with 2 elements */
    v2si v2;
    v2[0] = arr_int[30];
    v2[1] = arr_int[31];
    sum += v2[0] + v2[1];
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_sized(void)
{
    int sum = 0;
    
    /* Fixed-size array slice in loop - 8 elements of int */
    for (int i = 2; i < 10; ++i) {    /* count = 8, size = 8 * 32 = 256 bits */
        arr_int[i] = i * 2;
        sum += arr_int[i];
    }
    
    /* Larger slice with char type - 32 elements */
    for (int i = 0; i < 32; ++i) {    /* count = 32, size = 32 * 8 = 256 bits */
        arr_char[i] = i;
        sum += arr_char[i];
    }
    
    /* Mixed bounds with constant expressions */
    int lo = 15;
    int hi = 30;
    /* Use volatile to prevent constant propagation of bounds */
    if (g_volatile) {
        lo = 10;
        hi = 20;
    }
    
    /* Constant bounds determined by ternary with volatile guard */
    int start = g_volatile ? 5 : 25;      /* Will be 25 at runtime */
    int end = g_volatile ? 10 : 35;       /* Will be 35 at runtime */
    
    for (int i = start; i < end; ++i) {   /* count = 10, size = 10 * 32 = 320 bits */
        arr_int[i] = i * 3;
        sum += arr_int[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory reference cases (!MEM_P) ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void)
{
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - creates CONSTRUCTOR/VEC_PERM_EXPR */
    v4si shuffled = __builtin_shufflevector(v1, v2, 0, 2, 1, 3);
    
    /* Vector compound literal with constant indices */
    v4si sliced = (v4si){v1[0], v1[1], v2[0], v2[1]};
    
    /* Vector operations that yield non-memory results */
    v4si result = shuffled + sliced;
    
    /* Constant-bounded vector section from larger vector */
    v4si partial = (v4si){result[0], result[1], result[2], 0};
    
    return partial;
}

/* ========== SCENARIO 4: Mixed types and sizes ========== */
__attribute__((noinline))
static float test_mixed_types(void)
{
    float sum = 0.0f;
    
    /* Float array with constant bounds */
    for (int i = 0; i < 16; ++i) {    /* count = 16, size = 16 * 32 = 512 bits */
        arr_float[i] = i * 1.5f;
        sum += arr_float[i];
    }
    
    /* Nested constant bounds */
    for (int i = 2 + (g_volatile ? 0 : 3); i < 10 - (g_volatile ? 1 : 0); ++i) {
        arr_float[i] *= 2.0f;
        sum += arr_float[i];
    }
    
    return sum;
}

/* ========== SCENARIO 5: Edge cases and boundary conditions ========== */
__attribute__((noinline))
static int test_edge_cases(void)
{
    int sum = 0;
    
    /* Exactly 2 elements - boundary case */
    arr_int[50] = 100;
    arr_int[51] = 200;
    sum += arr_int[50] + arr_int[51];
    
    /* Single element with constant index expression */
    int idx = 5 + 3;  /* Constant expression */
    arr_int[idx] = 42;
    sum += arr_int[idx];
    
    /* Constant bounds with arithmetic */
    for (int i = 1*2; i < 3*4; ++i) {  /* i from 2 to 11, count = 10 */
        arr_char[i] = i % 256;
        sum += arr_char[i];
    }
    
    return sum;
}

/* ========== SCENARIO 6: Vector extensions with constant indices ========== */
__attribute__((noinline))
static int test_vector_extensions(void)
{
    v4si v = {1, 2, 3, 4};
    int sum = 0;
    
    /* Direct vector element access with constant indices */
    sum += v[0] + v[1] + v[2] + v[3];
    
    /* Vector slice using GNU extension */
    int v_low[2] = {v[0], v[1]};
    int v_high[2] = {v[2], v[3]};
    sum += v_low[0] + v_high[1];
    
    /* Create new vector from constant indices */
    v4si v_new = {v[3], v[2], v[1], v[0]};
    sum += v_new[0] + v_new[3];
    
    return sum;
}

/* ========== Main function ========== */
int main(void)
{
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr_int[i] = i;
    }
    for (int i = 0; i < 50; ++i) {
        arr_float[i] = i * 0.5f;
    }
    for (int i = 0; i < 200; ++i) {
        arr_char[i] = i % 128;
    }
    
    /* Execute all test scenarios */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_sized();
    
    v4si vec_result = test_non_memory_vector();
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    checksum += (int)test_mixed_types();
    checksum += test_edge_cases();
    checksum += test_vector_extensions();
    
    /* Use volatile to prevent dead code elimination */
    if (g_volatile) {
        checksum = 0;  /* This path never taken at runtime */
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
