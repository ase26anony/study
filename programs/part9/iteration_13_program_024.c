/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-unit strides */
struct DataStruct {
    int val;
    float fval;
    double dval;
    char padding[8];
};

/* Global arrays to prevent complete optimization */
int global_int_array[ARRAY_SIZE];
float global_float_array[ARRAY_SIZE];
double global_double_array[ARRAY_SIZE];
struct DataStruct global_struct_array[ARRAY_SIZE];

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load() {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_int_postinc_store(int value) {
    int *ptr = global_int_array;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
    return 0;
}

NOINLINE int test_int_postdec_load() {
    int sum = 0;
    int *ptr = &global_int_array[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- in loop - should trigger auto-dec */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE int test_int_postdec_store(int value) {
    int *ptr = &global_int_array[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
    return 0;
}

/* Test with volatile to prevent reordering */
NOINLINE int test_int_volatile_postinc() {
    volatile int *vptr = global_int_array;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    return sum;
}

/* Test with constant stride */
NOINLINE int test_int_stride4_postinc() {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* Access every 4th element with pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *ptr;
        ptr += 4;  /* Constant stride */
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load() {
    float sum = 0.0f;
    float *ptr = global_float_array;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_float_postinc_store(float value) {
    float *ptr = global_float_array;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + (float)i;
    }
    return 0;
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load() {
    double sum = 0.0;
    double *ptr = global_double_array;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_double_postinc_store(double value) {
    double *ptr = global_double_array;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + (double)i;
    }
    return 0;
}

/* ========== STRUCT TESTS ========== */

NOINLINE int test_struct_postinc_load() {
    int sum = 0;
    struct DataStruct *ptr = global_struct_array;
    
    /* Access struct members with pointer increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->val;
        ptr++;  /* Large stride (sizeof(struct DataStruct)) */
    }
    return sum;
}

NOINLINE int test_struct_member_access() {
    float sum = 0.0f;
    struct DataStruct *ptr = global_struct_array;
    
    /* Access only float members */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->fval;
        ptr++;
    }
    return (int)sum;
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal() {
    int arr2d[16][16];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            arr2d[i][j] = i * 16 + j;
        }
    }
    
    /* Traverse with single pointer in row-major order */
    int *ptr = &arr2d[0][0];
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loop_reset() {
    int sum = 0;
    int *ptr;
    
    /* Outer loop resets pointer each iteration */
    for (int outer = 0; outer < 10; outer++) {
        ptr = global_int_array;
        for (int inner = 0; inner < ARRAY_SIZE/10; inner++) {
            sum += *ptr++;
        }
    }
    return sum;
}

/* ========== COMPLEX PATTERNS ========== */

NOINLINE int test_mixed_increment_decrement() {
    int sum = 0;
    int *ptr1 = global_int_array;
    int *ptr2 = &global_int_array[ARRAY_SIZE - 1];
    
    /* Mix increment and decrement in same loop */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += *ptr1++;
        sum += *ptr2--;
    }
    return sum;
}

NOINLINE int test_pointer_arithmetic_with_offset() {
    int sum = 0;
    int *base = global_int_array;
    
    /* Use *(ptr + constant) pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(base + i);  /* Should become base register with offset */
    }
    return sum;
}

/* ========== ARCHITECTURE-SPECIFIC TESTS ========== */

#ifdef __ARM_ARCH
__attribute__((target("arch=armv7-a")))
NOINLINE int test_arm_specific_postinc() {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* ARM-specific pattern with register post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        asm volatile("" : "+r"(ptr) : : "memory");
        sum += *ptr++;
    }
    return sum;
}
#endif

/* ========== MAIN DRIVER ========== */

int main() {
    int total_sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = i;
        global_float_array[i] = (float)i * 1.5f;
        global_double_array[i] = (double)i * 2.5;
        global_struct_array[i].val = i;
        global_struct_array[i].fval = (float)i * 3.5f;
        global_struct_array[i].dval = (double)i * 4.5;
    }
    
    /* Run all tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load();
        test_int_postinc_store(iter);
        total_sum += test_int_postdec_load();
        test_int_postdec_store(iter);
        
        total_sum += (int)test_float_postinc_load();
        test_float_postinc_store((float)iter);
        
        total_sum += (int)test_double_postinc_load();
        test_double_postinc_store((double)iter);
        
        total_sum += test_struct_postinc_load();
        total_sum += test_struct_member_access();
        
        total_sum += test_2d_array_traversal();
        total_sum += test_nested_loop_reset();
        total_sum += test_mixed_increment_decrement();
        total_sum += test_pointer_arithmetic_with_offset();
        
        total_sum += test_int_volatile_postinc();
        total_sum += test_int_stride4_postinc();
        
        #ifdef __ARM_ARCH
        total_sum += test_arm_specific_postinc();
        #endif
    }
    
    /* Use result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    /* Verify array contents */
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += global_int_array[i];
    }
    printf("Array verification sum: %d\n", verify_sum);
    
    return (total_sum > 0 && verify_sum > 0) ? 0 : 1;
}
