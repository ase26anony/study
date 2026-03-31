/* Test program for constant-bounds array/vector analysis in GCC expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100] = {0};
    int sum = 0;
    
    /* Single element access - count = 1 */
    arr[5] = 42;           /* const_bounds_p: lo=5, hi=5, count=1 <= 2 */
    sum += arr[5];
    
    /* Two adjacent elements - count = 2 */
    arr[10] = 1;           /* lo=10, hi=11, count=2 <= 2 */
    arr[11] = 2;
    sum += arr[10] + arr[11];
    
    /* Use volatile to prevent constant propagation from removing bounds */
    int idx = g_volatile ? 20 : 30;
    arr[idx] = 99;         /* This part has variable index, but previous accesses are constant */
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_memory(void) {
    int arr[100] = {0};
    int sum = 0;
    
    /* Access 8 elements: TYPE_SIZE(int)=32, count=8, total=256 bits fits in uhwi */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        arr[i] = i * 2;
    }
    
    /* Compute checksum */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    
    /* Another case with char type: TYPE_SIZE(char)=8, count=20, total=160 bits */
    char buffer[50];
    for (int i = 5; i < 25; ++i) {  /* lo=5, hi=24, count=20 */
        buffer[i] = (char)(i & 0xFF);
        sum += buffer[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a direct memory reference */
    v4si shuffled = __builtin_shufflevector(v1, v2, 0, 2, 1, 3);
    
    /* Vector compound literal with constant indices */
    v4si sliced = (v4si){v1[0], v1[1], v2[0], v2[1]};
    
    /* Vector permutation with constant mask */
    v4si permuted;
    int mask[4] = {0, 2, 1, 3};
    for (int i = 0; i < 4; ++i) {
        permuted[i] = mask[i] < 4 ? v1[mask[i]] : v2[mask[i] - 4];
    }
    
    /* Return combination to prevent elimination */
    return shuffled + sliced + permuted;
}

/* ========== SCENARIO 4: Mixed array section with constant bounds ========== */
__attribute__((noinline))
static int test_mixed_array_sections(void) {
    float farray[50];
    int sum = 0;
    
    /* Initialize with volatile to prevent complete optimization */
    for (int i = 0; i < 50; ++i) {
        farray[i] = (float)(i + g_volatile);
    }
    
    /* Constant-bounded section in middle of array */
    for (int i = 15; i < 35; ++i) {  /* lo=15, hi=34, count=20 */
        farray[i] *= 2.0f;
        sum += (int)farray[i];
    }
    
    /* Small section at end */
    farray[48] = 100.0f;  /* count=1 */
    farray[49] = 200.0f;  /* count=1, but separate accesses */
    sum += (int)(farray[48] + farray[49]);
    
    return sum;
}

/* ========== SCENARIO 5: Vector memory access with constant bounds ========== */
__attribute__((noinline))
static int test_vector_memory_access(void) {
    v4si vectors[10];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; ++i) {
        vectors[i] = (v4si){i*4, i*4+1, i*4+2, i*4+3};
    }
    
    /* Access constant indices in vector array */
    for (int i = 2; i < 6; ++i) {  /* lo=2, hi=5, count=4 */
        for (int j = 0; j < 4; ++j) {
            sum += vectors[i][j];
        }
    }
    
    /* Single vector element access */
    sum += vectors[0][0];  /* count=1 */
    sum += vectors[0][1];  /* count=1, separate */
    
    return sum;
}

/* ========== SCENARIO 6: Struct with array member ========== */
__attribute__((noinline))
static int test_struct_array_member(void) {
    struct {
        int header;
        int data[20];
        int footer;
    } s;
    
    int sum = 0;
    
    /* Constant-bounded access to struct array member */
    for (int i = 3; i < 15; ++i) {  /* lo=3, hi=14, count=12 */
        s.data[i] = i * 3;
        sum += s.data[i];
    }
    
    /* Small access */
    s.data[0] = 99;  /* count=1 */
    sum += s.data[0];
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_memory();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; ++i) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_array_sections();
    checksum += test_vector_memory_access();
    checksum += test_struct_array_member();
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
