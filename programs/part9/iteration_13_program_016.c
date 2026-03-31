/* test_auto_inc_dec.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define CHECKSUM_SEED 0xDEADBEEF

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-unit stride access */
typedef struct {
    int val;
    float fval;
    double dval;
    char padding[8]; /* Ensure non-power-of-two size */
} TestStruct;

/* ========== INTEGER TESTS ========== */

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
    
    /* Post-decrement load from end to beginning */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Post-decrement store from end to beginning */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i * 2;
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Float post-increment with constant stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr;
        vptr = vptr + 1; /* Alternative form of increment */
    }
    
    /* Direct post-increment */
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    /* Store with post-increment and offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(vptr + 0) = value + i;
        vptr++;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Double with mixed increment patterns */
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += *vptr++;
        sum += *vptr++; /* Two increments per iteration */
    }
    
    return sum;
}

NOINLINE void test_double_postdec_store(double *arr, double value) {
    volatile double *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Post-decrement with computation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr = value * i;
        vptr--;
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE int test_struct_traversal(TestStruct *arr) {
    volatile TestStruct *vptr = arr;
    int sum = 0;
    
    /* Access struct members with pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr->val;
        vptr++; /* Large, non-unit stride */
    }
    
    /* Access specific member with offset calculation */
    TestStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->fval; /* Different offset within struct */
        ptr++;
    }
    
    return sum;
}

NOINLINE void test_struct_store(TestStruct *arr, int ival, float fval, double dval) {
    volatile TestStruct *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vptr->val = ival + i;
        vptr->fval = fval + i;
        vptr->dval = dval + i;
        vptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

#define ROWS 16
#define COLS 16

NOINLINE int test_2d_array_traversal(int arr[ROWS][COLS]) {
    volatile int *ptr = &arr[0][0];
    int sum = 0;
    
    /* Traverse 2D array as 1D with single pointer */
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_nested_loop_access(int arr[ROWS][COLS], int value) {
    /* Nested loops with pointer reset each iteration */
    for (int i = 0; i < ROWS; i++) {
        volatile int *row_ptr = arr[i];
        for (int j = 0; j < COLS; j++) {
            *row_ptr++ = value + i * COLS + j;
        }
    }
}

/* ========== COMPLEX PATTERN TESTS ========== */

NOINLINE int test_mixed_increment_patterns(int *arr) {
    int *ptr = arr;
    int sum = 0;
    
    /* Mixed increment amounts */
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        sum += *ptr;   /* No increment */
        sum += *(ptr + 1); /* Offset without modifying ptr */
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;      /* Bulk increment */
    }
    
    return sum;
}

NOINLINE void test_pointer_arithmetic_with_constants(int *arr) {
    volatile int *vptr = arr;
    
    /* Explicit pointer arithmetic with constants */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(vptr + 0) = i * 2;
        vptr = vptr + 1; /* Explicit reassignment */
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    TestStruct *struct_arr = (TestStruct*)malloc(ARRAY_SIZE * sizeof(TestStruct));
    int (*multi_arr)[COLS] = (int(*)[COLS])malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i ^ CHECKSUM_SEED;
        float_arr[i] = (float)i * 1.5f;
        double_arr[i] = (double)i * 2.5;
        struct_arr[i].val = i * 3;
        struct_arr[i].fval = (float)i * 0.5f;
        struct_arr[i].dval = (double)i * 1.5;
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            multi_arr[i][j] = i * COLS + j;
        }
    }
    
    /* Execute all test functions */
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    int struct_sum = 0;
    
    printf("Starting auto-inc/dec pattern tests...\n");
    
    /* Integer tests */
    int_sum += test_int_postinc_load(int_arr);
    test_int_postinc_store(int_arr, 42);
    int_sum += test_int_postdec_load(int_arr);
    test_int_postdec_store(int_arr, 100);
    
    /* Float tests */
    float_sum += test_float_postinc_load(float_arr);
    test_float_postinc_store(float_arr, 3.14f);
    
    /* Double tests */
    double_sum += test_double_postinc_load(double_arr);
    test_double_postdec_store(double_arr, 2.71828);
    
    /* Structure tests */
    struct_sum += test_struct_traversal(struct_arr);
    test_struct_store(struct_arr, 10, 20.0f, 30.0);
    
    /* Multi-dimensional tests */
    int_sum += test_2d_array_traversal(multi_arr);
    test_nested_loop_access(multi_arr, 999);
    
    /* Complex pattern tests */
    int_sum += test_mixed_increment_patterns(int_arr);
    test_pointer_arithmetic_with_constants(int_arr);
    
    /* Compute final checksum */
    uint64_t checksum = (uint64_t)int_sum + 
                       (uint64_t)float_sum + 
                       (uint64_t)double_sum + 
                       (uint64_t)struct_sum;
    
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    free(multi_arr);
    
    return 0;
}
