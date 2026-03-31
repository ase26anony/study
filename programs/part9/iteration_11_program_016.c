/* Test program to cover constant-bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile_zero = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

/* ========== Scenario 1: Small count memory access (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100] = {0};
    int sum = 0;
    
    /* Single element access - count = 1 */
    arr[5] = 42;                     /* const bounds: lo=5, hi=5, count=1 */
    sum += arr[5];
    
    /* Two adjacent elements - count = 2 */
    arr[10] = 1;                     /* const bounds: lo=10, hi=11, count=2 */
    arr[11] = 2;
    sum += arr[10] + arr[11];
    
    /* Use volatile to prevent constant propagation from eliminating loop */
    int start = g_volatile_zero ? 20 : 15;
    for (int i = start; i < start + 2; ++i) {  /* count=2 constant bounds */
        arr[i] = i * 3;
        sum += arr[i];
    }
    
    return sum;
}

/* ========== Scenario 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_memory(void) {
    char char_arr[256] = {0};
    int int_arr[64] = {0};
    int sum = 0;
    
    /* Access 10 chars: TYPE_SIZE=8 bits, count=10, total=80 bits fits in uhwi */
    for (int i = 2; i < 12; ++i) {   /* const bounds: lo=2, hi=11, count=10 */
        char_arr[i] = (char)(i * 7);
        sum += char_arr[i];
    }
    
    /* Access 8 ints: TYPE_SIZE=32 bits, count=8, total=256 bits fits in uhwi */
    int base = g_volatile_zero ? 0 : 8;
    for (int i = base; i < base + 8; ++i) {  /* const bounds with variable base */
        int_arr[i] = i * 11;
        sum += int_arr[i];
    }
    
    /* Struct with constant-sized array member */
    struct {
        int header;
        float data[20];
    } s;
    
    for (int i = 0; i < 15; ++i) {   /* const bounds: lo=0, hi=14, count=15 */
        s.data[i] = i * 1.5f;
        sum += (int)s.data[i];
    }
    
    return sum;
}

/* ========== Scenario 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a memory reference */
    v4si shuffled = __builtin_shufflevector(v1, v2, 0, 2, 1, 3);
    
    /* Vector compound literal with constant indices */
    v4si constructed = (v4si){v1[0], v1[1], v2[0], v2[1]};
    
    /* Vector permutation with constant mask */
    v4si permuted;
    int mask[4] = {0, 2, 1, 3};
    for (int i = 0; i < 4; ++i) {
        permuted[i] = (mask[i] < 4) ? v1[mask[i]] : v2[mask[i] - 4];
    }
    
    /* Return combination to prevent elimination */
    return shuffled + constructed + permuted;
}

/* ========== Scenario 4: Mixed vector memory and non-memory ========== */
__attribute__((noinline))
static int test_mixed_vector_scenarios(void) {
    v4si vec_arr[10];
    int sum = 0;
    
    /* Initialize vector array */
    for (int i = 0; i < 10; ++i) {
        vec_arr[i] = (v4si){i*4, i*4+1, i*4+2, i*4+3};
    }
    
    /* Memory reference to single vector element */
    sum += vec_arr[3][1];            /* const bounds: lo=1, hi=1, count=1 */
    
    /* Memory reference to two adjacent vector elements */
    sum += vec_arr[2][0] + vec_arr[2][1];  /* const bounds: lo=0, hi=1, count=2 */
    
    /* Non-memory: extract constant range from vector */
    v2si half_vec = (v2si){vec_arr[4][0], vec_arr[4][1]};
    sum += half_vec[0] + half_vec[1];
    
    /* Constant-bounded slice within loop */
    int idx = g_volatile_zero ? 5 : 6;
    for (int i = 0; i < 3; ++i) {    /* const bounds: lo=0, hi=2, count=3 */
        sum += vec_arr[idx][i];
    }
    
    return sum;
}

/* ========== Scenario 5: Multi-dimensional array with constant bounds ========== */
__attribute__((noinline))
static int test_multi_dimensional(void) {
    int matrix[10][20] = {{0}};
    int sum = 0;
    
    /* Constant bounds in both dimensions */
    for (int i = 2; i < 7; ++i) {            /* const bounds: lo=2, hi=6, count=5 */
        for (int j = 3; j < 8; ++j) {        /* const bounds: lo=3, hi=7, count=5 */
            matrix[i][j] = i * 100 + j;
            sum += matrix[i][j];
        }
    }
    
    /* Partial row with constant bounds */
    int row = g_volatile_zero ? 8 : 9;
    for (int j = 0; j < 5; ++j) {            /* const bounds: lo=0, hi=4, count=5 */
        sum += matrix[row][j];
    }
    
    return sum;
}

/* ========== Scenario 6: Bitfield and packed structures ========== */
__attribute__((noinline))
static int test_bitfield_access(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 12;
        unsigned int d : 8;
    } __attribute__((packed)) p[10];
    
    int sum = 0;
    
    /* Access multiple elements with constant bounds */
    for (int i = 0; i < 5; ++i) {    /* const bounds: lo=0, hi=4, count=5 */
        p[i].a = i;
        p[i].b = i * 2;
        sum += p[i].a + p[i].b;
    }
    
    /* Access specific bitfield members */
    p[6].c = 100;
    sum += p[6].c;
    
    return sum;
}

/* ========== Main function to execute all tests ========== */
int main(void) {
    int checksum = 0;
    
    checksum += test_small_count_memory();
    checksum += test_larger_constant_memory();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; ++i) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_vector_scenarios();
    checksum += test_multi_dimensional();
    checksum += test_bitfield_access();
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
