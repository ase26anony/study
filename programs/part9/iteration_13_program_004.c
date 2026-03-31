/* auto_inc_dec_test.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

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
    int *ptr = arr;
    
    /* Pattern 3: Post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
    
    /* Pattern 4: Post-decrement store */
    ptr = &arr[ARRAY_SIZE - 1];
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
    
    /* Pattern 6: Float post-increment with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 7: Float with constant offset */
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 1);  /* Constant offset of 1 float */
        ptr++;
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    float *ptr = arr;
    
    /* Pattern 8: Float post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value * i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Pattern 9: Double post-increment with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postdec_store(double *arr, double value) {
    double *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern 10: Double post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value / (i + 1);
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE double test_struct_traversal(struct TestStruct *arr) {
    double sum = 0.0;
    
    /* Pattern 11: Struct member access with pointer arithmetic */
    struct TestStruct *ptr = arr;
    
    /* Access different members to test various offsets */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->value;      /* float member */
        sum += ptr->data;       /* double member */
        ptr++;                  /* Large stride (sizeof struct) */
    }
    
    /* Pattern 12: Mixed access pattern */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = i;
        ptr->value = i * 1.5f;
        ptr++;
    }
    
    return sum;
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr2d[16][16]) {
    int sum = 0;
    
    /* Pattern 13: 2D array traversal with single pointer */
    int *ptr = &arr2d[0][0];
    
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    
    /* Pattern 14: Nested loops with pointer reset */
    for (int row = 0; row < 16; row++) {
        ptr = &arr2d[row][0];
        for (int col = 0; col < 16; col++) {
            *ptr++ = row * 16 + col;
        }
    }
    
    return sum;
}

/* ========== COMPLEX PATTERNS ========== */

NOINLINE int test_mixed_patterns(int *arr1, int *arr2, int *arr3) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    int *p3 = arr3;
    
    /* Pattern 15: Multiple pointers with interleaved increments */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *p1++;
        *p2++ = sum;
        sum -= *p3++;
    }
    
    /* Pattern 16: Pointer arithmetic with compile-time constant */
    p1 = arr1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* This should generate (mem (plus (reg) (const_int 4))) */
        sum += *(p1 + 1);
        p1++;
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct TestStruct *struct_array = 
        (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    int array_2d[16][16];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (i + 1) * 1.5f;
        double_array[i] = (i + 1) * 2.5;
        struct_array[i].id = i;
        struct_array[i].value = i * 3.0f;
        struct_array[i].data = i * 4.0;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_sum = 0;
    
    /* Run tests multiple times to ensure loops aren't optimized away */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_sum += test_int_postdec_load(int_array);
        
        total_sum += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, iter * 1.0f);
        
        total_sum += (int)test_double_postinc_load(double_array);
        test_double_postdec_store(double_array, iter * 2.0);
        
        total_sum += (int)test_struct_traversal(struct_array);
        total_sum += test_2d_array_traversal(array_2d);
        
        /* Create additional arrays for mixed pattern test */
        int *arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
        int *arr3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
        if (arr2 && arr3) {
            total_sum += test_mixed_patterns(int_array, arr2, arr3);
            free(arr2);
            free(arr3);
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
