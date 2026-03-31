/* test_auto_inc_dec.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-unit strides */
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

/* Pointer arithmetic with constant stride */
NOINLINE int test_int_stride4_load(int *arr) {
    int *ptr = arr;
    int sum = 0;
    /* Access every 4th element using pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr += 4;           /* Explicit stride */
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

NOINLINE float test_float_postdec_load(float *arr) {
    volatile float *vptr = &arr[ARRAY_SIZE - 1];
    float sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;  /* Float post-decrement load */
    }
    return sum;
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

NOINLINE int test_struct_array_load(struct test_struct *arr) {
    struct test_struct *ptr = arr;
    int sum = 0;
    /* Access struct members with non-unit stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->val;    /* Access through pointer */
        ptr++;              /* Pointer increment by struct size */
    }
    return sum;
}

NOINLINE void test_struct_array_store(struct test_struct *arr, int base) {
    struct test_struct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->val = base + i;
        ptr->fval = (float)(base + i);
        ptr->dval = (double)(base + i);
        ptr++;  /* Post-increment by struct size */
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr2d[16][16]) {
    int *ptr = &arr2d[0][0];
    int sum = 0;
    /* Row-major traversal with single pointer */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_2d_array_store(int arr2d[16][16], int value) {
    int *ptr = &arr2d[0][0];
    for (int i = 0; i < 16 * 16; i++) {
        *ptr++ = value + i;
    }
}

/* ========== NESTED LOOP TESTS ========== */

NOINLINE int test_nested_loop_ptr(int *arr, int rows, int cols) {
    int sum = 0;
    for (int r = 0; r < rows; r++) {
        volatile int *vptr = &arr[r * cols];
        /* Inner loop with pointer reset each iteration */
        for (int c = 0; c < cols; c++) {
            sum += *vptr++;  /* Should recognize pattern within inner loop */
        }
    }
    return sum;
}

NOINLINE void test_nested_loop_store(int *arr, int rows, int cols, int base) {
    for (int r = 0; r < rows; r++) {
        volatile int *vptr = &arr[r * cols];
        for (int c = 0; c < cols; c++) {
            *vptr++ = base + r * cols + c;
        }
    }
}

/* ========== MIXED PATTERN TESTS ========== */

NOINLINE int test_mixed_access_patterns(int *arr1, float *arr2, double *arr3) {
    volatile int *iptr = arr1;
    volatile float *fptr = arr2;
    volatile double *dptr = arr3;
    int isum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Interleaved access patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        isum += *iptr++;
        fsum += *fptr++;
        dsum += *dptr++;
    }
    
    return isum + (int)fsum + (int)dsum;
}

/* ========== COMPLEX POINTER ARITHMETIC ========== */

NOINLINE int test_complex_ptr_expr(int *arr, int offset) {
    int *ptr = arr + offset;
    int sum = 0;
    /* Start from offset, traverse with post-increment */
    for (int i = 0; i < ARRAY_SIZE - offset; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
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
        float_array[i] = (float)i * 1.5f;
        double_array[i] = (double)i * 2.5;
        struct_array[i].val = i * 3;
        struct_array[i].fval = (float)i * 4.0f;
        struct_array[i].dval = (double)i * 5.0;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Execute all test functions multiple times to ensure patterns are exercised */
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
        total_checksum += (int)test_float_postdec_load(float_array);
        
        /* Double tests */
        total_checksum += (int)test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, (double)iter);
        
        /* Structure tests */
        total_checksum += test_struct_array_load(struct_array);
        test_struct_array_store(struct_array, iter);
        
        /* Multi-dimensional tests */
        total_checksum += test_2d_array_traversal(array_2d);
        test_2d_array_store(array_2d, iter);
        
        /* Nested loop tests */
        total_checksum += test_nested_loop_ptr(int_array, 16, 16);
        test_nested_loop_store(int_array, 16, 16, iter);
        
        /* Mixed pattern tests */
        total_checksum += test_mixed_access_patterns(int_array, float_array, double_array);
        
        /* Complex pointer arithmetic */
        total_checksum += test_complex_ptr_expr(int_array, 16);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Total checksum: %d\n", total_checksum);
    
    /* Verify some values to ensure correctness */
    printf("Verification: int_array[0]=%d, float_array[0]=%.2f, struct_array[0].val=%d\n",
           int_array[0], float_array[0], struct_array[0].val);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
