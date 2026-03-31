/* auto_inc_dec_test.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define CHECKSUM_SEED 0xDEADBEEF

/* Structure for testing non-trivial offsets */
struct test_struct {
    int id;
    float value;
    double data;
    char tag;
    int padding[3]; /* Ensure non-power-of-two size */
};

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

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
    
    /* Pattern 1: Post-increment store with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 2: Store with pointer arithmetic */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(ptr + i) = value - i;  /* Different offset pattern */
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Pattern 1: Post-decrement load from end to beginning */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern 1: Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + (i * 2);
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Mixed volatile and non-volatile patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += ptr[i];  /* Array indexing that should become pointer arithmetic */
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Simple post-increment with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pointer with constant offset in loop */
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 1);  /* Constant offset of 1 element */
        ptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postdec_store(double *arr, double value) {
    volatile double *vptr = &arr[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value / (i + 1);
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE double test_struct_traversal(struct test_struct *arr) {
    double sum = 0.0;
    
    /* Access multiple members with different offsets */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i].value + arr[i].data;
    }
    
    /* Pointer-based traversal */
    struct test_struct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->data;
        ptr++;  /* Large, non-power-of-two stride */
    }
    
    return sum;
}

NOINLINE void test_struct_store(struct test_struct *arr, int base) {
    volatile struct test_struct *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vptr->id = base + i;
        vptr->value = (float)(base + i) / 2.0f;
        vptr->data = (double)(base + i) * 1.5;
        vptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr2d[16][16]) {
    int sum = 0;
    
    /* Row-major traversal with single pointer */
    int *ptr = &arr2d[0][0];
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    
    /* Nested loops with pointer reset */
    for (int row = 0; row < 16; row++) {
        int *row_ptr = arr2d[row];
        for (int col = 0; col < 16; col++) {
            sum += *row_ptr++;
        }
    }
    
    return sum;
}

NOINLINE void test_2d_array_store(int arr2d[16][16], int value) {
    volatile int *ptr = &arr2d[0][0];
    
    for (int i = 0; i < 16 * 16; i++) {
        *ptr++ = value + i;
    }
}

/* ========== COMPLEX PATTERN TESTS ========== */

NOINLINE int test_mixed_increment_patterns(int *arr) {
    int sum = 0;
    volatile int *vptr = arr;
    
    /* Mix of pre and post operations */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += *vptr++;      /* Post-increment */
        sum += *(vptr + 1);  /* Offset without increment */
        vptr++;              /* Bare increment */
    }
    
    /* Different stride */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 3) {
        sum += ptr[0] + ptr[1] + ptr[2];
        ptr += 3;
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct test_struct *struct_arr = 
        (struct test_struct*)malloc(ARRAY_SIZE * sizeof(struct test_struct));
    int arr2d[16][16];
    
    /* Initialize with non-zero patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i * 3 + 1;
        float_arr[i] = (float)i * 1.5f;
        double_arr[i] = (double)i * 2.5;
        struct_arr[i].id = i;
        struct_arr[i].value = (float)i * 0.5f;
        struct_arr[i].data = (double)i * 1.25;
        struct_arr[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            arr2d[i][j] = i * 16 + j;
        }
    }
    
    /* Execute all test patterns */
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    double struct_sum = 0.0;
    int md_sum = 0;
    
    /* Integer tests */
    int_sum += test_int_postinc_load(int_arr);
    test_int_postinc_store(int_arr, 42);
    int_sum += test_int_postdec_load(int_arr);
    test_int_postdec_store(int_arr, 100);
    int_sum += test_mixed_increment_patterns(int_arr);
    
    /* Float tests */
    float_sum += test_float_postinc_load(float_arr);
    test_float_postinc_store(float_arr, 3.14f);
    
    /* Double tests */
    double_sum += test_double_postinc_load(double_arr);
    test_double_postdec_store(double_arr, 2.71828);
    
    /* Struct tests */
    struct_sum += test_struct_traversal(struct_arr);
    test_struct_store(struct_arr, 1000);
    
    /* Multi-dimensional tests */
    md_sum += test_2d_array_traversal(arr2d);
    test_2d_array_store(arr2d, 500);
    
    /* Compute verification checksum */
    uint32_t checksum = CHECKSUM_SEED;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= int_arr[i];
        checksum = (checksum << 13) | (checksum >> 19);
        checksum += (uint32_t)(float_arr[i] * 1000);
        checksum ^= (uint32_t)(double_arr[i] * 1000);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("  Integer sum: %d\n", int_sum);
    printf("  Float sum: %f\n", float_sum);
    printf("  Double sum: %f\n", double_sum);
    printf("  Struct sum: %f\n", struct_sum);
    printf("  2D array sum: %d\n", md_sum);
    printf("  Final checksum: 0x%08X\n", checksum);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return (checksum == CHECKSUM_SEED) ? 0 : 1;
}
