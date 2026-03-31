/* Test case for expr.cc constant bounds analysis coverage.
   Specifically targeting lines 7691-7700 in expr.cc.
   
   Compile with: gcc -O2 -fprofile-arcs -ftest-coverage -fdump-tree-ccp1 -fdump-tree-forwprop1 -o test_expr test_expr.c
   Run with: ./test_expr
   Check coverage with: gcov -b expr.cc
*/

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile_zero = 0;

/* Vector types for non-MEM_P cases */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small count (count <= 2) memory access ========== */
__attribute__((noinline))
static int test_small_count_mem(void) {
    int arr[100] = {0};
    int sum = 0;
    
    /* Single element access - count = 1 */
    if (g_volatile_zero == 0) {
        arr[5] = 42;  /* lo=5, hi=5, count=1 */
        sum += arr[5];
    }
    
    /* Two adjacent elements - count = 2 */
    if (g_volatile_zero == 0) {
        arr[10] = 1;   /* lo=10, hi=11, count=2 */
        arr[11] = 2;
        sum += arr[10] + arr[11];
    }
    
    /* Using vector type with 2-element access */
    typedef int v2si __attribute__((vector_size(8)));
    v2si v2;
    int *pv2 = (int*)&v2;
    
    pv2[0] = 10;  /* lo=0, hi=1, count=2 */
    pv2[1] = 20;
    sum += pv2[0] + pv2[1];
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_mem(void) {
    int arr[100] = {0};
    int sum = 0;
    
    /* Access 8 elements: TYPE_SIZE(int)=32, count=8, total=256 bits */
    /* This fits in unsigned HWI (assuming 64-bit host) */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        arr[i] = i * 3;
    }
    
    /* Force compiler to see the constant bounds */
    if (g_volatile_zero == 0) {
        for (int i = 2; i < 10; ++i) {
            sum += arr[i];
        }
    }
    
    /* Another case with char type: 20 chars = 160 bits */
    char buf[50];
    for (int i = 5; i < 25; ++i) {  /* lo=5, hi=24, count=20 */
        buf[i] = (char)(i % 256);
    }
    
    if (g_volatile_zero == 0) {
        for (int i = 5; i < 25; ++i) {
            sum += buf[i];
        }
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_mem_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a memory load/store */
    /* Creates VEC_PERM_EXPR or CONSTRUCTOR nodes */
    v4si result;
    
    if (g_volatile_zero == 0) {
        /* Shuffle selecting first two elements from v1, last two from v2 */
        result = __builtin_shufflevector(v1, v2, 0, 1, 4, 5);
        /* Constant bounds: lo=0, hi=3, count=4, but not MEM_P */
    } else {
        /* Compound literal with constant indices */
        result = (v4si){v1[0], v1[1], v2[0], v2[1]};
    }
    
    /* Another non-memory case: vector extraction */
    typedef int v2si __attribute__((vector_size(8)));
    v2si low_half = __builtin_shufflevector(v1, v1, 0, 1);  /* lo=0, hi=1, count=2 */
    
    /* Use the result to prevent elimination */
    result += (v4si){low_half[0], low_half[1], 0, 0};
    
    return result;
}

/* ========== SCENARIO 4: Mixed bounds with volatile control ========== */
__attribute__((noinline))
static int test_mixed_bounds(void) {
    float farray[50];
    int sum = 0;
    
    /* Use volatile to control which constant bounds are seen */
    volatile int selector = g_volatile_zero;
    
    for (int i = 0; i < (selector ? 5 : 10); ++i) {
        /* Compiler sees both bounds 5 and 10 during analysis */
        farray[i] = i * 1.5f;
    }
    
    /* Actually use one path at runtime */
    int limit = (g_volatile_zero == 0) ? 10 : 5;
    for (int i = 0; i < limit; ++i) {
        sum += (int)farray[i];
    }
    
    /* Vector version with conditional bounds */
    v4sf v = {1.0f, 2.0f, 3.0f, 4.0f};
    float *pv = (float*)&v;
    
    int start = (g_volatile_zero == 0) ? 0 : 1;
    int end = (g_volatile_zero == 0) ? 4 : 3;
    
    for (int i = start; i < end; ++i) {
        sum += (int)pv[i];
    }
    
    return sum;
}

/* ========== SCENARIO 5: Struct with constant array section ========== */
__attribute__((noinline))
static int test_struct_array_section(void) {
    struct {
        int header;
        int data[20];
        int footer;
    } s = {0};
    
    int sum = 0;
    
    /* Access middle section: lo=5, hi=14, count=10 */
    for (int i = 5; i < 15; ++i) {
        s.data[i] = i * 2;
    }
    
    if (g_volatile_zero == 0) {
        for (int i = 5; i < 15; ++i) {
            sum += s.data[i];
        }
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */
int main(void) {
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_mem();
    checksum += test_larger_constant_mem();
    
    v4si vec_result = test_non_mem_vector();
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    checksum += test_mixed_bounds();
    checksum += test_struct_array_section();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
