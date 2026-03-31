/* Test program to cover constant bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */
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
    sum += arr[5];  /* const_bounds_p: lo=5, hi=5, count=1 */
    
    /* Two adjacent elements - count = 2 */
    sum += arr[10] + arr[11];  /* lo=10, hi=11, count=2 */
    
    /* Use volatile to prevent constant folding */
    int idx = g_volatile ? 20 : 30;
    /* Still constant bounds: lo=20, hi=21, count=2 */
    sum += arr[idx] + arr[idx + 1];
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_sized(void)
{
    int arr[200];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 200; i++) {
        arr[i] = i * 2;
    }
    
    /* Access 8 elements - TYPE_SIZE(int)=32, count=8, total=256 bits */
    /* This should fit in unsigned HWI on 64-bit systems */
    for (int i = 2; i < 10; i++) {  /* lo=2, hi=9, count=8 */
        sum += arr[i];
    }
    
    /* Different element type with smaller size */
    char char_arr[1000];
    for (int i = 0; i < 1000; i++) {
        char_arr[i] = (char)(i % 256);
    }
    
    /* Access 100 chars - TYPE_SIZE(char)=8, count=100, total=800 bits */
    for (int i = 100; i < 200; i++) {  /* lo=100, hi=199, count=100 */
        sum += char_arr[i];
    }
    
    /* Mixed bounds using conditional with volatile */
    int start = g_volatile ? 50 : 60;
    /* Still constant: lo=50/60, hi=69/79, count=20 */
    for (int i = start; i < start + 20; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void)
{
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a MEM_P reference */
    v4si shuffled = __builtin_shufflevector(vec1, vec2, 0, 2, 1, 3);
    
    /* Vector compound literal with constant indices */
    v4si sliced = (v4si){vec1[0], vec1[1], vec2[0], vec2[1]};
    
    /* Vector permute with constant mask */
    v4si permuted;
    int mask[4] = {0, 2, 1, 3};
    /* Use inline asm to prevent optimization while keeping constant indices */
    __asm__ volatile("" : "+r"(vec1), "+r"(vec2));
    
    /* Return combination of results */
    return shuffled + sliced;
}

/* ========== SCENARIO 4: Mixed array/vector with struct ========== */
struct small_struct {
    int a;
    int b;
};

__attribute__((noinline))
static int test_struct_access(void)
{
    struct small_struct arr[50];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        arr[i].a = i;
        arr[i].b = i * 2;
    }
    
    /* Access two adjacent struct elements - count=2 for the struct array */
    sum += arr[5].a + arr[5].b + arr[6].a + arr[6].b;
    
    /* Single struct element access */
    sum += arr[10].a;
    
    return sum;
}

/* ========== SCENARIO 5: Vector memory access with constant bounds ========== */
__attribute__((noinline))
static int test_vector_memory(void)
{
    v4si vectors[20];
    int sum = 0;
    
    /* Initialize vectors */
    for (int i = 0; i < 20; i++) {
        vectors[i] = (v4si){i, i+1, i+2, i+3};
    }
    
    /* Access single vector element - count=1 */
    sum += vectors[3][0];
    
    /* Access two vector elements - count=2 */
    sum += vectors[5][1] + vectors[5][2];
    
    /* Access range of vector elements within a single vector */
    v4si v = vectors[10];
    /* This creates a non-memory reference with constant bounds */
    int partial_sum = v[0] + v[1] + v[2];  /* lo=0, hi=2, count=3 */
    
    return sum + partial_sum;
}

/* ========== SCENARIO 6: Multi-dimensional array with constant bounds ========== */
__attribute__((noinline))
static int test_multi_dimensional(void)
{
    int matrix[10][20];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds in both dimensions */
    for (int i = 2; i < 6; i++) {      /* lo=2, hi=5, count=4 */
        for (int j = 3; j < 8; j++) {  /* lo=3, hi=7, count=5 */
            sum += matrix[i][j];
        }
    }
    
    /* Single row access */
    for (int j = 0; j < 5; j++) {  /* lo=0, hi=4, count=5 */
        sum += matrix[8][j];
    }
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void)
{
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_sized();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    checksum += test_struct_access();
    checksum += test_vector_memory();
    checksum += test_multi_dimensional();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
