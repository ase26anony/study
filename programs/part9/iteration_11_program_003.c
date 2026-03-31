/* Test case for GCC expr.cc constant bounds analysis coverage */
/* Compile with: gcc -O2 -fdump-tree-ccp1 -fprofile-arcs -ftest-coverage -o test_expr test_expr.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile_zero = 0;

/* Vector types for non-MEM_P cases */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small count (count <= 2) memory access ========== */
static __attribute__((noinline)) 
void test_small_count_mem(void) {
    int arr[100];
    v4si vec;
    
    /* Initialize to prevent undefined behavior */
    for (int i = 0; i < 100; i++) arr[i] = i;
    vec = (v4si){1, 2, 3, 4};
    
    /* Single element access (count = 1) */
    int x = arr[5 + g_volatile_zero];  /* volatile ensures analysis sees constant 5 */
    
    /* Two adjacent elements via small struct-like access */
    struct { int a; int b; } two_ints;
    two_ints.a = arr[10];
    two_ints.b = arr[11];
    
    /* Vector element access - two elements */
    int first = vec[0];
    int second = vec[1];
    
    /* Prevent dead code elimination */
    printf("%d %d %d %d\n", x, two_ints.a, first, second);
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
static __attribute__((noinline))
void test_larger_constant_mem(void) {
    char char_arr[200];
    int int_arr[50];
    
    /* Initialize */
    for (int i = 0; i < 200; i++) char_arr[i] = i;
    for (int i = 0; i < 50; i++) int_arr[i] = i * 2;
    
    /* Constant bounds loop with char type */
    /* TYPE_SIZE(char) = 8 bits, count = 8, total bits = 64 (fits in uhwi) */
    for (int i = 2; i < 10; ++i) {
        char_arr[i] = char_arr[i] + 1;
    }
    
    /* Constant bounds loop with int type */
    /* TYPE_SIZE(int) = 32 bits, count = 4, total bits = 128 (fits in uhwi) */
    for (int i = 5; i < 9; ++i) {
        int_arr[i] = int_arr[i] * 2;
    }
    
    /* Mixed bounds using volatile to prevent constant folding */
    int start = 20 + g_volatile_zero;
    int end = 30 + g_volatile_zero;
    /* Compiler sees start=20, end=30 during analysis due to volatile zero */
    for (int i = start; i < end; ++i) {
        int_arr[i] = int_arr[i] - 1;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 50; i++) sum += int_arr[i];
    printf("Sum: %d\n", sum);
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
static __attribute__((noinline))
v4si test_non_mem_vector(void) {
    v4si a = (v4si){1, 2, 3, 4};
    v4si b = (v4si){5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    v4si shuffled = __builtin_shufflevector(a, b, 0, 1, 4, 5);
    
    /* Vector compound literal with constant indices */
    v4si sliced = (v4si){a[0], a[1], b[2], b[3]};
    
    /* Vector constructor with constant elements */
    v4si constructed;
    for (int i = 0; i < 4; i++) {
        constructed[i] = (i < 2) ? a[i] : b[i];
    }
    
    /* Return mixed result */
    return shuffled + sliced + constructed;
}

/* ========== SCENARIO 4: Mixed array section with constant bounds ========== */
static __attribute__((noinline))
void test_mixed_array_sections(void) {
    float farr[64];
    double darr[32];
    
    /* Initialize */
    for (int i = 0; i < 64; i++) farr[i] = i * 1.5f;
    for (int i = 0; i < 32; i++) darr[i] = i * 2.5;
    
    /* Multiple constant-bounded sections in same function */
    
    /* Section 1: count = 3, float type (32 bits * 3 = 96 bits) */
    float sum1 = 0;
    for (int i = 3; i < 6; i++) {
        sum1 += farr[i];
    }
    
    /* Section 2: count = 5, double type (64 bits * 5 = 320 bits) */
    double sum2 = 0;
    for (int i = 10; i < 15; i++) {
        sum2 += darr[i];
    }
    
    /* Section 3: Using compile-time constant expression */
    int base = 20;
    for (int i = base; i < base + 4; i++) {
        farr[i] = farr[i] * 2.0f;
    }
    
    printf("Sums: %f %f\n", sum1, sum2);
}

/* ========== SCENARIO 5: Vector with constant bounds ========== */
static __attribute__((noinline))
void test_vector_constant_bounds(void) {
    v8hi v1 = (v8hi){0, 1, 2, 3, 4, 5, 6, 7};
    v4sf v2 = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Access constant range of vector elements */
    short s1 = v1[2];
    short s2 = v1[3];
    float f1 = v2[0];
    float f2 = v2[1];
    
    /* Create new vector from constant indices */
    v8hi v3 = __builtin_shufflevector(v1, v1, 0, 1, 2, 3, 0, 1, 2, 3);
    
    printf("Vector: %d %d %f %f\n", s1, s2, f1, f2);
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int checksum = 0;
    
    /* Call all test functions */
    test_small_count_mem();
    
    test_larger_constant_mem();
    
    v4si vec_result = test_non_mem_vector();
    for (int i = 0; i < 4; i++) checksum += vec_result[i];
    
    test_mixed_array_sections();
    
    test_vector_constant_bounds();
    
    /* Final checksum to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
