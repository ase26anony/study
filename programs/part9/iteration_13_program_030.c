/* test_auto_inc_dec.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-trivial offsets */
struct TestStruct {
    int id;
    float value;
    double data;
    char padding[8];
};

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Should trigger post-increment load pattern */
    }
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;  /* Post-increment store */
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;  /* Post-decrement load */
    }
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value - i;  /* Post-decrement store */
    }
}

/* Simple pointer arithmetic with constant stride */
NOINLINE int test_int_stride4_load(int *arr) {
    int *ptr = arr;
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr += 4;
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Float post-increment load */
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + (float)i;  /* Float post-increment store */
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Double post-increment load */
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    volatile double *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + (double)i;  /* Double post-increment store */
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE double test_struct_traversal(struct TestStruct *arr) {
    struct TestStruct *ptr = arr;
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        /* Access different struct members - larger offsets */
        sum += ptr->value + ptr->data;
        ptr++;  /* Pointer increment by struct size */
    }
    return sum;
}

NOINLINE void test_struct_member_store(struct TestStruct *arr, int base) {
    struct TestStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        ptr->id = base + i;
        ptr->value = (float)(base + i) * 0.5f;
        ptr->data = (double)(base + i) * 0.25;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr2d[16][16]) {
    int *ptr = &arr2d[0][0];
    int sum = 0;
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;  /* Linear traversal of 2D array */
    }
    return sum;
}

NOINLINE void test_2d_array_store(int arr2d[16][16], int value) {
    int *ptr = &arr2d[0][0];
    for (int i = 0; i < 16 * 16; i++) {
        *ptr++ = value + i;  /* Linear store with pointer increment */
    }
}

/* ========== NESTED LOOP TESTS ========== */

NOINLINE int test_nested_loop_ptr(int *arr, int rows, int cols) {
    int sum = 0;
    for (int r = 0; r < rows; r++) {
        volatile int *vptr = &arr[r * cols];
        for (int c = 0; c < cols; c++) {
            sum += *vptr++;  /* Inner loop pointer reset each iteration */
        }
    }
    return sum;
}

NOINLINE void test_nested_loop_store(int *arr, int rows, int cols, int base) {
    for (int r = 0; r < rows; r++) {
        volatile int *vptr = &arr[r * cols];
        for (int c = 0; c < cols; c++) {
            *vptr++ = base + r * cols + c;  /* Inner loop store */
        }
    }
}

/* ========== MIXED ACCESS PATTERN TESTS ========== */

NOINLINE int test_mixed_access_patterns(int *arr1, float *arr2, double *arr3) {
    volatile int *iptr = arr1;
    volatile float *fptr = arr2;
    volatile double *dptr = arr3;
    int result = 0;
    
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        /* Mix of loads with post-increment */
        result += *iptr++;
        result += (int)(*fptr++ * 100.0f);
        result += (int)(*dptr++ * 100.0);
    }
    return result;
}

/* ========== COMPLEX POINTER ARITHMETIC ========== */

NOINLINE int test_complex_ptr_arithmetic(int *base, int offset) {
    int *ptr = base + offset;
    int sum = 0;
    
    /* Loop with pointer starting at non-zero offset */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += *ptr;
        ptr += 2;  /* Skip every other element */
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
    int array_2d[16][16];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = (float)i * 0.5f;
        double_array[i] = (double)i * 0.25;
        struct_array[i].id = i;
        struct_array[i].value = (float)i * 1.5f;
        struct_array[i].data = (double)i * 2.5;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Run all test functions multiple times to ensure optimization triggers */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_checksum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_checksum += test_int_postdec_load(int_array);
        test_int_postdec_store(int_array, iter);
        total_checksum += test_int_stride4_load(int_array);
        
        /* Float tests */
        total_checksum += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, (float)iter);
        
        /* Double tests */
        total_checksum += (int)test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, (double)iter);
        
        /* Structure tests */
        total_checksum += (int)test_struct_traversal(struct_array);
        test_struct_member_store(struct_array, iter);
        
        /* Multi-dimensional tests */
        total_checksum += test_2d_array_traversal(array_2d);
        test_2d_array_store(array_2d, iter);
        
        /* Nested loop tests */
        total_checksum += test_nested_loop_ptr(int_array, 8, 32);
        test_nested_loop_store(int_array, 8, 32, iter);
        
        /* Mixed pattern tests */
        total_checksum += test_mixed_access_patterns(int_array, float_array, double_array);
        
        /* Complex pointer arithmetic */
        total_checksum += test_complex_ptr_arithmetic(int_array, 16);
    }
    
    /* Final validation */
    printf("Final checksum: %d\n", total_checksum);
    printf("Array[0] = %d, Array[255] = %d\n", int_array[0], int_array[255]);
    printf("Float[127] = %f, Double[127] = %f\n", float_array[127], double_array[127]);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
