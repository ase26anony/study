/* auto_inc_dec_test.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-trivial offsets */
struct test_struct {
    int val;
    float fval;
    double dval;
    char padding[8];
};

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Simple post-increment load with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Non-volatile version */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    
    /* Post-increment store with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Non-volatile version */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - i;
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = arr + ARRAY_SIZE - 1;
    int sum = 0;
    
    /* Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = arr + ARRAY_SIZE - 1;
    
    /* Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i;
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Float post-increment load */
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
    
    /* Float post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + (float)i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Double post-increment load */
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
    
    /* Double post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + (double)i;
    }
}

/* ========== CONSTANT STRIDE TESTS ========== */

NOINLINE int test_const_stride(int *arr) {
    int *ptr = arr;
    int sum = 0;
    
    /* Pointer arithmetic with constant stride */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;
    }
    
    return sum;
}

/* ========== STRUCTURE ARRAY TESTS ========== */

NOINLINE double test_struct_traversal(struct test_struct *arr) {
    volatile struct test_struct *vptr = arr;
    double sum = 0.0;
    
    /* Access multiple members with pointer increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr->val;
        sum += vptr->fval;
        sum += vptr->dval;
        vptr++;
    }
    
    return sum;
}

NOINLINE void test_struct_store(struct test_struct *arr) {
    volatile struct test_struct *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vptr->val = i;
        vptr->fval = (float)i;
        vptr->dval = (double)i;
        vptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr2d[16][16]) {
    int *ptr = &arr2d[0][0];
    int sum = 0;
    
    /* Single pointer traversal of 2D array */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE int test_nested_loops(int arr[16][16]) {
    int sum = 0;
    
    /* Nested loops with pointer reset */
    for (int i = 0; i < 16; i++) {
        volatile int *row_ptr = arr[i];
        for (int j = 0; j < 16; j++) {
            sum += *row_ptr++;
        }
    }
    
    return sum;
}

/* ========== MIXED PATTERN TESTS ========== */

NOINLINE int test_mixed_patterns(int *arr1, float *arr2, double *arr3) {
    volatile int *vptr1 = arr1;
    volatile float *vptr2 = arr2;
    volatile double *vptr3 = arr3;
    int result = 0;
    
    /* Mixed types in same loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        result += *vptr1++;
        result += (int)(*vptr2++);
        result += (int)(*vptr3++);
    }
    
    return result;
}

/* ========== COMPLEX POINTER ARITHMETIC ========== */

NOINLINE int test_complex_arithmetic(int *base) {
    int sum = 0;
    int *ptr = base;
    
    /* Complex but predictable pattern */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += ptr[0];      /* Base + 0 offset */
        sum += ptr[1];      /* Base + 1 offset */
        ptr += 2;           /* Post-increment by 2 */
        
        /* Additional operation to prevent over-optimization */
        if (sum & 1) {
            sum ^= 0x5555;
        }
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct test_struct *struct_array = 
        (struct test_struct*)malloc(ARRAY_SIZE * sizeof(struct test_struct));
    int array_2d[16][16];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = (float)i;
        double_array[i] = (double)i;
        struct_array[i].val = i;
        struct_array[i].fval = (float)i;
        struct_array[i].dval = (double)i;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_result = 0;
    
    /* Run all tests multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_result += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_result += test_int_postdec_load(int_array);
        test_int_postdec_store(int_array, iter);
        
        total_result += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, (float)iter);
        
        total_result += (int)test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, (double)iter);
        
        total_result += test_const_stride(int_array);
        total_result += (int)test_struct_traversal(struct_array);
        test_struct_store(struct_array);
        
        total_result += test_2d_array_traversal(array_2d);
        total_result += test_nested_loops(array_2d);
        total_result += test_mixed_patterns(int_array, float_array, double_array);
        total_result += test_complex_arithmetic(int_array);
        
        /* Prevent compiler from optimizing everything away */
        if (total_result > 0x7FFFFFFF) {
            total_result = 0;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %d\n", total_result);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
