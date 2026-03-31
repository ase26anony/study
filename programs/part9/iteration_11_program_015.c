/* Test program to cover constant bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile_zero = 0;
static volatile int g_volatile_one = 1;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4f __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* ========== Scenario 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void)
{
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Single element access - count = 1 */
    if (g_volatile_zero) {
        /* This path won't execute but compiler sees constant bounds */
        sum += arr[5];  /* lo=5, hi=5, count=1 */
    } else {
        sum += arr[10]; /* lo=10, hi=10, count=1 */
    }
    
    /* Two adjacent elements - count = 2 */
    /* Use volatile to prevent constant propagation */
    int idx = g_volatile_zero ? 20 : 30;
    sum += arr[idx] + arr[idx + 1];  /* lo=idx, hi=idx+1, count=2 */
    
    /* Two-element struct-like access */
    struct pair { int a; int b; } p;
    p.a = arr[40];
    p.b = arr[41];  /* Two-element memory reference */
    sum += p.a + p.b;
    
    return sum;
}

/* ========== Scenario 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_bounds(void)
{
    int arr[200];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 200; i++) {
        arr[i] = i * 3;
    }
    
    /* Constant bounds loop accessing 8 elements */
    /* lo=2, hi=9, count=8, element_size=32 bits, total=256 bits */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    
    /* Different constant bounds with char type */
    /* Smaller element size, larger count still fits in HWI */
    char carr[1000];
    for (int i = 0; i < 1000; i++) {
        carr[i] = (char)(i % 256);
    }
    
    /* Access 100 chars: lo=100, hi=199, count=100, element_size=8 bits, total=800 bits */
    char csum = 0;
    for (int i = 100; i < 200; ++i) {
        csum += carr[i];
    }
    sum += csum;
    
    /* Mixed with volatile condition to preserve analysis */
    int start = g_volatile_one ? 50 : 60;
    int end = start + 20;  /* Still constant: 20 elements */
    for (int i = start; i < end; ++i) {
        sum += arr[i] * 2;
    }
    
    return sum;
}

/* ========== Scenario 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void)
{
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a memory reference */
    v4si shuffled = __builtin_shufflevector(v1, v2, 0, 2, 1, 3);
    
    /* Vector compound literal with constant indices */
    v4si constructed = (v4si){v1[0], v1[2], v2[1], v2[3]};
    
    /* Vector permutation with constant mask */
    v4si perm = __builtin_shufflevector(v1, v2, 3, 2, 1, 0);
    
    /* Return combination to prevent elimination */
    return shuffled + constructed + perm;
}

/* ========== Scenario 4: Mixed array/vector with constant bounds ========== */
__attribute__((noinline))
static int test_mixed_constant_access(void)
{
    /* Test with float array */
    float farr[64];
    for (int i = 0; i < 64; i++) {
        farr[i] = i * 1.5f;
    }
    
    float fsum = 0.0f;
    /* Constant bounds: 16 elements */
    for (int i = 8; i < 24; ++i) {
        fsum += farr[i];
    }
    
    /* Test with vector array */
    v2si vec_arr[32];
    for (int i = 0; i < 32; i++) {
        vec_arr[i] = (v2si){i, i*2};
    }
    
    /* Access two adjacent vectors - each is 2 ints */
    v2si vsum = vec_arr[5] + vec_arr[6];
    
    /* Convert results to int for checksum */
    return (int)fsum + vsum[0] + vsum[1];
}

/* ========== Scenario 5: Edge cases with different types ========== */
__attribute__((noinline))
static int test_edge_cases(void)
{
    /* Very small element type with many elements */
    unsigned char byte_arr[512];
    for (int i = 0; i < 512; i++) {
        byte_arr[i] = (unsigned char)(i % 256);
    }
    
    unsigned char byte_sum = 0;
    /* 256 elements * 8 bits = 2048 bits, should fit in HWI */
    for (int i = 128; i < 384; ++i) {
        byte_sum += byte_arr[i];
    }
    
    /* Double type with small count */
    double darr[32];
    for (int i = 0; i < 32; i++) {
        darr[i] = i * 0.5;
    }
    
    double dsum = 0.0;
    /* 2 elements of double */
    dsum += darr[10] + darr[11];
    
    /* Use volatile to create conditional constant bounds */
    int choice = g_volatile_zero;
    int start_idx = choice ? 5 : 15;
    int count = choice ? 3 : 4;  /* Still constant in each path */
    
    int int_arr[50];
    for (int i = 0; i < 50; i++) int_arr[i] = i;
    
    int partial_sum = 0;
    for (int i = start_idx; i < start_idx + count; ++i) {
        partial_sum += int_arr[i];
    }
    
    return byte_sum + (int)dsum + partial_sum;
}

/* ========== Main function ========== */
int main(void)
{
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_bounds();
    
    v4si vec_result = test_non_memory_vector();
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    checksum += test_mixed_constant_access();
    checksum += test_edge_cases();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
