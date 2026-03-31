/* test_auto_inc_dec.c
 * Comprehensive test for auto-increment/decrement optimization coverage
 * Targets specific lines in GCC's auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-trivial offsets */
struct DataPoint {
    int id;
    float value;
    double precision;
    char tag;
};

/* Global arrays to prevent complete optimization */
static int global_int_array[1024];
static float global_float_array[1024];
static double global_double_array[1024];
static struct DataPoint global_struct_array[256];

/* Initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < 1024; i++) {
        global_int_array[i] = i;
        global_float_array[i] = i * 1.5f;
        global_double_array[i] = i * 2.5;
    }
    for (int i = 0; i < 256; i++) {
        global_struct_array[i].id = i;
        global_struct_array[i].value = i * 3.0f;
        global_struct_array[i].precision = i * 4.0;
        global_struct_array[i].tag = 'A' + (i % 26);
    }
}

/* ========== INTEGER OPERATIONS ========== */

/* Post-increment load with zero offset pattern */
NOINLINE int test_int_postinc_load(void) {
    volatile int* vptr = global_int_array;
    int sum = 0;
    
    /* Simple pointer increment - should trigger mem_insn with reg1_val = 0 */
    for (int i = 0; i < 256; i++) {
        sum += *vptr++;  /* Post-increment load */
    }
    
    /* Non-volatile version */
    int* ptr = global_int_array + 256;
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Post-increment store with zero offset */
NOINLINE void test_int_postinc_store(int value) {
    volatile int* vptr = global_int_array;
    
    /* Store with post-increment */
    for (int i = 0; i < 128; i++) {
        *vptr++ = value + i;
    }
    
    /* Non-volatile store */
    int* ptr = global_int_array + 128;
    for (int i = 0; i < 128; i++) {
        *ptr++ = value - i;
    }
}

/* Post-decrement operations */
NOINLINE int test_int_postdec(void) {
    volatile int* vptr = &global_int_array[255];
    int sum = 0;
    
    /* Load with post-decrement */
    for (int i = 0; i < 128; i++) {
        sum += *vptr--;
    }
    
    /* Store with post-decrement */
    int* ptr = &global_int_array[383];
    for (int i = 0; i < 128; i++) {
        *ptr-- = sum + i;
    }
    
    return sum;
}

/* Pointer arithmetic with constant stride */
NOINLINE int test_int_stride4(void) {
    int* ptr = global_int_array;
    int sum = 0;
    
    /* Access every 4th element using pointer arithmetic */
    for (int i = 0; i < 64; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr += 4;           /* Pointer increment */
    }
    
    return sum;
}

/* ========== FLOAT OPERATIONS ========== */

NOINLINE float test_float_postinc_load(void) {
    volatile float* vptr = global_float_array;
    float sum = 0.0f;
    
    for (int i = 0; i < 256; i++) {
        sum += *vptr++;
    }
    
    float* ptr = global_float_array + 256;
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float base) {
    volatile float* vptr = global_float_array;
    
    for (int i = 0; i < 128; i++) {
        *vptr++ = base + i * 0.5f;
    }
    
    float* ptr = global_float_array + 128;
    for (int i = 0; i < 128; i++) {
        *ptr++ = base - i * 0.5f;
    }
}

/* ========== DOUBLE OPERATIONS ========== */

NOINLINE double test_double_postinc_load(void) {
    volatile double* vptr = global_double_array;
    double sum = 0.0;
    
    for (int i = 0; i < 256; i++) {
        sum += *vptr++;
    }
    
    double* ptr = global_double_array + 256;
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postinc_store(double base) {
    volatile double* vptr = global_double_array;
    
    for (int i = 0; i < 128; i++) {
        *vptr++ = base + i * 0.25;
    }
    
    double* ptr = global_double_array + 128;
    for (int i = 0; i < 128; i++) {
        *ptr++ = base - i * 0.25;
    }
}

/* ========== STRUCTURE OPERATIONS ========== */

/* Structure traversal with member access - creates non-zero offsets */
NOINLINE int test_struct_traversal(void) {
    volatile struct DataPoint* vptr = global_struct_array;
    int sum = 0;
    
    /* Access structure members - each access has different offset */
    for (int i = 0; i < 64; i++) {
        sum += vptr->id;
        sum += (int)vptr->value;
        vptr++;  /* Pointer increment by structure size */
    }
    
    /* Access specific member in array */
    struct DataPoint* ptr = global_struct_array;
    for (int i = 0; i < 64; i++) {
        ptr->precision = i * 1.5;
        ptr++;
    }
    
    return sum;
}

/* ========== MULTI-DIMENSIONAL ACCESS ========== */

NOINLINE int test_2d_array_traversal(void) {
    int array2d[16][16];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array2d[i][j] = i * 16 + j;
        }
    }
    
    /* Traverse as 1D with single pointer */
    int* ptr = &array2d[0][0];
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    
    /* Nested loop with pointer reset */
    for (int i = 0; i < 16; i++) {
        int* row_ptr = array2d[i];
        for (int j = 0; j < 16; j++) {
            *row_ptr++ += sum;
        }
    }
    
    return sum;
}

/* ========== MIXED PATTERNS ========== */

NOINLINE int test_mixed_patterns(void) {
    int local_array[512];
    int sum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < 512; i++) {
        local_array[i] = i * 3;
    }
    
    /* Mixed increment/decrement */
    int* inc_ptr = &local_array[0];
    int* dec_ptr = &local_array[511];
    
    for (int i = 0; i < 128; i++) {
        sum += *inc_ptr++;  /* Post-increment */
        sum += *dec_ptr--;  /* Post-decrement */
    }
    
    /* Pointer with constant offset in loop */
    int* base_ptr = local_array;
    for (int i = 0; i < 64; i++) {
        sum += *(base_ptr + 8);  /* Constant offset */
        base_ptr += 8;
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main(void) {
    int int_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Initialize data */
    init_arrays();
    
    /* Execute all test patterns */
    int_result += test_int_postinc_load();
    test_int_postinc_store(100);
    int_result += test_int_postdec();
    int_result += test_int_stride4();
    
    float_result += test_float_postinc_load();
    test_float_postinc_store(50.0f);
    
    double_result += test_double_postinc_load();
    test_double_postinc_store(100.0);
    
    int_result += test_struct_traversal();
    int_result += test_2d_array_traversal();
    int_result += test_mixed_patterns();
    
    /* Verification output (prevents dead code elimination) */
    printf("Results: int=%d, float=%.2f, double=%.2f\n", 
           int_result, float_result, double_result);
    
    /* Additional volatile access to ensure side effects */
    volatile int check = global_int_array[0] + 
                       (int)global_float_array[0] + 
                       (int)global_double_array[0];
    
    return (int_result > 0) ? 0 : 1;
}
