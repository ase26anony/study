/* auto_inc_dec_test.c - Comprehensive test for GCC auto-increment/decrement optimization */
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
    int id;
    float value;
    double data;
    char padding[8];
} DataStruct;

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 2: Non-volatile pointer with stride */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        sum += *ptr;
        ptr += 4;  /* Constant stride */
    }
    
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    
    /* Pattern 3: Post-increment store with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 4: Post-decrement store */
    int *ptr = &arr[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    int *ptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Pattern 5: Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Pattern 6: Float post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 7: Float with pointer arithmetic */
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + i);  /* Base + index pattern */
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    /* Pattern 8: Float post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Pattern 9: Double post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 10: Double post-decrement */
    double *ptr = &arr[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    double *ptr = arr;
    
    /* Pattern 11: Double store with constant stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr = value + i;
        ptr += 1;  /* Explicit increment */
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE double test_struct_traversal(DataStruct *arr) {
    double sum = 0.0;
    
    /* Pattern 12: Structure member access with pointer */
    DataStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->value + ptr->data;
        ptr++;  /* Large offset increment */
    }
    
    /* Pattern 13: Direct member access with index */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i].data;  /* Non-trivial offset calculation */
    }
    
    return sum;
}

NOINLINE void test_struct_store(DataStruct *arr) {
    DataStruct *ptr = arr;
    
    /* Pattern 14: Structure store with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = i;
        ptr->value = i * 1.5f;
        ptr->data = i * 2.5;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

#define ROWS 16
#define COLS 16

NOINLINE int test_2d_array_traversal(int matrix[ROWS][COLS]) {
    int sum = 0;
    
    /* Pattern 15: 2D array traversal with single pointer */
    int *ptr = &matrix[0][0];
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    
    /* Pattern 16: Nested loops with pointer reset */
    for (int i = 0; i < ROWS; i++) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < COLS; j++) {
            sum += *row_ptr++;
        }
    }
    
    return sum;
}

NOINLINE void test_2d_array_store(int matrix[ROWS][COLS]) {
    int *ptr = &matrix[0][0];
    
    /* Pattern 17: 2D array store with post-increment */
    for (int i = 0; i < ROWS * COLS; i++) {
        *ptr++ = i;
    }
}

/* ========== COMPLEX PATTERN TESTS ========== */

NOINLINE int test_mixed_increment_patterns(int *arr1, int *arr2) {
    int sum = 0;
    
    /* Pattern 18: Mixed increment patterns in same loop */
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr1++;
        sum += *ptr2++;
        
        if (i % 2 == 0) {
            sum += *ptr1;  /* No increment here */
        }
    }
    
    return sum;
}

NOINLINE void test_pointer_arithmetic_with_constants(int *arr) {
    /* Pattern 19: Explicit pointer arithmetic */
    int *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(ptr + 0) = i;      /* Zero offset */
        *(ptr + 1) = i * 2;  /* Constant offset */
        ptr += 2;
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    DataStruct *struct_array = (DataStruct*)malloc(ARRAY_SIZE * sizeof(DataStruct));
    int matrix[ROWS][COLS];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = i * 1.0f;
        double_array[i] = i * 2.0;
        struct_array[i].id = i;
        struct_array[i].value = i * 3.0f;
        struct_array[i].data = i * 4.0;
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
    
    int total_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* Run tests multiple times to ensure loops are executed */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_sum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_sum += test_int_postdec_load(int_array);
        
        /* Float tests */
        float_sum += test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, iter * 1.0f);
        
        /* Double tests */
        double_sum += test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, iter * 2.0);
        
        /* Structure tests */
        double_sum += test_struct_traversal(struct_array);
        test_struct_store(struct_array);
        
        /* Multi-dimensional tests */
        total_sum += test_2d_array_traversal(matrix);
        test_2d_array_store(matrix);
        
        /* Complex pattern tests */
        total_sum += test_mixed_increment_patterns(int_array, int_array + ARRAY_SIZE/2);
        test_pointer_arithmetic_with_constants(int_array);
    }
    
    /* Verify results (simplified checks) */
    printf("Integer checksum: %d\n", total_sum);
    printf("Float checksum: %f\n", float_sum);
    printf("Double checksum: %f\n", double_sum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
