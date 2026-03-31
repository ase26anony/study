/* Test program to cover constant-bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== Scenario 1: Small count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void)
{
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* Single element access - count = 1 */
    if (g_volatile) {
        sum += arr[5];      /* Constant lower bound = 5, hi = 5, count = 1 */
    } else {
        sum += arr[10];     /* Alternative constant bound */
    }
    
    /* Two adjacent elements - count = 2 */
    if (g_volatile) {
        sum += arr[20] + arr[21];  /* lo = 20, hi = 21, count = 2 */
    } else {
        sum += arr[30] + arr[31];  /* Alternative */
    }
    
    /* Using vector type with 2 elements */
    v2si vec2;
    int *p = (int*)&vec2;
    p[0] = 100;  /* Constant index 0 */
    p[1] = 200;  /* Constant index 1 */
    sum += p[0] + p[1];
    
    return sum;
}

/* ========== Scenario 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_section(void)
{
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Access 8 elements - count = 8, size = 8 * sizeof(int) * 8 bits */
    /* This should trigger: TYPE_SIZE(elttype) * count fits in uhwi */
    for (int i = 2; i < 10; ++i) {  /* lo = 2, hi = 9, count = 8 */
        sum += arr[i];
    }
    
    /* Different constant bounds */
    if (g_volatile) {
        for (int i = 15; i < 30; ++i) {  /* lo = 15, hi = 29, count = 15 */
            sum += arr[i];
        }
    } else {
        for (int i = 40; i < 60; ++i) {  /* lo = 40, hi = 59, count = 20 */
            sum += arr[i];
        }
    }
    
    /* Test with char type - more elements but smaller total size */
    char carr[200];
    for (int i = 0; i < 200; i++) {
        carr[i] = (char)(i % 100);
    }
    
    for (int i = 10; i < 50; ++i) {  /* lo = 10, hi = 49, count = 40 */
        sum += carr[i];  /* 40 * 8 bits = 320 bits, fits in uhwi */
    }
    
    return sum;
}

/* ========== Scenario 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void)
{
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a direct memory reference */
    v4si result;
    
    if (g_volatile) {
        /* Using compound literal to create vector section */
        result = (v4si){v1[0], v1[1], v2[2], v2[3]};  /* Constant indices */
    } else {
        /* Alternative constant indices */
        result = (v4si){v1[3], v1[2], v2[1], v2[0]};  /* Constant indices */
    }
    
    /* Vector permutation with constant mask */
    typedef int v8si __attribute__((vector_size(32)));
    v8si v8 = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Extract first 4 elements with constant indices */
    v4si first_half = __builtin_shufflevector(v8, v8, 0, 1, 2, 3);
    result += first_half;
    
    return result;
}

/* ========== Scenario 4: Mixed types and bounds ========== */
__attribute__((noinline))
static float test_mixed_types(void)
{
    float farr[50];
    double darr[30];
    float sum = 0.0f;
    
    /* Initialize arrays */
    for (int i = 0; i < 50; i++) {
        farr[i] = i * 0.5f;
    }
    for (int i = 0; i < 30; i++) {
        darr[i] = i * 0.25;
    }
    
    /* Float array with constant bounds */
    for (int i = 3; i < 12; ++i) {  /* lo = 3, hi = 11, count = 9 */
        sum += farr[i];
    }
    
    /* Double array with constant bounds */
    if (g_volatile) {
        for (int i = 5; i < 15; ++i) {  /* lo = 5, hi = 14, count = 10 */
            sum += (float)darr[i];
        }
    }
    
    /* Vector float operations */
    v4sf vf1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vf2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Constant-indexed vector element access */
    sum += vf1[0] + vf1[1] + vf2[2] + vf2[3];
    
    return sum;
}

/* ========== Scenario 5: Struct with array member ========== */
__attribute__((noinline))
static int test_struct_array(void)
{
    struct S {
        int header;
        int data[20];
        int footer;
    };
    
    struct S s;
    int sum = 0;
    
    /* Initialize struct */
    s.header = 100;
    s.footer = 200;
    for (int i = 0; i < 20; i++) {
        s.data[i] = i * 3;
    }
    
    /* Constant bounds within struct array member */
    for (int i = 2; i < 8; ++i) {  /* lo = 2, hi = 7, count = 6 */
        sum += s.data[i];
    }
    
    /* Single element access */
    if (g_volatile) {
        sum += s.data[10];  /* count = 1 */
    } else {
        sum += s.data[15];  /* count = 1 */
    }
    
    return sum;
}

/* ========== Main function ========== */
int main(void)
{
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_memory();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    checksum += test_larger_constant_section();
    
    float float_sum = test_mixed_types();
    checksum += (int)float_sum;
    
    checksum += test_struct_array();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
