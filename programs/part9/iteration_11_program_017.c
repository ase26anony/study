/* Test case for GCC expr.cc constant bounds analysis coverage.
   Specifically targeting lines 7691-7700 in expr.cc. */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating test functions */
#define NOINLINE __attribute__((noinline))

/* Vector types for non-memory reference tests */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent constant propagation */
extern volatile int g_volatile_zero;
extern volatile int g_volatile_one;

/* ========== SCENARIO 1: Small count (count <= 2) memory access ========== */
NOINLINE static void test_small_count_memory(void) {
    int arr[100] = {0};
    v2si vec2 = {1, 2};
    
    /* Single element access - count = 1 */
    if (g_volatile_zero) {
        /* This branch uses constant bounds but won't be eliminated */
        arr[5] = 42;  /* lo=5, hi=5, count=1 <= 2 */
    } else {
        arr[10] = 43; /* Alternative constant bound */
    }
    
    /* Two adjacent elements - count = 2 */
    /* Using struct to force two-element memory access */
    struct two_ints { int a; int b; } two;
    two.a = arr[20];  /* Could be optimized as two loads */
    two.b = arr[21];  /* lo=20, hi=21, count=2 <= 2 */
    
    /* Vector element access - still memory reference */
    int first = vec2[0];  /* lo=0, hi=0, count=1 */
    int second = vec2[1]; /* lo=1, hi=1, count=1 */
    
    /* Prevent dead store elimination */
    if (g_volatile_one) {
        printf("%d %d\n", two.a + two.b, first + second);
    }
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
NOINLINE static int test_larger_constant_memory(void) {
    int arr[100];
    char bytes[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) arr[i] = i;
    for (int i = 0; i < 256; i++) bytes[i] = i & 0xFF;
    
    int sum = 0;
    
    /* Access 8 ints: count=8, TYPE_SIZE(int)=32, total=256 bits fits in uhwi */
    /* Use volatile condition to preserve loop structure */
    int start = g_volatile_zero ? 2 : 10;
    for (int i = start; i < start + 8; ++i) {  /* lo=start, hi=start+7, count=8 */
        sum += arr[i];
    }
    
    /* Access 64 chars: count=64, TYPE_SIZE(char)=8, total=512 bits fits */
    int byte_start = g_volatile_one ? 32 : 64;
    for (int i = byte_start; i < byte_start + 64; ++i) {  /* lo=byte_start, hi=byte_start+63, count=64 */
        sum += bytes[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
NOINLINE static v4si test_non_memory_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a direct memory load */
    /* This creates VEC_PERM_EXPR or CONSTRUCTOR, not MEM_REF */
    v4si shuffled;
    if (g_volatile_zero) {
        /* Constant bounds: indices 0,1,2,3 */
        shuffled = __builtin_shufflevector(v1, v2, 0, 1, 2, 3);
    } else {
        /* Alternative constant bounds: indices 3,2,1,0 */
        shuffled = __builtin_shufflevector(v1, v2, 3, 2, 1, 0);
    }
    
    /* Vector compound literal with constant indices */
    v4si constructed;
    if (g_volatile_one) {
        /* Constant bounds: accessing elements 0,1 from v1 and 0,1 from v2 */
        constructed = (v4si){v1[0], v1[1], v2[0], v2[1]};  /* lo=0, hi=1 for each */
    } else {
        constructed = (v4si){v1[3], v1[2], v2[3], v2[2]};  /* lo=2, hi=3 for each */
    }
    
    return shuffled + constructed;
}

/* ========== SCENARIO 4: Mixed array section with constant bounds ========== */
NOINLINE static int test_mixed_array_sections(void) {
    float farray[50];
    double darray[30];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) farray[i] = i * 1.5f;
    for (int i = 0; i < 30; i++) darray[i] = i * 2.5;
    
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Constant bounds that vary based on volatile condition */
    int fstart = g_volatile_zero ? 5 : 15;
    int fend = fstart + 12;  /* 12 elements: count=12, TYPE_SIZE(float)=32, total=384 bits */
    
    for (int i = fstart; i < fend; ++i) {
        fsum += farray[i];
    }
    
    /* Different element type, still constant bounds */
    int dstart = g_volatile_one ? 3 : 8;
    int dend = dstart + 6;  /* 6 elements: count=6, TYPE_SIZE(double)=64, total=384 bits */
    
    for (int i = dstart; i < dend; ++i) {
        dsum += darray[i];
    }
    
    return (int)(fsum + dsum);
}

/* ========== SCENARIO 5: Struct with array members ========== */
NOINLINE static int test_struct_array_member(void) {
    struct with_array {
        int header;
        int data[20];
        int footer;
    } s;
    
    /* Initialize */
    s.header = 100;
    for (int i = 0; i < 20; i++) s.data[i] = i * 3;
    s.footer = 200;
    
    int sum = s.header + s.footer;
    
    /* Access constant range within struct member array */
    /* This creates memory references with constant bounds */
    int start = g_volatile_zero ? 2 : 8;
    for (int i = start; i < start + 5; ++i) {  /* lo=start, hi=start+4, count=5 */
        sum += s.data[i];
    }
    
    /* Also test single element access (count=1) */
    sum += s.data[15];  /* lo=15, hi=15, count=1 */
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile globals (simulated) */
    int volatile g_volatile_zero = 0;
    int volatile g_volatile_one = 1;
    
    /* Run all test scenarios */
    test_small_count_memory();
    
    checksum += test_larger_constant_memory();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; i++) checksum += vec_result[i];
    
    checksum += test_mixed_array_sections();
    
    checksum += test_struct_array_member();
    
    /* Print checksum to prevent optimization and verify execution */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
