/* Test program for constant-bounds array/vector analysis in GCC expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4f __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */

/* Single element access - count = 1 */
__attribute__((noinline))
static int test_small_count_1(int *arr) {
    int sum = 0;
    /* Force compiler to see constant bounds [5,5] */
    int lo = g_volatile ? 3 : 5;
    int hi = g_volatile ? 7 : 5;
    
    /* Single element - count = 1 */
    if (lo == 5 && hi == 5) {
        sum += arr[5];  /* MEM_P with count=1 */
    }
    return sum;
}

/* Two adjacent elements - count = 2 */
__attribute__((noinline))
static int test_small_count_2(int *arr) {
    int sum = 0;
    /* Constant bounds [2,3] */
    for (int i = 2; i <= 3; ++i) {
        sum += arr[i];  /* MEM_P with count=2 */
    }
    return sum;
}

/* Vector with 2 elements */
__attribute__((noinline))
static v2si test_vector_small(v2si *vec) {
    v2si result;
    /* Access two elements of vector - count=2 */
    result[0] = (*vec)[0];
    result[1] = (*vec)[1];
    return result;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Larger constant-sized block where TYPE_SIZE * count fits in uhwi */
__attribute__((noinline))
static int test_larger_constant_block(char *data) {
    int sum = 0;
    /* Access 10 chars: TYPE_SIZE=8 bits, count=10, total=80 bits < HWI */
    for (int i = 2; i < 12; ++i) {  /* bounds [2,11], count=10 */
        sum += data[i];
    }
    return sum;
}

/* Integer array with medium-sized constant slice */
__attribute__((noinline))
static int test_int_array_slice(int *arr) {
    int sum = 0;
    /* Access 8 ints: TYPE_SIZE=32 bits, count=8, total=256 bits */
    for (int i = 5; i < 13; ++i) {  /* bounds [5,12], count=8 */
        sum += arr[i];
    }
    return sum;
}

/* Struct with constant-sized access */
struct small_struct {
    char a;
    char b;
    char c;
};

__attribute__((noinline))
static int test_struct_array(struct small_struct *arr) {
    int sum = 0;
    /* Access 4 structs: TYPE_SIZE=24 bits, count=4, total=96 bits */
    for (int i = 0; i < 4; ++i) {  /* bounds [0,3], count=4 */
        sum += arr[i].a + arr[i].b;
    }
    return sum;
}

/* ========== SCENARIO 3: Non-memory reference cases (!MEM_P) ========== */

/* Vector shuffle with constant indices - returns VEC_PERM_EXPR */
__attribute__((noinline))
static v4si test_vector_shuffle(v4si a, v4si b) {
    /* Create constant-bounded vector section via shuffle */
    v4si result = __builtin_shufflevector(a, b, 0, 1, 2, 3);
    return result;
}

/* Vector compound literal with constant indices */
__attribute__((noinline))
static v4si test_vector_constructor(v4si v) {
    /* Create new vector from constant indices */
    v4si result = (v4si){v[0], v[1], v[2], v[3]};
    return result;
}

/* Vector slice using shuffle with constant bounds */
__attribute__((noinline))
static v2si test_vector_slice(v4si v) {
    /* Extract first two elements - constant bounds [0,1] */
    v2si result = __builtin_shufflevector(v, v, 0, 1);
    return result;
}

/* ========== SCENARIO 4: Mixed bounds with volatile control ========== */

/* Test with volatile-controlled constant bounds */
__attribute__((noinline))
static int test_volatile_bounds(int *arr) {
    int sum = 0;
    /* Use volatile to prevent constant propagation of bounds,
       but both paths have constant bounds */
    if (g_volatile) {
        /* Path 1: bounds [1,4], count=4 */
        for (int i = 1; i <= 4; ++i) {
            sum += arr[i];
        }
    } else {
        /* Path 2: bounds [10,15], count=6 */
        for (int i = 10; i <= 15; ++i) {
            sum += arr[i];
        }
    }
    return sum;
}

/* Test with computed but constant bounds */
__attribute__((noinline))
static int test_computed_bounds(int *arr) {
    int sum = 0;
    /* Bounds are constant expressions but computed */
    int start = 2 + 3;      /* 5 */
    int end = 3 * 4;        /* 12 */
    
    for (int i = start; i < end; ++i) {  /* bounds [5,11], count=7 */
        sum += arr[i];
    }
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize test data */
    int int_array[100];
    char char_array[100];
    struct small_struct struct_array[10];
    v2si vec2 = {1, 2};
    v4si vec4 = {1, 2, 3, 4};
    v4si vec4b = {5, 6, 7, 8};
    
    for (int i = 0; i < 100; ++i) {
        int_array[i] = i;
        char_array[i] = i & 0xFF;
    }
    for (int i = 0; i < 10; ++i) {
        struct_array[i].a = i;
        struct_array[i].b = i + 1;
        struct_array[i].c = i + 2;
    }
    
    /* Execute all test scenarios */
    checksum += test_small_count_1(int_array);
    checksum += test_small_count_2(int_array);
    
    v2si vec2_result = test_vector_small(&vec2);
    checksum += vec2_result[0] + vec2_result[1];
    
    checksum += test_larger_constant_block(char_array);
    checksum += test_int_array_slice(int_array);
    checksum += test_struct_array(struct_array);
    
    v4si shuffle_result = test_vector_shuffle(vec4, vec4b);
    checksum += shuffle_result[0] + shuffle_result[1];
    
    v4si constructor_result = test_vector_constructor(vec4);
    checksum += constructor_result[2] + constructor_result[3];
    
    v2si slice_result = test_vector_slice(vec4);
    checksum += slice_result[0] + slice_result[1];
    
    checksum += test_volatile_bounds(int_array);
    checksum += test_computed_bounds(int_array);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
