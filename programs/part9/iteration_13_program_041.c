/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Target architecture attributes for ARM */
#ifdef __arm__
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

/* Structure for testing non-trivial offsets */
typedef struct {
    int val;
    float fval;
    double dval;
    char padding[8];
} TestStruct;

/* ========== INTEGER ARRAY TRAVERSALS ========== */

NOINLINE TARGET_ARM
int test_int_postinc_load(int* arr, int size) {
    int sum = 0;
    int* ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc */
    for (int i = 0; i < size; i++) {
        sum += *ptr++;  /* Post-increment load */
    }
    return sum;
}

NOINLINE TARGET_ARM
void test_int_postinc_store(int* arr, int size, int value) {
    int* ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < size; i++) {
        *ptr++ = value + i;  /* Post-increment store */
    }
}

NOINLINE TARGET_ARM
int test_int_postdec_load(int* arr, int size) {
    int sum = 0;
    int* ptr = &arr[size - 1];  /* Start from end */
    
    /* Pattern: *ptr-- in loop - should trigger auto-dec */
    for (int i = 0; i < size; i++) {
        sum += *ptr--;  /* Post-decrement load */
    }
    return sum;
}

NOINLINE TARGET_ARM
void test_int_postdec_store(int* arr, int size, int value) {
    int* ptr = &arr[size - 1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < size; i++) {
        *ptr-- = value - i;  /* Post-decrement store */
    }
}

/* ========== FLOAT ARRAY TRAVERSALS ========== */

NOINLINE TARGET_ARM
float test_float_postinc_load(float* arr, int size) {
    float sum = 0.0f;
    float* ptr = arr;
    
    /* Mixed patterns with different strides */
    for (int i = 0; i < size; i += 2) {
        sum += *ptr;        /* Direct access */
        sum += *(ptr + 1);  /* Offset access */
        ptr += 2;           /* Manual increment */
    }
    return sum;
}

NOINLINE TARGET_ARM
void test_float_postinc_store(float* arr, int size) {
    volatile float* vptr = arr;  /* Volatile to prevent elimination */
    
    /* Volatile pointer with post-increment */
    for (int i = 0; i < size; i++) {
        *vptr++ = (float)i * 1.5f;
    }
}

/* ========== DOUBLE ARRAY TRAVERSALS ========== */

NOINLINE TARGET_ARM
double test_double_postinc_load(double* arr, int size) {
    double sum = 0.0;
    double* ptr = arr;
    
    /* Simple post-increment pattern */
    for (int i = 0; i < size; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE TARGET_ARM
void test_double_postdec_store(double* arr, int size) {
    double* ptr = &arr[size - 1];
    
    /* Post-decrement store with computation */
    for (int i = 0; i < size; i++) {
        *ptr-- = (double)(size - i) / 2.0;
    }
}

/* ========== STRUCTURE ARRAY TRAVERSALS ========== */

NOINLINE TARGET_ARM
int test_struct_traversal(TestStruct* arr, int size) {
    int sum = 0;
    TestStruct* ptr = arr;
    
    /* Access struct members with pointer arithmetic */
    for (int i = 0; i < size; i++) {
        sum += ptr->val;      /* Access member - non-trivial offset */
        ptr++;                /* Pointer increment by struct size */
    }
    return sum;
}

NOINLINE TARGET_ARM
void test_struct_member_store(TestStruct* arr, int size) {
    TestStruct* ptr = arr;
    
    /* Store to different struct members */
    for (int i = 0; i < size; i++) {
        ptr->val = i;
        ptr->fval = (float)i;
        ptr->dval = (double)i;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TRAVERSAL ========== */

NOINLINE TARGET_ARM
int test_2d_array_traversal(int arr[][16], int rows) {
    int sum = 0;
    
    /* Row-major traversal with pointer reset */
    for (int i = 0; i < rows; i++) {
        int* ptr = arr[i];  /* Reset pointer each outer iteration */
        
        /* Inner loop with post-increment */
        for (int j = 0; j < 16; j++) {
            sum += *ptr++;
        }
    }
    return sum;
}

/* ========== COMPLEX PATTERNS WITH CONSTANT STRIDE ========== */

NOINLINE TARGET_ARM
int test_stride_access(int* arr, int size) {
    int sum = 0;
    int* ptr = arr;
    const int stride = 4;
    
    /* Access with constant stride using pointer arithmetic */
    for (int i = 0; i < size / stride; i++) {
        sum += *ptr;        /* Base */
        sum += *(ptr + 1);  /* Offset 1 */
        sum += *(ptr + 2);  /* Offset 2 */
        sum += *(ptr + 3);  /* Offset 3 */
        ptr += stride;      /* Jump by stride */
    }
    return sum;
}

/* ========== VOLATILE ACCESS PATTERNS ========== */

NOINLINE TARGET_ARM
int test_volatile_mixed(volatile int* arr, int size) {
    int sum = 0;
    volatile int* vptr = arr;
    
    /* Mix of volatile and non-volatile patterns */
    for (int i = 0; i < size; i++) {
        sum += *vptr++;  /* Volatile post-increment */
    }
    
    /* Additional non-volatile pointer in same function */
    int* nptr = (int*)arr;
    for (int i = 0; i < size; i += 2) {
        sum += nptr[i];  /* Indexed access */
    }
    
    return sum;
}

/* ========== NESTED LOOP WITH POINTER RESET ========== */

NOINLINE TARGET_ARM
void test_nested_loop_reset(int* arr, int outer, int inner) {
    /* Outer loop resets pointer each iteration */
    for (int i = 0; i < outer; i++) {
        int* ptr = arr + (i * inner);
        
        /* Inner loop with post-increment */
        for (int j = 0; j < inner; j++) {
            *ptr++ = i * 100 + j;
        }
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    const int SIZE = 256;
    const int STRUCT_SIZE = 64;
    const int ROWS = 8;
    
    /* Allocate and initialize arrays */
    int* int_arr = (int*)malloc(SIZE * sizeof(int));
    float* float_arr = (float*)malloc(SIZE * sizeof(float));
    double* double_arr = (double*)malloc(SIZE * sizeof(double));
    TestStruct* struct_arr = (TestStruct*)malloc(STRUCT_SIZE * sizeof(TestStruct));
    int (*arr_2d)[16] = (int(*)[16])malloc(ROWS * 16 * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i + 1;
        float_arr[i] = (float)(i + 1) * 0.5f;
        double_arr[i] = (double)(i + 1) * 0.25;
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        struct_arr[i].val = i * 2;
        struct_arr[i].fval = (float)i * 1.5f;
        struct_arr[i].dval = (double)i * 2.5;
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < 16; j++) {
            arr_2d[i][j] = i * 16 + j;
        }
    }
    
    /* Execute all test functions */
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* Integer tests */
    int_sum += test_int_postinc_load(int_arr, SIZE);
    test_int_postinc_store(int_arr, SIZE, 42);
    int_sum += test_int_postdec_load(int_arr, SIZE);
    test_int_postdec_store(int_arr, SIZE, 100);
    
    /* Float tests */
    float_sum += test_float_postinc_load(float_arr, SIZE);
    test_float_postinc_store(float_arr, SIZE);
    
    /* Double tests */
    double_sum += test_double_postinc_load(double_arr, SIZE);
    test_double_postdec_store(double_arr, SIZE);
    
    /* Struct tests */
    int_sum += test_struct_traversal(struct_arr, STRUCT_SIZE);
    test_struct_member_store(struct_arr, STRUCT_SIZE);
    
    /* Multi-dimensional tests */
    int_sum += test_2d_array_traversal(arr_2d, ROWS);
    
    /* Complex pattern tests */
    int_sum += test_stride_access(int_arr, SIZE);
    int_sum += test_volatile_mixed(int_arr, SIZE);
    
    /* Nested loop test */
    test_nested_loop_reset(int_arr, 4, SIZE/4);
    
    /* Verification output */
    printf("Results:\n");
    printf("  Integer sum: %d\n", int_sum);
    printf("  Float sum: %f\n", float_sum);
    printf("  Double sum: %lf\n", double_sum);
    
    /* Verify some values */
    int verify = 0;
    for (int i = 0; i < 10; i++) {
        verify += int_arr[i];
    }
    printf("  First 10 elements sum: %d\n", verify);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    free(arr_2d);
    
    return 0;
}
