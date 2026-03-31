/* Test program to cover constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void)
{
    int arr[100] = {0};
    int sum = 0;
    
    /* Single element access - count = 1 */
    if (g_volatile) {
        arr[5] = 42;        /* Constant lower and upper bound = 5 */
    } else {
        arr[3+2] = 43;      /* Constant expression bound = 5 */
    }
    sum += arr[5];          /* count = 1, MEM_P true, count <= 2 */
    
    /* Two adjacent elements - count = 2 */
    struct two_ints { int a; int b; } s;
    s.a = arr[10];          /* Access through struct member */
    s.b = arr[11];          /* Adjacent access, count = 2 */
    sum += s.a + s.b;
    
    /* Vector with 2 elements */
    v2si v2;
    int* pv2 = (int*)&v2;
    pv2[0] = arr[20];       /* First element */
    pv2[1] = arr[21];       /* Second element, count = 2 */
    sum += pv2[0] + pv2[1];
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_sized(void)
{
    int arr[100] = {0};
    int sum = 0;
    
    /* Fixed-size array slice: 8 elements * 4 bytes = 32 bytes = 256 bits */
    /* TYPE_SIZE(int) = 32 bits, count = 8, total = 256 bits fits in uhwi */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        arr[i] = i * 2;
        sum += arr[i];
    }
    
    /* Different element type: char array, 32 elements * 1 byte = 32 bytes */
    char carr[64] = {0};
    for (int i = 16; i < 48; ++i) {  /* lo=16, hi=47, count=32 */
        carr[i] = (char)(i % 256);
        sum += carr[i];
    }
    
    /* Use volatile to prevent bound elimination but keep them constant */
    int start = g_volatile ? 30 : 30;  /* Always 30, but compiler doesn't know */
    for (int i = start; i < start + 15; ++i) {  /* lo=30, hi=44, count=15 */
        arr[i] = i * 3;
        sum += arr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory reference cases (!MEM_P) ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void)
{
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    /* Not a memory reference, but has constant bounds for the permute */
    v4si shuffled = __builtin_shufflevector(v1, v2, 0, 2, 1, 3);
    
    /* Vector compound literal with constant indices */
    v4si constructed = (v4si){v1[0], v1[1], v2[0], v2[1]};  /* CONSTRUCTOR node */
    
    /* Vector section selection with constant bounds */
    v4si result;
    for (int i = 0; i < 2; ++i) {  /* Constant bounds, small count */
        result[i] = shuffled[i] + constructed[i];
    }
    for (int i = 2; i < 4; ++i) {  /* Constant bounds, small count */
        result[i] = shuffled[i] * constructed[i];
    }
    
    return result;
}

/* ========== SCENARIO 4: Mixed cases with different element types ========== */
__attribute__((noinline))
static float test_mixed_types(void)
{
    float farr[50];
    double darr[30];
    v4sf vf;
    
    /* Initialize arrays */
    for (int i = 0; i < 50; ++i) farr[i] = i * 1.5f;
    for (int i = 0; i < 30; ++i) darr[i] = i * 2.5;
    
    float sum = 0.0f;
    
    /* Float array with constant bounds, count = 4 */
    for (int i = 5; i < 9; ++i) {  /* lo=5, hi=8, count=4 */
        sum += farr[i];
    }
    
    /* Double array with constant bounds, count = 6 */
    int base = g_volatile ? 10 : 10;  /* Constant but opaque to compiler */
    for (int i = base; i < base + 6; ++i) {  /* lo=10, hi=15, count=6 */
        sum += (float)darr[i];
    }
    
    /* Vector float operations - non-memory */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vsum = v1 + v2;  /* Vector operation, not memory load/store */
    
    /* Extract constant-bounded elements from vector result */
    sum += vsum[0] + vsum[1] + vsum[2] + vsum[3];  /* All constant indices */
    
    return sum;
}

/* ========== SCENARIO 5: Edge cases with bit-sized computations ========== */
__attribute__((noinline))
static int test_edge_cases(void)
{
    /* Test with different integer types to exercise TYPE_SIZE computations */
    int8_t  s8arr[128];
    int16_t s16arr[64];
    int32_t s32arr[32];
    int64_t s64arr[16];
    
    int sum = 0;
    
    /* 8-bit elements: 16 elements * 8 bits = 128 bits */
    for (int i = 0; i < 16; ++i) {
        s8arr[i] = i % 128;
        sum += s8arr[i];
    }
    
    /* 16-bit elements: 8 elements * 16 bits = 128 bits */
    for (int i = 4; i < 12; ++i) {  /* lo=4, hi=11, count=8 */
        s16arr[i] = i * 2;
        sum += s16arr[i];
    }
    
    /* 32-bit elements: 4 elements * 32 bits = 128 bits */
    for (int i = 8; i < 12; ++i) {  /* lo=8, hi=11, count=4 */
        s32arr[i] = i * 3;
        sum += s32arr[i];
    }
    
    /* 64-bit elements: 2 elements * 64 bits = 128 bits (count <= 2) */
    s64arr[5] = 100;
    s64arr[6] = 200;  /* lo=5, hi=6, count=2 */
    sum += (int)(s64arr[5] + s64arr[6]);
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void)
{
    int checksum = 0;
    
    /* Call all test functions to exercise different paths */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_sized();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; ++i) {
        checksum += vec_result[i];
    }
    
    checksum += (int)test_mixed_types();
    checksum += test_edge_cases();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
