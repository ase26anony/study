/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Target-specific attributes for different architectures */
#ifdef __ARM_ARCH
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

#ifdef __powerpc__
#define TARGET_PPC __attribute__((target("cpu=powerpc64le")))
#else
#define TARGET_PPC
#endif

/* ========== Basic Type Array Traversal ========== */

NOINLINE TARGET_ARM TARGET_PPC
int test_int_postinc_load(int* arr, int n) {
    int sum = 0;
    int* ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc recognition */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;  /* Post-increment load */
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
void test_int_postinc_store(int* arr, int n, int value) {
    int* ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;  /* Post-increment store */
    }
}

NOINLINE TARGET_ARM TARGET_PPC
int test_int_postdec_load(int* arr, int n) {
    int sum = 0;
    int* ptr = &arr[n-1];  /* Start from end */
    
    /* Pattern: *ptr-- in loop */
    for (int i = 0; i < n; i++) {
        sum += *ptr--;  /* Post-decrement load */
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
void test_int_postdec_store(int* arr, int n, int value) {
    int* ptr = &arr[n-1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;  /* Post-decrement store */
    }
}

/* ========== Floating Point Types ========== */

NOINLINE TARGET_ARM TARGET_PPC
float test_float_postinc_load(float* arr, int n) {
    float sum = 0.0f;
    float* ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;  /* Float post-increment load */
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
void test_float_postinc_store(float* arr, int n, float value) {
    float* ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (float)i;  /* Float post-increment store */
    }
}

NOINLINE TARGET_ARM TARGET_PPC
double test_double_postinc_load(double* arr, int n) {
    double sum = 0.0;
    double* ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;  /* Double post-increment load */
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
void test_double_postinc_store(double* arr, int n, double value) {
    double* ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (double)i;  /* Double post-increment store */
    }
}

/* ========== Volatile Accesses ========== */

NOINLINE TARGET_ARM TARGET_PPC
int test_volatile_postinc(volatile int* arr, int n) {
    volatile int* vptr = arr;
    int sum = 0;
    
    /* Volatile access prevents elimination but should still allow auto-inc pattern */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;  /* Volatile post-increment */
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
void test_volatile_postinc_store(volatile int* arr, int n, int value) {
    volatile int* vptr = arr;
    
    for (int i = 0; i < n; i++) {
        *vptr++ = value;  /* Volatile post-increment store */
    }
}

/* ========== Constant Stride Patterns ========== */

NOINLINE TARGET_ARM TARGET_PPC
int test_const_stride(int* arr, int n) {
    int sum = 0;
    int* ptr = arr;
    
    /* Pattern with constant stride in pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr += 4;           /* Explicit stride */
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
int test_mixed_stride(int* arr, int n) {
    int sum = 0;
    int* ptr = arr;
    
    /* Mix of direct access and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += *ptr++;  /* Post-increment */
        } else {
            sum += *(ptr + 1);  /* Offset access */
            ptr += 2;           /* Different stride */
        }
    }
    return sum;
}

/* ========== Structure Array Traversal ========== */

typedef struct {
    int id;
    float value;
    double data;
    char tag;
} TestStruct;

NOINLINE TARGET_ARM TARGET_PPC
double test_struct_traversal(TestStruct* arr, int n) {
    double sum = 0.0;
    TestStruct* ptr = arr;
    
    /* Accessing struct members with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += ptr->value * ptr->data;  /* Non-contiguous member access */
        ptr++;  /* Post-increment of struct pointer */
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
void test_struct_store(TestStruct* arr, int n, int base) {
    TestStruct* ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->id = base + i;
        ptr->value = (float)(base + i) * 0.5f;
        ptr->data = (double)(base + i) * 0.25;
        ptr->tag = 'A' + (i % 26);
        ptr++;  /* Struct pointer increment */
    }
}

/* ========== Multi-Dimensional Array Patterns ========== */

NOINLINE TARGET_ARM TARGET_PPC
int test_2d_array_row_major(int arr[16][16]) {
    int sum = 0;
    int* ptr = &arr[0][0];  /* Flatten to single pointer */
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
int test_2d_array_nested(int arr[16][16]) {
    int sum = 0;
    
    /* Nested loops with inner pointer reset */
    for (int i = 0; i < 16; i++) {
        int* row_ptr = arr[i];  /* Reset pointer each outer iteration */
        for (int j = 0; j < 16; j++) {
            sum += *row_ptr++;  /* Inner loop auto-inc pattern */
        }
    }
    return sum;
}

/* ========== Complex Mixed Patterns ========== */

NOINLINE TARGET_ARM TARGET_PPC
int test_complex_pattern(int* arr1, int* arr2, int n) {
    int sum = 0;
    int* ptr1 = arr1;
    int* ptr2 = arr2;
    
    /* Multiple pointers with auto-increment in same loop */
    for (int i = 0; i < n; i++) {
        int val1 = *ptr1++;  /* Load with post-inc from arr1 */
        int val2 = *ptr2++;  /* Load with post-inc from arr2 */
        sum += val1 * val2;
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
void test_store_load_chain(int* arr, int n) {
    int* ptr = arr;
    
    /* Chain of store/load with auto-increment */
    for (int i = 0; i < n; i++) {
        *ptr = i;           /* Store */
        int val = *ptr;     /* Load */
        *ptr++ = val * 2;   /* Store with post-inc */
    }
}

/* ========== Main Driver ========== */

int main() {
    const int ARRAY_SIZE = 256;
    const int STRUCT_COUNT = 100;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    volatile int* volatile_array = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    TestStruct* struct_array = (TestStruct*)malloc(STRUCT_COUNT * sizeof(TestStruct));
    int multi_array[16][16];
    
    if (!int_array || !float_array || !double_array || !volatile_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.5f;
        double_array[i] = i * 0.25;
        volatile_array[i] = i % 100;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].id = i;
        struct_array[i].value = i * 1.5f;
        struct_array[i].data = i * 2.5;
        struct_array[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 16 + j;
        }
    }
    
    int checksum = 0;
    
    /* Execute all test patterns */
    checksum += test_int_postinc_load(int_array, ARRAY_SIZE);
    
    test_int_postinc_store(int_array, ARRAY_SIZE, 42);
    checksum += int_array[0] + int_array[ARRAY_SIZE-1];
    
    checksum += test_int_postdec_load(int_array, ARRAY_SIZE);
    
    test_int_postdec_store(int_array, ARRAY_SIZE, 100);
    checksum += int_array[0] + int_array[ARRAY_SIZE-1];
    
    checksum += (int)test_float_postinc_load(float_array, ARRAY_SIZE);
    test_float_postinc_store(float_array, ARRAY_SIZE, 3.14f);
    checksum += (int)float_array[0] + (int)float_array[ARRAY_SIZE-1];
    
    checksum += (int)test_double_postinc_load(double_array, ARRAY_SIZE);
    test_double_postinc_store(double_array, ARRAY_SIZE, 6.28);
    checksum += (int)double_array[0] + (int)double_array[ARRAY_SIZE-1];
    
    checksum += test_volatile_postinc(volatile_array, ARRAY_SIZE);
    test_volatile_postinc_store(volatile_array, ARRAY_SIZE, 999);
    checksum += volatile_array[0] + volatile_array[ARRAY_SIZE-1];
    
    checksum += test_const_stride(int_array, ARRAY_SIZE/4);
    checksum += test_mixed_stride(int_array, ARRAY_SIZE/2);
    
    checksum += (int)test_struct_traversal(struct_array, STRUCT_COUNT);
    test_struct_store(struct_array, STRUCT_COUNT, 1000);
    checksum += struct_array[0].id + struct_array[STRUCT_COUNT-1].id;
    
    checksum += test_2d_array_row_major(multi_array);
    checksum += test_2d_array_nested(multi_array);
    
    /* Create second array for complex pattern test */
    int* int_array2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array2[i] = ARRAY_SIZE - i;
    }
    
    checksum += test_complex_pattern(int_array, int_array2, ARRAY_SIZE);
    test_store_load_chain(int_array, ARRAY_SIZE);
    checksum += int_array[0] + int_array[ARRAY_SIZE-1];
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free((void*)volatile_array);
    free(struct_array);
    free(int_array2);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed. Compile with appropriate flags to trigger auto-inc-dec optimization.\n");
    
    return 0;
}
