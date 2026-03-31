/* Test program to cover constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing our test cases */
static volatile int g_volatile = 0;

/* Vector types for non-MEM_P cases */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Small struct for 2-element access */
struct two_elem {
    int a;
    int b;
};

/* ========== SCENARIO 1: Small count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100] = {0};
    struct two_elem s = {1, 2};
    int sum = 0;
    
    /* Single element access - count = 1 */
    sum += arr[5];  /* const bounds: lo=5, hi=5, count=1 */
    
    /* Two adjacent elements - count = 2 */
    sum += arr[10] + arr[11];  /* const bounds: lo=10, hi=11, count=2 */
    
    /* Struct access - also count = 2 for two ints */
    sum += s.a + s.b;
    
    /* Vector with 2-element access using GNU extension */
    typedef int v2si __attribute__((vector_size(8)));
    v2si v = {10, 20};
    int* p = (int*)&v;
    sum += p[0] + p[1];  /* Access both elements */
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_access(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Access 8 elements - count = 8, fits in HWI */
    /* Use volatile to prevent loop unrolling from changing the pattern */
    volatile int start = 20;
    for (int i = start ? 20 : 25; i < (start ? 28 : 33); ++i) {
        sum += arr[i];  /* const bounds when start is known: lo=20, hi=27, count=8 */
    }
    
    /* Another example with char type - larger count but small total size */
    char buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = i & 0xFF;
    }
    
    /* Access 64 chars - count=64, TYPE_SIZE=8 bits, total=512 bits fits in HWI */
    int char_sum = 0;
    for (int i = 32; i < 96; ++i) {  /* lo=32, hi=95, count=64 */
        char_sum += buffer[i];
    }
    
    return sum + char_sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a memory reference */
    v4si c = __builtin_shufflevector(a, b, 0, 1, 4, 5);
    /* Indices: 0,1 from 'a', 0,1 from 'b' (4,5 in combined indexing) */
    
    /* Vector compound literal with constant indices */
    v4si d = (v4si){a[0], a[1], b[0], b[1]};
    /* Constant bounds for each element access in the constructor */
    
    /* Vector permutation with mask */
    v4si mask = {0, 2, 1, 3};
    v4si e = __builtin_shuffle(a, mask);
    
    return c + d + e;
}

/* ========== SCENARIO 4: Mixed array section with constant bounds ========== */
__attribute__((noinline))
static int test_mixed_array_sections(void) {
    int arr[50];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        arr[i] = i * 2;
    }
    
    /* Multiple constant-bounded sections */
    
    /* Section 1: Single element */
    sum += arr[10];
    
    /* Section 2: Two elements */
    sum += arr[20] + arr[21];
    
    /* Section 3: Medium section (5 elements) */
    for (int i = 30; i < 35; i++) {
        sum += arr[i];
    }
    
    /* Section 4: Using computed but constant bounds */
    const int lo = 40;
    const int hi = 49;
    for (int i = lo; i <= hi; i++) {
        sum += arr[i];  /* lo=40, hi=49, count=10 */
    }
    
    return sum;
}

/* ========== SCENARIO 5: Vector memory access with constant bounds ========== */
__attribute__((noinline))
static int test_vector_memory_access(void) {
    v4si vectors[10];
    int sum = 0;
    
    /* Initialize vectors */
    for (int i = 0; i < 10; i++) {
        vectors[i] = (v4si){i, i+1, i+2, i+3};
    }
    
    /* Access individual vector elements with constant indices */
    sum += vectors[2][0];  /* First element of third vector */
    sum += vectors[2][1];  /* Second element */
    sum += vectors[2][2];  /* Third element */
    sum += vectors[2][3];  /* Fourth element */
    
    /* Access slice of vector array */
    for (int i = 3; i < 7; i++) {
        sum += vectors[i][0];  /* Access first element of vectors 3-6 */
    }
    
    return sum;
}

/* ========== SCENARIO 6: Conditional constant bounds ========== */
__attribute__((noinline))
static int test_conditional_bounds(void) {
    int arr[100];
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Use volatile to prevent constant propagation from removing the condition */
    volatile int choice = g_volatile;
    
    if (choice) {
        /* This path has count = 4 */
        for (int i = 10; i < 14; i++) {
            sum += arr[i];
        }
    } else {
        /* This path has count = 6 */
        for (int i = 20; i < 26; i++) {
            sum += arr[i];
        }
    }
    
    /* Another conditional with different element types */
    float farr[50];
    for (int i = 0; i < 50; i++) {
        farr[i] = i * 1.5f;
    }
    
    if (choice & 1) {
        /* Access 3 floats */
        for (int i = 5; i < 8; i++) {
            sum += (int)farr[i];
        }
    } else {
        /* Access 7 floats */
        for (int i = 15; i < 22; i++) {
            sum += (int)farr[i];
        }
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Run all test scenarios */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_access();
    
    v4si vec_result = test_non_memory_vector();
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_array_sections();
    checksum += test_vector_memory_access();
    checksum += test_conditional_bounds();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
