/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Prevent unwanted optimizations that might obscure the patterns */
#define NOINLINE __attribute__((noinline, noipa))
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#define TARGET_PPC __attribute__((target("cpu=powerpc")))

/* Test structures for complex access patterns */
struct TestStruct {
    int id;
    float value;
    double data;
    char tag;
};

/* Global arrays to prevent stack optimizations */
static int global_int_array[1024];
static float global_float_array[1024];
static double global_double_array[1024];
static struct TestStruct global_struct_array[256];

/* Initialize arrays with non-zero values */
void initialize_arrays(void) {
    for (int i = 0; i < 1024; i++) {
        global_int_array[i] = i * 3 + 1;
        global_float_array[i] = i * 1.5f;
        global_double_array[i] = i * 2.5;
    }
    for (int i = 0; i < 256; i++) {
        global_struct_array[i].id = i;
        global_struct_array[i].value = i * 0.75f;
        global_struct_array[i].data = i * 1.25;
        global_struct_array[i].tag = 'A' + (i % 26);
    }
}

/* ========== INTEGER TESTS ========== */

NOINLINE TARGET_ARM
int test_int_postinc_load(void) {
    volatile int* vptr = global_int_array;
    int sum = 0;
    
    /* Pattern: post-increment load with volatile */
    for (int i = 0; i < 256; i++) {
        sum += *vptr++;  /* Should trigger mem_insn setup with reg1_val = 0 */
    }
    
    /* Non-volatile version */
    int* ptr = global_int_array + 256;
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_int_postinc_store(int value) {
    int* ptr = global_int_array + 512;
    
    /* Pattern: post-increment store */
    for (int i = 0; i < 256; i++) {
        *ptr++ = value + i;
    }
    
    /* With constant stride */
    ptr = global_int_array + 768;
    for (int i = 0; i < 128; i++) {
        *(ptr + 4) = value * i;  /* Different offset pattern */
        ptr += 2;  /* Non-unit stride */
    }
}

NOINLINE TARGET_PPC
int test_int_postdec_ops(void) {
    volatile int* vptr = &global_int_array[511];
    int sum = 0;
    
    /* Pattern: post-decrement load */
    for (int i = 0; i < 256; i++) {
        sum += *vptr--;
    }
    
    /* Post-decrement store */
    int* ptr = &global_int_array[767];
    for (int i = 0; i < 256; i++) {
        *ptr-- = i * 2;
    }
    
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE
float test_float_postinc_load(void) {
    volatile float* vptr = global_float_array;
    float sum = 0.0f;
    
    /* Simple base pointer traversal */
    for (int i = 0; i < 256; i++) {
        sum += *vptr++;
    }
    
    /* Pointer arithmetic with constant offset */
    float* ptr = global_float_array + 256;
    for (int i = 0; i < 256; i++) {
        sum += *(ptr + 1);  /* Constant offset of 1 float */
        ptr++;
    }
    
    return sum;
}

NOINLINE
void test_float_postinc_store(float base) {
    float* ptr = global_float_array + 512;
    
    /* Mixed increment patterns */
    for (int i = 0; i < 128; i++) {
        *ptr++ = base + i;
        *ptr++ = base - i;  /* Two stores per iteration */
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE TARGET_ARM
double test_double_postinc_load(void) {
    double* ptr = global_double_array;
    double sum = 0.0;
    
    /* Clean post-increment pattern */
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    
    /* With volatile for different RTL generation */
    volatile double* vptr = global_double_array + 256;
    for (int i = 0; i < 256; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

NOINLINE TARGET_PPC
void test_double_postdec_store(double base) {
    double* ptr = &global_double_array[511];
    
    /* Post-decrement store pattern */
    for (int i = 0; i < 256; i++) {
        *ptr-- = base * i;
    }
    
    /* Mixed increment/decrement */
    ptr = &global_double_array[767];
    for (int i = 0; i < 128; i++) {
        *ptr = base + i;
        ptr -= 2;  /* Decrement by 2 */
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE TARGET_ARM
double test_struct_traversal(void) {
    struct TestStruct* ptr = global_struct_array;
    double sum = 0.0;
    
    /* Access different struct members with same base pointer */
    for (int i = 0; i < 128; i++) {
        sum += ptr->value;      /* float member */
        sum += ptr->data;       /* double member */
        ptr++;                  /* Post-increment */
    }
    
    /* Access with pointer arithmetic */
    ptr = global_struct_array + 128;
    for (int i = 0; i < 128; i++) {
        sum += (ptr + i)->data;  /* Constant offset pattern */
    }
    
    return sum;
}

NOINLINE
void test_struct_member_store(int start_id) {
    struct TestStruct* ptr = global_struct_array;
    
    /* Store to different struct members */
    for (int i = 0; i < 128; i++) {
        ptr->id = start_id + i;
        ptr->value = i * 0.5f;
        ptr++;  /* Post-increment after multiple stores */
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE TARGET_ARM
int test_2d_array_traversal(void) {
    int matrix[64][64];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = i * 64 + j;
        }
    }
    
    /* Row-major traversal with single pointer */
    int* ptr = &matrix[0][0];
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            sum += *ptr++;
        }
    }
    
    /* Column-major with reset each row */
    for (int j = 0; j < 64; j++) {
        ptr = &matrix[0][j];  /* Reset each outer iteration */
        for (int i = 0; i < 64; i++) {
            sum += *ptr;
            ptr += 64;  /* Next row, same column */
        }
    }
    
    return sum;
}

/* ========== COMPLEX PATTERN TESTS ========== */

NOINLINE TARGET_PPC
int test_mixed_increment_patterns(void) {
    int data[1024];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 1024; i++) {
        data[i] = i;
    }
    
    /* Pattern 1: Simple post-increment */
    int* ptr1 = data;
    for (int i = 0; i < 256; i++) {
        sum += *ptr1++;
    }
    
    /* Pattern 2: Post-increment with stride */
    int* ptr2 = data + 256;
    for (int i = 0; i < 128; i++) {
        sum += *ptr2;
        ptr2 += 2;  /* Stride of 2 */
    }
    
    /* Pattern 3: Post-decrement */
    int* ptr3 = data + 767;
    for (int i = 0; i < 256; i++) {
        sum += *ptr3--;
    }
    
    /* Pattern 4: Pre-increment (should NOT trigger the same pattern) */
    int* ptr4 = data + 512;
    for (int i = 0; i < 128; i++) {
        sum += *(++ptr4);
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main(void) {
    /* Initialize test data */
    initialize_arrays();
    
    printf("Starting auto-inc-dec pattern tests...\n");
    
    /* Integer tests */
    int int_sum = test_int_postinc_load();
    printf("Integer post-inc load sum: %d\n", int_sum);
    
    test_int_postinc_store(42);
    
    int dec_sum = test_int_postdec_ops();
    printf("Integer post-dec ops sum: %d\n", dec_sum);
    
    /* Float tests */
    float float_sum = test_float_postinc_load();
    printf("Float post-inc load sum: %f\n", float_sum);
    
    test_float_postinc_store(3.14f);
    
    /* Double tests */
    double double_sum = test_double_postinc_load();
    printf("Double post-inc load sum: %f\n", double_sum);
    
    test_double_postdec_store(2.71828);
    
    /* Structure tests */
    double struct_sum = test_struct_traversal();
    printf("Struct traversal sum: %f\n", struct_sum);
    
    test_struct_member_store(1000);
    
    /* Multi-dimensional tests */
    int matrix_sum = test_2d_array_traversal();
    printf("2D array traversal sum: %d\n", matrix_sum);
    
    /* Complex pattern test */
    int mixed_sum = test_mixed_increment_patterns();
    printf("Mixed pattern sum: %d\n", mixed_sum);
    
    /* Verification - compute checksum */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += global_int_array[i];
        checksum += (int)global_float_array[i];
        checksum += (int)global_double_array[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
