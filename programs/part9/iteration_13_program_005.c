/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

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

/* Volatile pointer types for preventing optimization */
typedef volatile int* vint_ptr;
typedef volatile float* vfloat_ptr;
typedef volatile double* vdouble_ptr;

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int* arr) {
    int sum = 0;
    int* ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_int_postinc_store(int* arr, int value) {
    int* ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
}

NOINLINE int test_int_postdec_load(int* arr) {
    int sum = 0;
    int* ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- in loop - should trigger auto-dec */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE void test_int_postdec_store(int* arr, int value) {
    int* ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
}

NOINLINE int test_int_volatile_postinc(vint_ptr arr) {
    int sum = 0;
    vint_ptr ptr = arr;
    
    /* Volatile pointer with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_int_stride4(int* arr) {
    int sum = 0;
    int* ptr = arr;
    
    /* Pointer arithmetic with constant stride */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float* arr) {
    float sum = 0.0f;
    float* ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float* arr, float value) {
    float* ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value * i;
    }
}

NOINLINE float test_float_postdec_load(float* arr) {
    float sum = 0.0f;
    float* ptr = &arr[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    return sum;
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double* arr) {
    double sum = 0.0;
    double* ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double* arr, double value) {
    double* ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value / (i + 1);
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE double test_struct_array_traversal(struct TestStruct* arr) {
    double sum = 0.0;
    struct TestStruct* ptr = arr;
    
    /* Accessing struct members with non-trivial offsets */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->value + ptr->data;
        ptr++;  /* Should generate auto-inc with large offset */
    }
    return sum;
}

NOINLINE void test_struct_member_store(struct TestStruct* arr, int base) {
    struct TestStruct* ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = base + i;
        ptr->value = (float)i * 1.5f;
        ptr->data = (double)i * 2.5;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_row_major(int arr[16][16]) {
    int sum = 0;
    int* ptr = &arr[0][0];
    
    /* Single pointer traversing 2D array in row-major order */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loop_reset(int arr[8][32]) {
    int total = 0;
    
    /* Outer loop resets pointer each iteration */
    for (int row = 0; row < 8; row++) {
        int* ptr = arr[row];
        int row_sum = 0;
        
        /* Inner loop with pointer increment */
        for (int col = 0; col < 32; col++) {
            row_sum += *ptr++;
        }
        total += row_sum;
    }
    return total;
}

/* ========== MIXED PATTERNS ========== */

NOINLINE int test_mixed_increment_patterns(int* arr1, int* arr2) {
    int sum = 0;
    int* p1 = arr1;
    int* p2 = arr2;
    
    /* Mixed pre and post operations */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *p1++;      /* post-increment */
        sum += *(++p2);    /* pre-increment - different pattern */
    }
    return sum;
}

NOINLINE void test_pointer_arithmetic_loop(int* arr) {
    int* ptr = arr;
    
    /* Explicit pointer arithmetic that should simplify to auto-inc */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr = i * 2;
        ptr = ptr + 1;  /* Not ptr++, different syntax */
    }
}

/* ========== COMPLEX SCENARIO ========== */

NOINLINE double test_complex_access_pattern(double* arr, int* indices) {
    double sum = 0.0;
    double* ptr = arr;
    
    /* More complex but still sequential pattern */
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += *ptr++;
        sum += *ptr++;  /* Two increments per iteration */
        
        /* Additional computation to prevent over-optimization */
        sum += sin((double)i);
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct TestStruct* struct_array = (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    
    int arr_2d[16][16];
    int arr_nested[8][32];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (float)i * 0.5f;
        double_array[i] = (double)i * 0.25;
        struct_array[i].id = i;
        struct_array[i].value = (float)i * 1.1f;
        struct_array[i].data = (double)i * 2.2;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            arr_2d[i][j] = i * 16 + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 32; j++) {
            arr_nested[i][j] = i * 32 + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Execute all test functions */
    total_checksum += test_int_postinc_load(int_array);
    test_int_postinc_store(int_array, 42);
    total_checksum += test_int_postdec_load(int_array);
    test_int_postdec_store(int_array, 100);
    
    total_checksum += (int)test_float_postinc_load(float_array);
    test_float_postinc_store(float_array, 3.14f);
    total_checksum += (int)test_float_postdec_load(float_array);
    
    total_checksum += (int)test_double_postinc_load(double_array);
    test_double_postinc_store(double_array, 2.71828);
    
    total_checksum += (int)test_struct_array_traversal(struct_array);
    test_struct_member_store(struct_array, 1000);
    
    total_checksum += test_2d_array_row_major(arr_2d);
    total_checksum += test_nested_loop_reset(arr_nested);
    
    /* Volatile test */
    total_checksum += test_int_volatile_postinc((vint_ptr)int_array);
    
    /* Mixed patterns */
    total_checksum += test_int_stride4(int_array);
    total_checksum += test_mixed_increment_patterns(int_array, int_array + ARRAY_SIZE/2);
    test_pointer_arithmetic_loop(int_array);
    
    /* Complex pattern */
    int* indices = (int*)malloc(ARRAY_SIZE * sizeof(int));
    for (int i = 0; i < ARRAY_SIZE; i++) indices[i] = i;
    total_checksum += (int)test_complex_access_pattern(double_array, indices);
    
    /* Verification output */
    printf("Total checksum: %d\n", total_checksum);
    printf("Sample values for verification:\n");
    printf("  int_array[0] = %d, [100] = %d\n", int_array[0], int_array[100]);
    printf("  float_array[50] = %f\n", float_array[50]);
    printf("  struct_array[10].id = %d\n", struct_array[10].id);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    free(indices);
    
    return (total_checksum != 0) ? 0 : 1;
}
