/* test_auto_inc_dec.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-unit strides */
struct DataPoint {
    int id;
    float value;
    double precision;
    char tag;
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

/* Non-volatile pointer version */
NOINLINE int test_int_ptr_arithmetic(int *arr) {
    int *ptr = arr;
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr;    /* Access with zero offset */
        ptr += 1;       /* Explicit increment - may still pattern match */
    }
    return sum;
}

/* Constant stride access */
NOINLINE int test_int_stride4(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *vptr;   /* Access with implicit offset 0 */
        vptr += 4;      /* Constant stride */
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
        *vptr++ = value * i;  /* Float post-increment store */
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postdec_load(double *arr) {
    volatile double *vptr = &arr[ARRAY_SIZE - 1];
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;  /* Double post-decrement load */
    }
    return sum;
}

NOINLINE void test_double_postdec_store(double *arr, double value) {
    volatile double *vptr = &arr[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value / (i + 1);  /* Double post-decrement store */
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE int test_struct_traversal(struct DataPoint *arr) {
    volatile struct DataPoint *vptr = arr;
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr->id;      /* Access structure member */
        vptr++;               /* Large stride (sizeof(struct DataPoint)) */
    }
    return sum;
}

NOINLINE void test_struct_member_store(struct DataPoint *arr, int base) {
    volatile struct DataPoint *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vptr->value = base + i;  /* Store to structure member */
        vptr->id = i;
        vptr++;                  /* Post-increment */
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr[][16]) {
    volatile int *vptr = &arr[0][0];
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Linear traversal of 2D array */
    }
    return sum;
}

NOINLINE int test_nested_loop_ptr_reset(int arr[][16]) {
    int sum = 0;
    for (int row = 0; row < 16; row++) {
        volatile int *vptr = arr[row];  /* Reset each outer iteration */
        for (int col = 0; col < 16; col++) {
            sum += *vptr++;  /* Inner loop with fresh pointer */
        }
    }
    return sum;
}

/* ========== MIXED PATTERNS ========== */

NOINLINE int test_mixed_access_patterns(int *arr1, float *arr2, double *arr3) {
    volatile int *vptr1 = arr1;
    volatile float *vptr2 = arr2;
    volatile double *vptr3 = arr3;
    int result = 0;
    
    /* Interleaved accesses with different types */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        result += *vptr1++;
        *vptr2++ = (float)result;
        *vptr3-- = (double)result;  /* Note: decrement on arr3 */
    }
    return result;
}

/* ========== COMPLEX POINTER ARITHMETIC ========== */

NOINLINE int test_complex_offset_calculation(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    /* Pattern: *(ptr + constant) where ptr increments */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(vptr + 0);  /* Explicit zero offset */
        vptr += 1;
    }
    return sum;
}

NOINLINE int test_alternating_increment(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    /* Alternate between increment and no-increment */
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += *vptr;    /* No increment here */
        vptr++;
        sum += *vptr;    /* Access after increment */
        vptr++;
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct DataPoint *struct_array = (struct DataPoint*)malloc(ARRAY_SIZE * sizeof(struct DataPoint));
    int (*int_2d)[16] = (int(*)[16])malloc(16 * 16 * sizeof(int));
    
    if (!int_array || !float_array || !double_array || !struct_array || !int_2d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (float)(i + 1) * 0.5f;
        double_array[i] = (double)(i + 1) * 0.25;
        struct_array[i].id = i;
        struct_array[i].value = (float)i;
        struct_array[i].precision = (double)i;
        struct_array[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Run tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_checksum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_checksum += test_int_postdec_load(int_array);
        test_int_postdec_store(int_array, iter);
        total_checksum += test_int_ptr_arithmetic(int_array);
        total_checksum += test_int_stride4(int_array);
        
        /* Float tests */
        total_checksum += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, (float)iter);
        
        /* Double tests */
        total_checksum += (int)test_double_postdec_load(double_array);
        test_double_postdec_store(double_array, (double)iter);
        
        /* Structure tests */
        total_checksum += test_struct_traversal(struct_array);
        test_struct_member_store(struct_array, iter);
        
        /* Multi-dimensional tests */
        total_checksum += test_2d_array_traversal(int_2d);
        total_checksum += test_nested_loop_ptr_reset(int_2d);
        
        /* Mixed patterns */
        total_checksum += test_mixed_access_patterns(int_array, float_array, double_array);
        
        /* Complex patterns */
        total_checksum += test_complex_offset_calculation(int_array);
        total_checksum += test_alternating_increment(int_array);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    free(int_2d);
    
    return 0;
}
