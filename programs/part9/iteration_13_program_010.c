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
struct DataStruct {
    int val;
    float fval;
    double dval;
    char padding[32];  /* Force larger stride */
};

/* ========== INTEGER TESTS ========== */

/* Simple post-increment load */
NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Should trigger auto-inc pattern */
    }
    return sum;
}

/* Simple post-increment store */
NOINLINE void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;  /* Store with post-increment */
    }
}

/* Post-decrement load */
NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;  /* Should trigger auto-dec pattern */
    }
    return sum;
}

/* Post-decrement store */
NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value - i;  /* Store with post-decrement */
    }
}

/* Pointer arithmetic with constant stride */
NOINLINE int test_int_stride4_load(int *arr) {
    int *ptr = arr;
    int sum = 0;
    /* Access every 4th element using pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr += 4;           /* Explicit increment */
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Float auto-inc load */
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * i;  /* Float auto-inc store */
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Double auto-inc load */
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    volatile double *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value / (i + 1);  /* Double auto-inc store */
    }
}

/* ========== STRUCTURE TESTS ========== */

/* Structure array traversal - tests larger offsets */
NOINLINE int test_struct_member_load(struct DataStruct *arr) {
    int sum = 0;
    struct DataStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->val;  /* Access struct member */
        ptr++;            /* Pointer increments by struct size */
    }
    return sum;
}

NOINLINE void test_struct_member_store(struct DataStruct *arr, int value) {
    struct DataStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->val = value + i;  /* Store to struct member */
        ptr->fval = (float)(value + i) / 10.0f;
        ptr->dval = (double)(value + i) / 100.0;
        ptr++;                 /* Post-increment */
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_row_major(int arr2d[16][16]) {
    int sum = 0;
    int *ptr = &arr2d[0][0];  /* Treat as 1D array */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;  /* Row-major traversal */
    }
    return sum;
}

NOINLINE void test_2d_array_nested(int arr2d[16][16], int value) {
    for (int i = 0; i < 16; i++) {
        volatile int *vptr = arr2d[i];
        for (int j = 0; j < 16; j++) {
            *vptr++ = value + i * 16 + j;  /* Inner loop pointer */
        }
    }
}

/* ========== MIXED ACCESS PATTERN TESTS ========== */

/* Combined load/store with same pointer */
NOINLINE void test_mixed_load_store(int *src, int *dst) {
    volatile int *vsrc = src;
    volatile int *vdst = dst;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int val = *vsrc++;  /* Load with auto-inc */
        *vdst++ = val * 2;  /* Store with auto-inc */
    }
}

/* Loop with pointer reset - tests pattern recognition in smaller blocks */
NOINLINE int test_pointer_reset(int *arr, int blocks) {
    int sum = 0;
    for (int b = 0; b < blocks; b++) {
        volatile int *vptr = arr;
        for (int i = 0; i < 16; i++) {
            sum += *vptr++;  /* Pointer reset each outer iteration */
        }
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct DataStruct *struct_arr = (struct DataStruct*)malloc(ARRAY_SIZE * sizeof(struct DataStruct));
    int arr2d[16][16];
    
    if (!int_arr || !float_arr || !double_arr || !struct_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].val = i * 3;
        struct_arr[i].fval = i * 0.75f;
        struct_arr[i].dval = i * 1.25;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            arr2d[i][j] = i * 16 + j;
        }
    }
    
    int checksum = 0;
    
    /* Run all tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        checksum += test_int_postinc_load(int_arr);
        test_int_postinc_store(int_arr, iter);
        checksum += test_int_postdec_load(int_arr);
        test_int_postdec_store(int_arr, iter);
        checksum += test_int_stride4_load(int_arr);
        
        /* Float tests */
        checksum += (int)test_float_postinc_load(float_arr);
        test_float_postinc_store(float_arr, iter * 1.0f);
        
        /* Double tests */
        checksum += (int)test_double_postinc_load(double_arr);
        test_double_postinc_store(double_arr, iter * 1.0);
        
        /* Structure tests */
        checksum += test_struct_member_load(struct_arr);
        test_struct_member_store(struct_arr, iter);
        
        /* Multi-dimensional tests */
        checksum += test_2d_array_row_major(arr2d);
        test_2d_array_nested(arr2d, iter);
        
        /* Mixed pattern tests */
        int *dst_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
        if (dst_arr) {
            test_mixed_load_store(int_arr, dst_arr);
            checksum += dst_arr[0];
            free(dst_arr);
        }
        
        checksum += test_pointer_reset(int_arr, 4);
    }
    
    /* Final validation */
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return 0;
}
