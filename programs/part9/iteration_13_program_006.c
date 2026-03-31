/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-trivial offsets */
typedef struct {
    int val;
    float fval;
    double dval;
    char padding[32];  /* Force larger stride */
} TestStruct;

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 2: Non-volatile pointer with post-increment */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    
    /* Pattern 3: Post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 4: Non-volatile post-increment store */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - i;
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = arr + ARRAY_SIZE - 1;
    int sum = 0;
    
    /* Pattern 5: Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = arr + ARRAY_SIZE - 1;
    
    /* Pattern 6: Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i * 2;
    }
}

NOINLINE int test_int_constant_stride(int *arr) {
    int sum = 0;
    
    /* Pattern 7: Pointer arithmetic with constant stride */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr += 4;
        if (ptr >= arr + ARRAY_SIZE) break;
    }
    
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * i;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value / (i + 1);
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    volatile double *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * sin(i * 0.01);
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE int test_struct_traversal(TestStruct *arr) {
    int sum = 0;
    
    /* Pattern 8: Structure member access with pointer arithmetic */
    TestStruct *ptr = arr;
    
    /* Access different members in sequence */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->val;
        ptr++;
    }
    
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)ptr->fval;
        ptr++;
    }
    
    return sum;
}

NOINLINE void test_struct_store(TestStruct *arr, int base) {
    TestStruct *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->val = base + i;
        ptr->fval = (float)(base + i) * 0.5f;
        ptr->dval = (double)(base + i) * 0.25;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int (*arr2d)[16]) {
    int sum = 0;
    
    /* Pattern 9: 2D array traversal with single pointer */
    int *ptr = &arr2d[0][0];
    
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE int test_nested_loop_reset(int *arr) {
    int sum = 0;
    
    /* Pattern 10: Nested loops with pointer reset */
    for (int outer = 0; outer < 4; outer++) {
        int *ptr = arr + outer * 64;
        
        for (int inner = 0; inner < 64; inner++) {
            sum += *ptr++;
        }
    }
    
    return sum;
}

/* ========== MIXED PATTERN TESTS ========== */

NOINLINE int test_mixed_increment_patterns(int *arr) {
    int sum = 0;
    volatile int *vptr = arr;
    
    /* Mix of pre and post operations */
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += *vptr;      /* Load without increment */
        vptr++;
        sum += *vptr;      /* Load current */
        vptr++;            /* Post increment */
    }
    
    return sum;
}

NOINLINE void test_pointer_arithmetic_combinations(int *arr) {
    int *ptr1 = arr;
    int *ptr2 = arr + ARRAY_SIZE/2;
    
    /* Multiple pointers with different update patterns */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        *ptr1++ = i;
        *ptr2++ = i * 2;
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    TestStruct *struct_array = (TestStruct*)malloc(ARRAY_SIZE * sizeof(TestStruct));
    int array_2d[16][16];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 7;
        float_array[i] = (float)i * 1.5f;
        double_array[i] = (double)i * 2.5;
        struct_array[i].val = i * 4;
        struct_array[i].fval = (float)i * 0.75f;
        struct_array[i].dval = (double)i * 1.25;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_sum = 0;
    
    /* Execute all test patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_sum += test_int_postdec_load(int_array);
        test_int_postdec_store(int_array, iter * 2);
        total_sum += test_int_constant_stride(int_array);
        
        total_sum += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, (float)iter);
        
        total_sum += (int)test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, (double)iter);
        
        total_sum += test_struct_traversal(struct_array);
        test_struct_store(struct_array, iter * 3);
        
        total_sum += test_2d_array_traversal(array_2d);
        total_sum += test_nested_loop_reset(int_array);
        total_sum += test_mixed_increment_patterns(int_array);
        test_pointer_arithmetic_combinations(int_array);
    }
    
    /* Verification and output to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    printf("Sample values: int[0]=%d, float[100]=%.2f, struct[50].val=%d\n",
           int_array[0], float_array[100], struct_array[50].val);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
