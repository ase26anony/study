/* test_auto_inc_dec.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-unit strides */
struct test_struct {
    int val;
    float fval;
    double dval;
    char padding[32];  /* Force larger stride */
};

/* Global arrays to prevent complete optimization */
int global_int_array[ARRAY_SIZE];
float global_float_array[ARRAY_SIZE];
double global_double_array[ARRAY_SIZE];
struct test_struct global_struct_array[ARRAY_SIZE];

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load() {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* Pattern: *ptr++ with zero offset initially */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_int_postinc_store(int value) {
    int *ptr = global_int_array;
    
    /* Pattern: *ptr++ = value */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
    return 0;
}

NOINLINE int test_int_postdec_load() {
    int sum = 0;
    int *ptr = &global_int_array[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE int test_int_postdec_store(int value) {
    int *ptr = &global_int_array[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- = value */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
    return 0;
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

/* ========== VOLATILE TESTS ========== */

NOINLINE int test_volatile_postinc() {
    volatile int *vptr = global_int_array;
    int sum = 0;
    
    /* Volatile access should still trigger auto-inc pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    return sum;
}

NOINLINE int test_mixed_volatile_nonvolatile() {
    int sum = 0;
    int *ptr = global_int_array;
    volatile int *vptr = &global_int_array[128];
    
    /* Mix volatile and non-volatile accesses */
    for (int i = 0; i < 128; i++) {
        sum += *ptr++;      /* Non-volatile */
        sum += *vptr++;     /* Volatile */
    }
    return sum;
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE int test_struct_member_access() {
    int sum = 0;
    
    /* Access struct member with constant offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += global_struct_array[i].val;
    }
    return sum;
}

NOINLINE int test_struct_pointer_arithmetic() {
    int sum = 0;
    struct test_struct *ptr = global_struct_array;
    
    /* Pointer arithmetic with non-unit stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->val;
        ptr++;  /* Large stride due to struct size */
    }
    return sum;
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_row_major() {
    int arr2d[16][16];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            arr2d[i][j] = i * 16 + j;
        }
    }
    
    /* Traverse with single pointer - row-major order */
    int *ptr = &arr2d[0][0];
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loop_reset() {
    int sum = 0;
    int outer_arr[10][ARRAY_SIZE];
    
    /* Nested loops with pointer reset each iteration */
    for (int i = 0; i < 10; i++) {
        int *ptr = outer_arr[i];
        for (int j = 0; j < ARRAY_SIZE; j++) {
            *ptr++ = i * j;
            sum += outer_arr[i][j];
        }
    }
    return sum;
}

/* ========== CONSTANT STRIDE TESTS ========== */

NOINLINE int test_constant_stride_load() {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* Access with constant stride of 2 */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += *ptr;
        ptr += 2;  /* Constant stride */
    }
    return sum;
}

NOINLINE int test_constant_stride_store() {
    int *ptr = global_int_array;
    
    /* Store with constant stride */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        *ptr = i * 2;
        ptr += 2;
    }
    return 0;
}

/* ========== COMPLEX PATTERNS ========== */

NOINLINE int test_interleaved_load_store() {
    int temp[ARRAY_SIZE];
    int *src = global_int_array;
    int *dst = temp;
    
    /* Interleaved pattern that might still trigger auto-inc */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int val = *src++;
        *dst++ = val * 2;
    }
    
    /* Verify copy */
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += temp[i];
    }
    return sum;
}

NOINLINE int test_pointer_arithmetic_with_offset() {
    int sum = 0;
    int *base = global_int_array;
    
    /* *(ptr + constant) pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(base + i);  /* Should become base register with offset */
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    int total_checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = i;
        global_float_array[i] = (float)i;
        global_double_array[i] = (double)i;
        global_struct_array[i].val = i * 2;
        global_struct_array[i].fval = (float)i * 3.0f;
        global_struct_array[i].dval = (double)i * 4.0;
    }
    
    /* Run all tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_checksum += test_int_postinc_load();
        test_int_postinc_store(iter);
        total_checksum += test_int_postdec_load();
        test_int_postdec_store(iter);
        
        total_checksum += (int)test_float_postinc_load();
        test_float_postinc_store((float)iter);
        
        total_checksum += (int)test_double_postinc_load();
        test_double_postinc_store((double)iter);
        
        total_checksum += test_volatile_postinc();
        total_checksum += test_mixed_volatile_nonvolatile();
        
        total_checksum += test_struct_member_access();
        total_checksum += test_struct_pointer_arithmetic();
        
        total_checksum += test_2d_array_row_major();
        total_checksum += test_nested_loop_reset();
        
        total_checksum += test_constant_stride_load();
        test_constant_stride_store();
        
        total_checksum += test_interleaved_load_store();
        total_checksum += test_pointer_arithmetic_with_offset();
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("All tests completed. Compile with appropriate flags to trigger auto-inc-dec optimization.\n");
    
    /* Suggested compilation flags for coverage: */
    printf("\n=== Suggested compilation commands ===\n");
    printf("ARM: arm-linux-gnueabihf-gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec test.c -o test_arm\n");
    printf("x86: gcc -O3 -fdump-rtl-auto_inc_dec -fno-tree-vectorize test.c -o test_x86\n");
    printf("PowerPC: gcc -O2 -march=powerpc64le -fdump-rtl-auto_inc_dec test.c -o test_ppc\n");
    printf("With debug: gcc -O1 -da -fdump-rtl-all -march=armv7-a test.c -o test_debug\n");
    
    return 0;
}
