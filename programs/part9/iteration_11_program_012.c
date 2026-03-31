/* Test program to cover constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile = 0;

/* Vector types for non-MEM_P scenarios */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Small struct for count <= 2 scenario */
struct two_elem {
    int a;
    int b;
};

/* ========== Scenario 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static void test_small_count_mem(void) {
    int arr[100];
    struct two_elem sarr[50];
    
    /* Single element access - count = 1 */
    arr[5] = 42;
    int x = arr[5];  /* Load single element */
    
    /* Two adjacent elements - count = 2 */
    sarr[3].a = 1;
    sarr[3].b = 2;
    int y = sarr[3].a + sarr[3].b;
    
    /* Vector with 2 elements */
    typedef int v2si __attribute__((vector_size(8)));
    v2si v2;
    int* pv2 = (int*)&v2;
    pv2[0] = 10;  /* First element */
    pv2[1] = 20;  /* Second element */
    
    /* Use volatile to prevent elimination */
    if (g_volatile) printf("%d %d\n", x, y);
}

/* ========== Scenario 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_mem(void) {
    int arr[100];
    char carr[200];
    int sum = 0;
    
    /* Access 8 elements (count > 2) - total size = 8 * 32 = 256 bits */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        arr[i] = i * 2;
    }
    
    /* Force analysis with volatile condition */
    int limit = g_volatile ? 5 : 20;
    for (int i = 0; i < limit; ++i) {
        if (i >= 2 && i < 10) {
            sum += arr[i];
        }
    }
    
    /* Char array with larger count but smaller element size */
    /* 32 chars * 8 bits = 256 bits */
    for (int i = 10; i < 42; ++i) {  /* lo=10, hi=41, count=32 */
        carr[i] = i % 256;
    }
    
    /* Mixed constant bounds in same function */
    for (int i = 30; i < 50; ++i) {  /* lo=30, hi=49, count=20 */
        arr[i] = arr[i - 20] + carr[i - 20];
    }
    
    return sum;
}

/* ========== Scenario 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_mem_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a MEM_P */
    v4si v3 = __builtin_shufflevector(v1, v2, 0, 1, 4, 5);
    
    /* Vector compound literal with constant indices */
    v4si v4 = (v4si){v1[0], v1[1], v2[0], v2[1]};
    
    /* Vector permute with constant mask */
    typedef int v4si_mask __attribute__((vector_size(16)));
    v4si_mask mask = {0, 2, 4, 6};
    v4si v5 = __builtin_shufflevector(v1, v2, 0, 2, 4, 6);
    
    /* Return combination to prevent elimination */
    return v3 + v4 + v5;
}

/* ========== Scenario 4: Mixed bounds with volatile conditions ========== */
__attribute__((noinline))
static int test_mixed_bounds(void) {
    float farr[64];
    double darr[32];
    int result = 0;
    
    /* Volatile determines which constant bounds are used */
    int start = g_volatile ? 5 : 10;
    int end = g_volatile ? 15 : 25;
    
    /* Compiler sees both possibilities during analysis */
    for (int i = start; i < end; ++i) {
        farr[i] = i * 1.5f;
    }
    
    /* Nested loops with constant bounds */
    for (int i = 2; i < 8; ++i) {
        for (int j = 3; j < 7; ++j) {
            darr[i * 4 + j] = farr[i] + j;
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < 32; ++i) {
        result += (int)darr[i];
    }
    
    return result;
}

/* ========== Scenario 5: Array sections with constant expressions ========== */
__attribute__((noinline))
static int test_constant_expressions(void) {
    int arr[100];
    int sum = 0;
    
    /* Constant expressions as bounds */
    arr[1 + 3] = 10;          /* lo = hi = 4, count = 1 */
    arr[2 * 3] = 20;          /* lo = hi = 6, count = 1 */
    arr[10 - 2] = 30;         /* lo = hi = 8, count = 1 */
    
    /* Range with constant expressions */
    for (int i = 1 + 2; i < 4 * 3; ++i) {  /* lo=3, hi=11, count=9 */
        arr[i] = i * i;
    }
    
    /* Multiple of element size calculation */
    for (int i = 0; i < 16; i += 4) {  /* Each iteration accesses 4 elements */
        sum += arr[i] + arr[i+1] + arr[i+2] + arr[i+3];
    }
    
    return sum;
}

/* ========== Main function ========== */
int main(void) {
    int checksum = 0;
    
    /* Call all test scenarios */
    test_small_count_mem();
    checksum += test_larger_constant_mem();
    
    v4si vec_result = test_non_mem_vector();
    for (int i = 0; i < 4; ++i) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_bounds();
    checksum += test_constant_expressions();
    
    /* Print result to prevent elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
