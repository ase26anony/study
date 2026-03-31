/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prevent excessive inlining that might obscure patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-trivial offsets */
struct TestStruct {
    int id;
    float value;
    double data;
    char padding[8];
};

/* Global arrays to prevent stack optimizations */
static int global_int_array[1024];
static float global_float_array[1024];
static double global_double_array[1024];
static struct TestStruct global_struct_array[256];

/* Initialize arrays with non-zero patterns */
static void initialize_arrays(void) {
    for (int i = 0; i < 1024; i++) {
        global_int_array[i] = i * 3 + 1;
        global_float_array[i] = i * 1.5f;
        global_double_array[i] = i * 2.5;
    }
    for (int i = 0; i < 256; i++) {
        global_struct_array[i].id = i;
        global_struct_array[i].value = i * 0.75f;
        global_struct_array[i].data = i * 1.25;
    }
}

/* ========== INTEGER TESTS ========== */

/* Post-increment load with simple pointer */
NOINLINE int test_int_postinc_load(void) {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* Fixed iteration count for predictable pattern */
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;  /* Should trigger auto-inc pattern */
    }
    return sum;
}

/* Post-increment store with simple pointer */
NOINLINE void test_int_postinc_store(int value) {
    int *ptr = global_int_array;
    
    for (int i = 0; i < 256; i++) {
        *ptr++ = value + i;  /* Store with post-increment */
    }
}

/* Post-decrement load */
NOINLINE int test_int_postdec_load(void) {
    int sum = 0;
    int *ptr = &global_int_array[255];  /* Start from end */
    
    for (int i = 0; i < 256; i++) {
        sum += *ptr--;  /* Post-decrement load */
    }
    return sum;
}

/* Post-decrement store */
NOINLINE void test_int_postdec_store(int value) {
    int *ptr = &global_int_array[255];
    
    for (int i = 0; i < 256; i++) {
        *ptr-- = value - i;  /* Store with post-decrement */
    }
}

/* Pointer arithmetic with constant stride */
NOINLINE int test_int_stride4_load(void) {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* Access every 4th element */
    for (int i = 0; i < 64; i++) {
        sum += *(ptr + 4);  /* Constant offset */
        ptr += 4;           /* Explicit stride */
    }
    return sum;
}

/* Volatile pointer access - prevents aggressive optimization */
NOINLINE int test_int_volatile_load(void) {
    int sum = 0;
    volatile int *vptr = global_int_array;
    
    for (int i = 0; i < 256; i++) {
        sum += *vptr++;  /* Volatile ensures memory access */
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(void) {
    float sum = 0.0f;
    float *ptr = global_float_array;
    
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;  /* Float post-increment */
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float base) {
    float *ptr = global_float_array;
    
    for (int i = 0; i < 256; i++) {
        *ptr++ = base + i * 0.1f;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(void) {
    double sum = 0.0;
    double *ptr = global_double_array;
    
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;  /* Double post-increment */
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double base) {
    double *ptr = global_double_array;
    
    for (int i = 0; i < 256; i++) {
        *ptr++ = base + i * 0.01;
    }
}

/* ========== STRUCTURE TESTS ========== */

/* Access struct members with non-trivial offsets */
NOINLINE double test_struct_member_access(void) {
    double sum = 0.0;
    struct TestStruct *ptr = global_struct_array;
    
    for (int i = 0; i < 128; i++) {
        /* Access different members - creates complex offset patterns */
        sum += ptr->id + ptr->value + ptr->data;
        ptr++;  /* Pointer moves by sizeof(struct TestStruct) */
    }
    return sum;
}

/* Access only one member across array */
NOINLINE float test_struct_single_member(void) {
    float sum = 0.0f;
    struct TestStruct *ptr = global_struct_array;
    
    for (int i = 0; i < 128; i++) {
        sum += ptr->value;  /* Constant offset within struct */
        ptr++;
    }
    return sum;
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(void) {
    int local_2d[32][32];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            local_2d[i][j] = i * 32 + j;
        }
    }
    
    /* Traverse with single pointer in row-major order */
    int *ptr = &local_2d[0][0];
    for (int i = 0; i < 1024; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* Nested loop with pointer reset */
NOINLINE int test_nested_loop_reset(void) {
    int sum = 0;
    int matrix[8][16];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Outer loop resets pointer each iteration */
    for (int row = 0; row < 8; row++) {
        int *ptr = matrix[row];  /* Reset each outer iteration */
        for (int col = 0; col < 16; col++) {
            sum += *ptr++;  /* Inner loop auto-inc pattern */
        }
    }
    return sum;
}

/* ========== MIXED PATTERN TESTS ========== */

/* Mixed load/store pattern */
NOINLINE void test_mixed_load_store(void) {
    int buffer[256];
    int *read_ptr = global_int_array;
    int *write_ptr = buffer;
    
    /* Copy with post-increment on both sides */
    for (int i = 0; i < 256; i++) {
        *write_ptr++ = *read_ptr++;
    }
    
    /* Verify copy */
    int verify = 0;
    for (int i = 0; i < 256; i++) {
        verify += buffer[i];
    }
    /* Use result to prevent dead code elimination */
    global_int_array[0] = verify;
}

/* Interleaved increment/decrement */
NOINLINE int test_interleaved_inc_dec(void) {
    int sum = 0;
    int *inc_ptr = global_int_array;
    int *dec_ptr = &global_int_array[255];
    
    for (int i = 0; i < 128; i++) {
        sum += *inc_ptr++;  /* Post-increment */
        sum += *dec_ptr--;  /* Post-decrement */
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main(void) {
    int int_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    initialize_arrays();
    
    /* Execute all test patterns */
    int_result += test_int_postinc_load();
    test_int_postinc_store(42);
    int_result += test_int_postdec_load();
    test_int_postdec_store(100);
    int_result += test_int_stride4_load();
    int_result += test_int_volatile_load();
    
    float_result += test_float_postinc_load();
    test_float_postinc_store(3.14f);
    
    double_result += test_double_postinc_load();
    test_double_postinc_store(2.71828);
    
    double_result += test_struct_member_access();
    float_result += test_struct_single_member();
    
    int_result += test_2d_array_traversal();
    int_result += test_nested_loop_reset();
    
    test_mixed_load_store();
    int_result += test_interleaved_inc_dec();
    
    /* Print results to prevent optimization */
    printf("Results: int=%d, float=%.2f, double=%.2f\n", 
           int_result, float_result, double_result);
    
    /* Verify with simple checksum */
    int checksum = int_result + (int)float_result + (int)double_result;
    if (checksum != 0) {
        printf("Checksum: %d\n", checksum);
    }
    
    return 0;
}
