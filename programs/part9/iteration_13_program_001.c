/* test_auto_inc_dec.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets specific uncovered lines in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 256
#define CHECKSUM_SEED 0x12345678

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-trivial offsets */
struct TestStruct {
    int id;
    float value;
    double data;
    char tag;
    int padding[3]; /* Ensure size is multiple of 8 for alignment */
};

/* Global arrays to prevent complete optimization */
static int int_array[ARRAY_SIZE];
static float float_array[ARRAY_SIZE];
static double double_array[ARRAY_SIZE];
static struct TestStruct struct_array[ARRAY_SIZE];

/* ========== INTEGER TESTS ========== */

/* Post-increment load with simple pointer */
NOINLINE int test_int_postinc_load(int *arr, int size) {
    volatile int *vptr = arr; /* volatile to prevent elimination */
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        sum += *vptr++;  /* Should generate post-increment load */
    }
    return sum;
}

/* Post-increment store with simple pointer */
NOINLINE void test_int_postinc_store(int *arr, int size, int value) {
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        *ptr++ = value + i;  /* Should generate post-increment store */
    }
}

/* Post-decrement load */
NOINLINE int test_int_postdec_load(int *arr, int size) {
    volatile int *vptr = &arr[size - 1];
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        sum += *vptr--;  /* Should generate post-decrement load */
    }
    return sum;
}

/* Post-decrement store */
NOINLINE void test_int_postdec_store(int *arr, int size, int value) {
    int *ptr = &arr[size - 1];
    
    for (int i = 0; i < size; i++) {
        *ptr-- = value - i;  /* Should generate post-decrement store */
    }
}

/* Pointer arithmetic with constant stride */
NOINLINE int test_int_stride_load(int *arr, int size) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr += 8;  /* Non-unit stride */
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr, int size) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < size; i++) {
        sum += *vptr++;  /* Float post-increment load */
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, int size, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        *ptr++ = value + (float)i;  /* Float post-increment store */
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr, int size) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    for (int i = 0; i < size; i++) {
        sum += *vptr++;  /* Double post-increment load */
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, int size, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        *ptr++ = value + (double)i;  /* Double post-increment store */
    }
}

/* ========== STRUCTURE TESTS ========== */

/* Access structure members with non-trivial offsets */
NOINLINE int test_struct_traversal(struct TestStruct *arr, int size) {
    int sum = 0;
    struct TestStruct *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        sum += ptr->id;        /* Offset 0 */
        sum += (int)ptr->value; /* Offset 4 */
        ptr++;                 /* Post-increment with large stride */
    }
    return sum;
}

NOINLINE void test_struct_store(struct TestStruct *arr, int size) {
    struct TestStruct *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        ptr->id = i;
        ptr->value = (float)i * 1.5f;
        ptr->data = (double)i * 2.5;
        ptr->tag = 'A' + (i % 26);
        ptr++;  /* Post-increment with complex structure */
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

#define ROWS 16
#define COLS 16

NOINLINE int test_2d_array_traversal(int matrix[ROWS][COLS]) {
    int sum = 0;
    int *ptr = &matrix[0][0];  /* Flatten to single pointer */
    
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;  /* Linear traversal of 2D array */
    }
    return sum;
}

NOINLINE void test_2d_array_store(int matrix[ROWS][COLS]) {
    int *ptr = &matrix[0][0];
    int value = 1;
    
    for (int i = 0; i < ROWS * COLS; i++) {
        *ptr++ = value++;  /* Post-increment store in 2D array */
    }
}

/* ========== NESTED LOOP TESTS ========== */

NOINLINE int test_nested_loop_traversal(int *arr, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *ptr = &arr[i * cols];  /* Reset pointer each outer iteration */
        
        for (int j = 0; j < cols; j++) {
            sum += *ptr++;  /* Inner loop post-increment */
        }
    }
    return sum;
}

/* ========== MIXED PATTERNS ========== */

/* Mix load and store in same loop */
NOINLINE void test_mixed_load_store(int *src, int *dst, int size) {
    volatile int *vsrc = src;
    int *pdst = dst;
    
    for (int i = 0; i < size; i++) {
        *pdst++ = *vsrc++;  /* Both load and store with post-increment */
    }
}

/* Test with different increment amounts */
NOINLINE int test_variable_increment(int *arr, int size, int step) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        sum += *ptr;
        ptr += step;  /* Variable increment (but constant in loop) */
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    int checksum = CHECKSUM_SEED;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (float)(i + 1) * 1.1f;
        double_array[i] = (double)(i + 1) * 1.5;
        struct_array[i].id = i;
        struct_array[i].value = (float)i * 2.0f;
        struct_array[i].data = (double)i * 3.0;
        struct_array[i].tag = 'A' + (i % 26);
    }
    
    /* Integer tests */
    checksum ^= test_int_postinc_load(int_array, ARRAY_SIZE);
    test_int_postinc_store(int_array, ARRAY_SIZE, 100);
    checksum ^= test_int_postdec_load(int_array, ARRAY_SIZE);
    test_int_postdec_store(int_array, ARRAY_SIZE, 200);
    checksum ^= test_int_stride_load(int_array, ARRAY_SIZE / 8);
    
    /* Float tests */
    checksum ^= (int)test_float_postinc_load(float_array, ARRAY_SIZE);
    test_float_postinc_store(float_array, ARRAY_SIZE, 50.0f);
    
    /* Double tests */
    checksum ^= (int)test_double_postinc_load(double_array, ARRAY_SIZE);
    test_double_postinc_store(double_array, ARRAY_SIZE, 75.0);
    
    /* Structure tests */
    checksum ^= test_struct_traversal(struct_array, ARRAY_SIZE);
    test_struct_store(struct_array, ARRAY_SIZE);
    
    /* Multi-dimensional array test */
    int matrix[ROWS][COLS];
    test_2d_array_store(matrix);
    checksum ^= test_2d_array_traversal(matrix);
    
    /* Nested loop test */
    checksum ^= test_nested_loop_traversal(int_array, 16, 16);
    
    /* Mixed pattern test */
    int src_array[ARRAY_SIZE];
    int dst_array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) src_array[i] = i * 2;
    test_mixed_load_store(src_array, dst_array, ARRAY_SIZE);
    
    /* Variable increment test */
    checksum ^= test_variable_increment(int_array, ARRAY_SIZE / 2, 2);
    
    /* Final validation */
    printf("Final checksum: 0x%08x\n", checksum);
    
    /* Use results to prevent dead code elimination */
    volatile int result = checksum;
    if (result != CHECKSUM_SEED) {
        printf("Validation passed (checksum modified by operations)\n");
    }
    
    return 0;
}
