/* Test case for GCC expr.cc constant bounds analysis coverage.
   Specifically targeting lines 7691-7700 in expr.cc.
   Compile with: gcc -O2 -fprofile-arcs -ftest-coverage -fdump-tree-ccp1 -fdump-tree-forwprop1 -ftree-vectorize -fdump-tree-vect -o test_expr test_expr.c
*/

#include <stdio.h>
#include <stdlib.h>

typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent constant folding */
volatile int vol_zero = 0;
volatile int vol_one = 1;
volatile int vol_two = 2;

/* ========== SCENARIO 1: Small count (count <= 2) memory references ========== */

/* Single element access - count = 1 */
static void __attribute__((noinline))
test_small_count_1(int *arr, int *out) {
    /* arr[5] is constant bounded single element */
    *out = arr[5];
}

/* Two adjacent elements - count = 2 */
static void __attribute__((noinline))
test_small_count_2(int *arr, int *out1, int *out2) {
    /* arr[2] and arr[3] form a constant bounded section of 2 elements */
    *out1 = arr[2];
    *out2 = arr[3];
}

/* Vector element access - count = 2 via vector type */
static void __attribute__((noinline))
test_small_count_vector(v2si *vec, int *out) {
    /* Accessing vec[0][0] and vec[0][1] - 2 elements */
    v2si v = *vec;
    *out = v[0] + v[1];
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Fixed-size array slice with constant bounds */
static void __attribute__((noinline))
test_larger_constant_mem(int *arr, int *out) {
    int sum = 0;
    /* Loop with constant bounds: i from 2 to 9 inclusive -> count = 8 */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* Larger access with char type - TYPE_SIZE(elttype) = 8 bits */
static void __attribute__((noinline))
test_larger_constant_char(char *arr, int *out) {
    int sum = 0;
    /* 32 char elements -> 32 * 8 = 256 bits fits in unsigned HWI */
    for (int i = 0; i < 32; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* Mixed bounds using volatile to preserve analysis */
static void __attribute__((noinline))
test_mixed_bounds(int *arr, int *out) {
    int sum = 0;
    /* Use volatile in condition to prevent elimination */
    int start = vol_zero ? 5 : 10;  /* Compiler sees both 5 and 10 */
    int end = vol_one ? 15 : 20;    /* Compiler sees both 15 and 20 */
    
    /* But at runtime, we get constant bounds: 5 to 14 -> count = 10 */
    if (vol_zero == 0) {
        for (int i = start; i < end; ++i) {
            sum += arr[i];
        }
    }
    *out = sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */

/* Vector shuffle with constant indices - not a direct memory load */
static v4si __attribute__((noinline))
test_vector_shuffle(v4si a, v4si b) {
    /* Create a new vector from constant indices */
    return __builtin_shufflevector(a, b, 0, 1, 4, 5);
}

/* Vector compound literal with constant indices */
static v4si __attribute__((noinline))
test_vector_constructor(v4si v) {
    /* Extract constant-bounded section via constructor */
    return (v4si){v[0], v[1], v[2], v[3]};
}

/* Vector permute with constant mask */
static v4sf __attribute__((noinline))
test_vector_permute(v4sf a) {
    /* Reverse the vector using constant indices */
    return __builtin_shufflevector(a, a, 3, 2, 1, 0);
}

/* ========== MAIN FUNCTION ========== */

int main() {
    int arr[100];
    char carr[100];
    v2si vec2;
    v4si vec4;
    v4sf vec4f;
    
    /* Initialize data */
    for (int i = 0; i < 100; ++i) {
        arr[i] = i;
        carr[i] = i % 128;
    }
    
    vec2 = (v2si){10, 20};
    vec4 = (v4si){1, 2, 3, 4};
    vec4f = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    
    int checksum = 0;
    int out1, out2, out3, out4, out5;
    
    /* Test Scenario 1: Small count memory references */
    test_small_count_1(arr, &out1);
    checksum += out1;
    
    test_small_count_2(arr, &out1, &out2);
    checksum += out1 + out2;
    
    test_small_count_vector(&vec2, &out1);
    checksum += out1;
    
    /* Test Scenario 2: Larger constant-sized memory access */
    test_larger_constant_mem(arr, &out1);
    checksum += out1;
    
    test_larger_constant_char(carr, &out1);
    checksum += out1;
    
    test_mixed_bounds(arr, &out1);
    checksum += out1;
    
    /* Test Scenario 3: Non-memory vector operations */
    v4si shuffled = test_vector_shuffle(vec4, (v4si){5, 6, 7, 8});
    checksum += shuffled[0] + shuffled[1] + shuffled[2] + shuffled[3];
    
    v4si constructed = test_vector_constructor(vec4);
    checksum += constructed[0] + constructed[1] + constructed[2] + constructed[3];
    
    v4sf permuted = test_vector_permute(vec4f);
    checksum += (int)permuted[0] + (int)permuted[1] + (int)permuted[2] + (int)permuted[3];
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
