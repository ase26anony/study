/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_ACCESS volatile

/* Test structures for complex offset patterns */
struct TestStruct {
    int id;
    float data;
    double value;
    char padding[8];
};

/* ========== INTEGER ARRAY TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;
    }
    
    /* Verify by reading back */
    int sum = 0;
    ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_int_postdec_load(int *arr, int n) {
    int sum = 0;
    int *ptr = &arr[n - 1];
    
    /* Pattern: *ptr-- in loop - should trigger auto-dec */
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE int test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n - 1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;
    }
    
    /* Verify by reading back */
    int sum = 0;
    ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* ========== FLOATING POINT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    /* Pattern with float */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, int n, float base) {
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = base + (float)i * 0.5f;
    }
}

NOINLINE double test_double_postinc_load(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    /* Pattern with double */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, int n, double base) {
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = base + (double)i * 0.25;
    }
}

/* ========== VOLATILE ACCESS TESTS ========== */

NOINLINE int test_volatile_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *ptr = arr;
    
    /* Volatile access pattern - prevents reordering */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_volatile_postinc_store(volatile int *arr, int n) {
    volatile int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = i * 3;
    }
}

/* ========== STRUCT ARRAY TESTS ========== */

NOINLINE double test_struct_array_traversal(struct TestStruct *arr, int n) {
    double sum = 0.0;
    struct TestStruct *ptr = arr;
    
    /* Access struct members with constant offsets */
    for (int i = 0; i < n; i++) {
        sum += ptr->value + ptr->data;
        ptr->id = i;
        ptr++;
    }
    return sum;
}

NOINLINE int test_struct_pointer_arithmetic(struct TestStruct *arr, int n) {
    int sum = 0;
    
    /* Explicit pointer arithmetic with stride */
    for (int i = 0; i < n; i++) {
        struct TestStruct *s = arr + i;  /* Constant stride */
        sum += s->id;
    }
    return sum;
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr[][16], int rows) {
    int sum = 0;
    
    /* Row-major traversal with pointer */
    for (int i = 0; i < rows; i++) {
        int *ptr = arr[i];
        for (int j = 0; j < 16; j++) {
            sum += *ptr++;
        }
    }
    return sum;
}

NOINLINE int test_nested_loop_reset(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Inner loop pointer reset each iteration */
    for (int i = 0; i < rows; i++) {
        int *ptr = arr + i * cols;
        for (int j = 0; j < cols; j++) {
            sum += *ptr++;
        }
    }
    return sum;
}

/* ========== CONSTANT STRIDE PATTERNS ========== */

NOINLINE int test_constant_stride(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access with constant stride of 4 ints */
    for (int i = 0; i < n; i += 4) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;
    }
    return sum;
}

NOINLINE int test_mixed_offset_patterns(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Mixed offsets within loop */
    for (int i = 0; i < n; i++) {
        sum += ptr[0];      /* Zero offset */
        sum += ptr[1];      /* Constant offset 1 */
        sum += ptr[4];      /* Constant offset 4 */
        ptr += 1;
    }
    return sum;
}

/* ========== COMPLEX LOOP PATTERNS ========== */

NOINLINE int test_loop_unroll_candidate(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Fixed iteration count for potential unrolling */
    for (int i = 0; i < 64; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_multiple_pointers_same_array(int *arr, int n) {
    int sum = 0;
    int *read_ptr = arr;
    int *write_ptr = arr;
    
    /* Two pointers advancing through same array */
    for (int i = 0; i < n; i++) {
        int val = *read_ptr++;
        *write_ptr++ = val * 2;
        sum += val;
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct TestStruct *struct_array = (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    volatile int *volatile_array = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    int matrix[8][16];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = i * 1.5f;
        double_array[i] = i * 2.5;
        struct_array[i].id = i;
        struct_array[i].data = i * 0.75f;
        struct_array[i].value = i * 1.25;
        volatile_array[i] = i * 2;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    int total_sum = 0;
    
    /* Execute all test patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_array, ARRAY_SIZE);
        total_sum += test_int_postinc_store(int_array, ARRAY_SIZE, iter);
        total_sum += test_int_postdec_load(int_array, ARRAY_SIZE);
        total_sum += test_int_postdec_store(int_array, ARRAY_SIZE, iter);
        
        total_sum += (int)test_float_postinc_load(float_array, ARRAY_SIZE);
        test_float_postinc_store(float_array, ARRAY_SIZE, iter * 0.5f);
        
        total_sum += (int)test_double_postinc_load(double_array, ARRAY_SIZE);
        test_double_postinc_store(double_array, ARRAY_SIZE, iter * 0.25);
        
        total_sum += test_volatile_postinc(volatile_array, ARRAY_SIZE);
        test_volatile_postinc_store(volatile_array, ARRAY_SIZE);
        
        total_sum += (int)test_struct_array_traversal(struct_array, ARRAY_SIZE);
        total_sum += test_struct_pointer_arithmetic(struct_array, ARRAY_SIZE);
        
        total_sum += test_2d_array_traversal(matrix, 8);
        total_sum += test_nested_loop_reset(int_array, 16, 16);
        
        total_sum += test_constant_stride(int_array, ARRAY_SIZE);
        total_sum += test_mixed_offset_patterns(int_array, ARRAY_SIZE);
        
        total_sum += test_loop_unroll_candidate(int_array, 64);
        total_sum += test_multiple_pointers_same_array(int_array, ARRAY_SIZE);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    free((void*)volatile_array);
    
    return 0;
}
