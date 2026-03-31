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
struct TestStruct {
    int val;
    float fval;
    double dval;
    char padding[16]; /* Force larger stride */
};

/* ====== INTEGER TESTS ====== */

NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load with volatile */
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
    
    /* Pattern 1: Volatile post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 2: Non-volatile post-increment store */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - i;
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i;
    }
}

/* ====== FLOAT TESTS ====== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Multiple patterns in one function */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Post-increment load */
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;   /* Another post-increment */
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * i;  /* Post-increment store */
    }
}

/* ====== DOUBLE TESTS ====== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Test with constant stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr;
        vptr = vptr + 1;  /* Different pattern - pointer arithmetic */
    }
    
    /* Classic post-increment */
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postdec_store(double *arr, double value) {
    volatile double *vptr = &arr[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value / (i + 1);  /* Post-decrement store */
    }
}

/* ====== STRUCTURE TESTS ====== */

NOINLINE double test_struct_traversal(struct TestStruct *arr) {
    double sum = 0.0;
    
    /* Access different members with pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i].dval;  /* Array indexing - may convert to pointer */
    }
    
    /* Direct pointer traversal */
    struct TestStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->val;
        sum += ptr->fval;
        ptr++;  /* Post-increment of struct pointer */
    }
    
    return sum;
}

/* ====== MULTI-DIMENSIONAL ARRAY TESTS ====== */

NOINLINE int test_2d_array_traversal(int arr[16][16]) {
    int sum = 0;
    
    /* Row-major traversal with single pointer */
    int *ptr = &arr[0][0];
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    
    /* Nested loops with pointer reset */
    for (int i = 0; i < 16; i++) {
        int *row_ptr = arr[i];
        for (int j = 0; j < 16; j++) {
            sum += *row_ptr++;
        }
    }
    
    return sum;
}

/* ====== MIXED PATTERNS WITH OFFSETS ====== */

NOINLINE int test_mixed_offsets(int *arr) {
    int sum = 0;
    
    /* Pattern with constant offset */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr++;  /* Separate increment */
    }
    
    /* Zero offset pattern (should match uncovered lines) */
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr;  /* Zero offset access */
        vptr = vptr + 1;  /* Separate pointer arithmetic */
    }
    
    return sum;
}

/* ====== COMPLEX LOOP PATTERNS ====== */

NOINLINE void test_complex_pattern(int *arr1, int *arr2) {
    /* Two arrays with interleaved access */
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr1++ = *ptr2++;  /* Both load and store with post-increment */
    }
    
    /* Reverse copy */
    ptr1 = &arr1[ARRAY_SIZE - 1];
    ptr2 = &arr2[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr1-- = *ptr2--;  /* Both post-decrement */
    }
}

/* ====== MAIN DRIVER ====== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct TestStruct *struct_arr = (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    int multi_arr[16][16];
    
    if (!int_arr || !float_arr || !double_arr || !struct_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].val = i;
        struct_arr[i].fval = i * 3.5f;
        struct_arr[i].dval = i * 4.5;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_arr[i][j] = i * 16 + j;
        }
    }
    
    int total_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* Run tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_sum += test_int_postinc_load(int_arr);
        test_int_postinc_store(int_arr, iter);
        total_sum += test_int_postdec_load(int_arr);
        test_int_postdec_store(int_arr, iter);
        
        /* Float tests */
        float_sum += test_float_postinc_load(float_arr);
        test_float_postinc_store(float_arr, iter * 1.1f);
        
        /* Double tests */
        double_sum += test_double_postinc_load(double_arr);
        test_double_postdec_store(double_arr, iter * 2.2);
        
        /* Structure tests */
        double_sum += test_struct_traversal(struct_arr);
        
        /* Multi-dimensional tests */
        total_sum += test_2d_array_traversal(multi_arr);
        
        /* Mixed patterns */
        total_sum += test_mixed_offsets(int_arr);
        
        /* Complex patterns */
        test_complex_pattern(int_arr, int_arr + ARRAY_SIZE/2);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: int_sum=%d, float_sum=%f, double_sum=%f\n", 
           total_sum, float_sum, double_sum);
    
    /* Verification */
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += int_arr[i];
    }
    printf("Array sum: %d\n", verify_sum);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return 0;
}
