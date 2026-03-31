/* Test program to cover constant-bounds analysis in GCC expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile = 0;

/* Vector types using GNU extensions */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef char v16qi __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */

/* Single element load/store */
__attribute__((noinline))
static int test_small_count_1(int *arr) {
    int sum = 0;
    /* Single element access - count = 1 */
    sum += arr[5];  /* lo=5, hi=5, count=1 */
    arr[7] = sum;   /* Store with count=1 */
    return sum;
}

/* Two adjacent elements */
__attribute__((noinline))
static int test_small_count_2(int *arr) {
    int sum = 0;
    /* Two element access - count = 2 */
    sum += arr[2];  /* lo=2 */
    sum += arr[3];  /* hi=3, count=2 */
    return sum;
}

/* Vector with 2 elements */
__attribute__((noinline))
static v2si test_small_vector(v2si *vec) {
    v2si result;
    /* Accessing v[0] and v[1] - count=2 */
    result[0] = (*vec)[0];
    result[1] = (*vec)[1];
    return result;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Array slice with constant bounds, larger count */
__attribute__((noinline))
static int test_larger_slice(int *arr) {
    int sum = 0;
    /* lo=2, hi=9, count=8, element size=32 bits */
    /* total bits = 32 * 8 = 256, fits in uhwi */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Different element type (char) with larger count */
__attribute__((noinline))
static int test_char_array(char *arr) {
    int sum = 0;
    /* lo=10, hi=49, count=40, element size=8 bits */
    /* total bits = 8 * 40 = 320, fits in uhwi */
    for (int i = 10; i < 50; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Struct with known size */
struct small_struct {
    int a;
    char b;
};

__attribute__((noinline))
static int test_struct_array(struct small_struct *arr) {
    int sum = 0;
    /* lo=0, hi=4, count=5, element size likely 8 bytes (64 bits) */
    /* total bits = 64 * 5 = 320, fits in uhwi */
    for (int i = 0; i < 5; ++i) {
        sum += arr[i].a;
    }
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */

/* Vector shuffle with constant indices */
__attribute__((noinline))
static v4si test_vector_shuffle(v4si a, v4si b) {
    /* Create constant-bounded vector section without memory access */
    v4si result = __builtin_shufflevector(a, b, 0, 1, 2, 3);  /* All constant indices */
    return result;
}

/* Vector compound literal with constant indices */
__attribute__((noinline))
static v4si test_vector_constructor(v4si v) {
    /* Constant-bounded vector constructor */
    v4si result = (v4si){v[0], v[1], v[2], v[3]};  /* All constant indices */
    return result;
}

/* Vector permutation with constant mask */
__attribute__((noinline))
static v4sf test_vector_permute(v4sf a) {
    /* Constant permutation pattern */
    v4sf result = __builtin_shuffle(a, (v4si){3, 2, 1, 0});  /* Reverse order */
    return result;
}

/* ========== SCENARIO 4: Conditional constant bounds ========== */

/* Use volatile to prevent constant propagation from eliminating code */
__attribute__((noinline))
static int test_conditional_bounds(int *arr) {
    int sum = 0;
    int limit = g_volatile ? 5 : 10;  /* Volatile prevents constant folding */
    
    /* The compiler sees both possibilities during analysis */
    if (limit == 5) {
        /* lo=0, hi=4, count=5 */
        for (int i = 0; i < 5; ++i) {
            sum += arr[i];
        }
    } else {
        /* lo=0, hi=9, count=10 */
        for (int i = 0; i < 10; ++i) {
            sum += arr[i];
        }
    }
    return sum;
}

/* Mixed bounds with arithmetic */
__attribute__((noinline))
static int test_arithmetic_bounds(int *arr) {
    int sum = 0;
    /* Constant expressions as bounds */
    for (int i = 1+1; i < 3*3; ++i) {  /* lo=2, hi=8, count=7 */
        sum += arr[i];
    }
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int int_array[100] = {0};
    char char_array[100] = {0};
    v2si vec2 = {1, 2};
    v4si vec4 = {1, 2, 3, 4};
    v4sf vec4f = {1.0f, 2.0f, 3.0f, 4.0f};
    struct small_struct struct_array[10] = {{0}};
    
    int checksum = 0;
    
    /* Initialize arrays with some data */
    for (int i = 0; i < 100; ++i) {
        int_array[i] = i;
        char_array[i] = i % 128;
        if (i < 10) {
            struct_array[i].a = i * 2;
        }
    }
    
    /* Test all scenarios */
    checksum += test_small_count_1(int_array);
    checksum += test_small_count_2(int_array);
    
    v2si v2_result = test_small_vector(&vec2);
    checksum += v2_result[0] + v2_result[1];
    
    checksum += test_larger_slice(int_array);
    checksum += test_char_array(char_array);
    checksum += test_struct_array(struct_array);
    
    v4si v4_result = test_vector_shuffle(vec4, vec4);
    checksum += v4_result[0] + v4_result[1];
    
    v4si v4_constr = test_vector_constructor(vec4);
    checksum += v4_constr[0] + v4_constr[1];
    
    v4sf v4_perm = test_vector_permute(vec4f);
    checksum += (int)v4_perm[0];
    
    checksum += test_conditional_bounds(int_array);
    checksum += test_arithmetic_bounds(int_array);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
