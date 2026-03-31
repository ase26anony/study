/* Test program to cover constant-bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small count (count <= 2) memory references ========== */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100] = {0};
    int sum = 0;
    
    /* Single element access (count = 1) */
    arr[5] = 42;           /* Constant lower and upper bound: [5, 5] */
    sum += arr[5];
    
    /* Two adjacent elements (count = 2) */
    arr[10] = 1;
    arr[11] = 2;           /* Bounds: [10, 11], count = 2 */
    sum += arr[10] + arr[11];
    
    /* Use volatile to prevent constant propagation from eliminating the code */
    int idx = g_volatile ? 20 : 30;
    arr[idx] = 99;         /* This part is variable, but previous accesses are constant */
    
    return sum;
}

__attribute__((noinline))
static int test_small_struct_memory(void) {
    struct two_ints { int a; int b; } s;
    s.a = 10;              /* Access to first element */
    s.b = 20;              /* Access to second element - effectively count = 2 */
    return s.a + s.b;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_slice(void) {
    int arr[100];
    int sum = 0;
    
    /* Fixed-size array slice: bounds [2, 9], count = 8 */
    /* TYPE_SIZE(int) * count = 32 * 8 = 256 bits, fits in unsigned HWI */
    for (int i = 2; i < 10; ++i) {
        arr[i] = i * 2;
        sum += arr[i];
    }
    
    /* Another slice with char type: bounds [20, 39], count = 20 */
    /* TYPE_SIZE(char) * count = 8 * 20 = 160 bits */
    char char_arr[100];
    for (int i = 20; i < 40; ++i) {
        char_arr[i] = (char)(i % 256);
        sum += char_arr[i];
    }
    
    /* Use volatile to keep loop structure */
    if (g_volatile) {
        /* Alternative bounds that compiler must analyze */
        for (int i = 5; i < 15; ++i) {
            arr[i] = i * 3;
        }
    }
    
    return sum;
}

__attribute__((noinline))
static int test_mixed_bounds(void) {
    float farr[50];
    int sum = 0;
    
    /* Constant bounds determined by conditional with volatile */
    int start = g_volatile ? 5 : 10;
    int end = g_volatile ? 15 : 25;
    
    /* The compiler sees both possibilities during analysis */
    for (int i = start; i < end; ++i) {
        farr[i] = i * 1.5f;
        sum += (int)farr[i];
    }
    
    /* Constant sub-range within variable loop */
    for (int i = 0; i < 30; ++i) {
        if (i >= 10 && i < 20) {  /* Constant bounds [10, 19], count = 10 */
            farr[i] = 100.0f;
            sum += (int)farr[i];
        }
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_vector_shuffle(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a direct memory reference */
    /* This creates a VEC_PERM_EXPR or similar, not MEM_REF */
    v4si result = __builtin_shufflevector(v1, v2, 0, 2, 4, 6);
    
    /* Vector compound literal with constant indices */
    v4si slice = (v4si){v1[0], v1[1], v1[2], v1[3]};  /* All constant indices */
    
    /* Add them to prevent elimination */
    for (int i = 0; i < 4; ++i) {
        result[i] += slice[i];
    }
    
    return result;
}

__attribute__((noinline))
static v2si test_small_vector_ops(void) {
    v2si v = {10, 20};
    
    /* Various operations on 2-element vector with constant indices */
    v2si rotated = (v2si){v[1], v[0]};  /* Constant bounds [0, 1] */
    
    /* Conditional selection with constant indices */
    if (g_volatile) {
        return (v2si){v[0], v[0]};
    } else {
        return (v2si){v[1], v[1]};
    }
}

/* ========== SCENARIO 4: Complex constant bounds in nested structures ========== */
__attribute__((noinline))
static int test_nested_array_sections(void) {
    int arr[10][20];
    int sum = 0;
    
    /* Constant bounds in multi-dimensional array */
    for (int i = 2; i < 6; ++i) {          /* Outer bounds: [2, 5], count = 4 */
        for (int j = 3; j < 8; ++j) {      /* Inner bounds: [3, 7], count = 5 */
            arr[i][j] = i * j;
            sum += arr[i][j];
        }
    }
    
    /* Partial constant bounds */
    int row = g_volatile ? 1 : 8;
    for (int j = 5; j < 10; ++j) {         /* Constant column bounds: [5, 9] */
        arr[row][j] = j * 2;
        sum += arr[row][j];
    }
    
    return sum;
}

/* ========== SCENARIO 5: Bitfield and packed structure access ========== */
__attribute__((noinline))
static int test_bitfield_access(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } __attribute__((packed)) p;
    
    /* Access multiple bitfields - treated as array of bits with constant bounds */
    p.a = 5;
    p.b = 6;  /* These are adjacent bitfields, count = 2 in terms of bitfield elements */
    p.c = 7;
    p.d = 8;  /* Total of 4 elements, but bit offsets are constant */
    
    return p.a + p.b + p.c + p.d;
}

/* ========== Main function to execute all tests ========== */
int main(void) {
    int checksum = 0;
    
    /* Execute all test scenarios */
    checksum += test_small_count_memory();
    checksum += test_small_struct_memory();
    checksum += test_larger_constant_slice();
    checksum += test_mixed_bounds();
    
    v4si vresult = test_vector_shuffle();
    for (int i = 0; i < 4; ++i) {
        checksum += vresult[i];
    }
    
    v2si v2result = test_small_vector_ops();
    checksum += v2result[0] + v2result[1];
    
    checksum += test_nested_array_sections();
    checksum += test_bitfield_access();
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
