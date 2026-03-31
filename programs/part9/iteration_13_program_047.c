/* test_auto_inc_dec.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets specific uncovered lines in auto-inc-dec.cc:1352-1358
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop structure */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-trivial offsets */
struct test_struct {
    int id;
    float value;
    double data;
    char padding[8];
};

/* Volatile helper to prevent elimination */
static volatile int volatile_sink;

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: *ptr++ with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;  /* Should generate mem_loc with reg0 = ptr, offset 0 */
    }
    return sum;
}

NOINLINE int test_int_postinc_store(int *arr, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
    
    /* Verify */
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    return sum;
}

NOINLINE int test_int_postdec_load(int *arr) {
    int sum = 0;
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE int test_int_postdec_store(int *arr, int value) {
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- = value */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
    
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE float test_float_postinc_store(float *arr, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + (float)i;
    }
    
    float sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    return sum;
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE double test_double_postinc_store(double *arr, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + (double)i;
    }
    
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    return sum;
}

/* ========== VOLATILE TESTS ========== */

NOINLINE int test_volatile_postinc(volatile int *arr) {
    int sum = 0;
    volatile int *ptr = arr;
    
    /* Volatile access should still trigger the pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
        volatile_sink = sum; /* Prevent elimination */
    }
    return sum;
}

/* ========== STRUCT TESTS ========== */

NOINLINE double test_struct_traversal(struct test_struct *arr) {
    double sum = 0.0;
    struct test_struct *ptr = arr;
    
    /* Accessing same member across array - larger offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->data;
        ptr++;  /* ptr increment by struct size */
    }
    return sum;
}

NOINLINE void test_struct_member_store(struct test_struct *arr, double value) {
    struct test_struct *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->data = value + i;
        ptr->value = (float)(value + i);
        ptr->id = i;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

#define ROWS 16
#define COLS 16

NOINLINE int test_2d_array_traversal(int matrix[ROWS][COLS]) {
    int sum = 0;
    int *ptr = &matrix[0][0];
    
    /* Single pointer traversing 2D array */
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loop_pointer(int matrix[ROWS][COLS]) {
    int sum = 0;
    
    /* Nested loops with pointer reset */
    for (int row = 0; row < ROWS; row++) {
        int *ptr = matrix[row];
        for (int col = 0; col < COLS; col++) {
            sum += *ptr++;
        }
    }
    return sum;
}

/* ========== CONSTANT STRIDE TESTS ========== */

NOINLINE int test_constant_stride(int *arr, int stride) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pointer arithmetic with constant stride */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;  /* Constant stride */
    }
    return sum;
}

/* ========== MIXED PATTERNS ========== */

NOINLINE int test_mixed_increment_patterns(int *arr1, int *arr2) {
    int sum = 0;
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    /* Mixed pre/post operations */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr1++;      /* post-increment */
        sum += *++ptr2;      /* pre-increment */
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
    int matrix[ROWS][COLS];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = (float)i;
        double_array[i] = (double)i;
        struct_array[i].id = i;
        struct_array[i].value = (float)i;
        struct_array[i].data = (double)i;
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Run all tests multiple times to ensure pattern recognition */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_checksum += test_int_postinc_load(int_array);
        total_checksum += test_int_postinc_store(int_array, iter);
        total_checksum += test_int_postdec_load(int_array);
        total_checksum += test_int_postdec_store(int_array, iter);
        
        total_checksum += (int)test_float_postinc_load(float_array);
        total_checksum += (int)test_float_postinc_store(float_array, (float)iter);
        
        total_checksum += (int)test_double_postinc_load(double_array);
        total_checksum += (int)test_double_postinc_store(double_array, (double)iter);
        
        total_checksum += test_volatile_postinc(int_array);
        
        total_checksum += (int)test_struct_traversal(struct_array);
        test_struct_member_store(struct_array, (double)iter);
        
        total_checksum += test_2d_array_traversal(matrix);
        total_checksum += test_nested_loop_pointer(matrix);
        
        total_checksum += test_constant_stride(int_array, 4);
        total_checksum += test_mixed_increment_patterns(int_array, int_array);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
