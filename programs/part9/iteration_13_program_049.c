/* test_auto_inc_dec.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define MATRIX_SIZE 16

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Target-specific attributes for different architectures */
#ifdef __ARM_ARCH
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

/* Structure for testing structure array traversal */
struct DataPoint {
    int id;
    float value;
    double precision;
    char tag;
};

/* Global arrays to prevent complete optimization */
int global_int_array[ARRAY_SIZE];
float global_float_array[ARRAY_SIZE];
double global_double_array[ARRAY_SIZE];
struct DataPoint global_struct_array[ARRAY_SIZE];
int global_matrix[MATRIX_SIZE][MATRIX_SIZE];

/* ========== BASIC POST-INCREMENT LOAD OPERATIONS ========== */

NOINLINE TARGET_ARM
int test_postinc_load_int(void) {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* Simple post-increment load - should trigger auto-inc recognition */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;  /* Line to target: ptr++ after load */
    }
    
    return sum;
}

NOINLINE TARGET_ARM
float test_postinc_load_float(void) {
    float sum = 0.0f;
    volatile float *vptr = global_float_array;  /* volatile to prevent elimination */
    
    /* Volatile pointer with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* volatile access with increment */
    }
    
    return sum;
}

NOINLINE TARGET_ARM
double test_postinc_load_double(void) {
    double sum = 0.0;
    double *ptr = global_double_array;
    
    /* Double precision with constant stride simulation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr;
        ptr = ptr + 1;  /* Separate increment - may still be recognized */
    }
    
    return sum;
}

/* ========== BASIC POST-INCREMENT STORE OPERATIONS ========== */

NOINLINE TARGET_ARM
void test_postinc_store_int(int value) {
    int *ptr = global_int_array;
    
    /* Post-increment store pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;  /* Store with post-increment */
    }
}

NOINLINE TARGET_ARM
void test_postinc_store_float(float value) {
    volatile float *vptr = global_float_array;
    
    /* Volatile store with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * i;  /* volatile prevents reordering */
    }
}

/* ========== POST-DECREMENT OPERATIONS ========== */

NOINLINE TARGET_ARM
int test_postdec_load_int(void) {
    int sum = 0;
    int *ptr = &global_int_array[ARRAY_SIZE - 1];  /* Start from end */
    
    /* Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;  /* Decrement after load */
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_postdec_store_int(int value) {
    int *ptr = &global_int_array[ARRAY_SIZE - 1];
    
    /* Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;  /* Store with post-decrement */
    }
}

/* ========== POINTER ARITHMETIC WITH CONSTANT STRIDE ========== */

NOINLINE TARGET_ARM
int test_pointer_stride_load(void) {
    int sum = 0;
    int *ptr = global_int_array;
    
    /* Explicit pointer arithmetic with constant stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0);  /* Zero offset - should match mem_insn.reg1_val = 0 */
        ptr += 1;           /* Increment by stride */
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_pointer_stride_store(int value) {
    int *ptr = global_int_array;
    
    /* Store with stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(ptr + 0) = value;  /* Zero offset */
        ptr += 1;
    }
}

/* ========== STRUCTURE ARRAY TRAVERSAL ========== */

NOINLINE TARGET_ARM
double test_struct_array_traversal(void) {
    double sum = 0.0;
    struct DataPoint *ptr = global_struct_array;
    
    /* Access struct members - larger offsets may still trigger base recognition */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->precision;  /* Access member with non-zero offset */
        ptr++;                  /* Increment to next struct */
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_struct_array_store(int base) {
    struct DataPoint *ptr = global_struct_array;
    
    /* Store to struct members */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = base + i;
        ptr->value = (float)(base + i) / 10.0f;
        ptr->precision = (double)(base + i) / 100.0;
        ptr->tag = 'A' + (i % 26);
        ptr++;  /* Post-increment */
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TRAVERSAL ========== */

NOINLINE TARGET_ARM
int test_2d_array_row_major(void) {
    int sum = 0;
    int *ptr = &global_matrix[0][0];  /* Flatten 2D array */
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_2d_array_nested_loops(void) {
    /* Nested loops with pointer reset each iteration */
    for (int row = 0; row < MATRIX_SIZE; row++) {
        int *ptr = global_matrix[row];  /* Reset pointer each outer iteration */
        
        for (int col = 0; col < MATRIX_SIZE; col++) {
            *ptr++ = row * MATRIX_SIZE + col;  /* Inner loop with post-increment */
        }
    }
}

/* ========== MIXED PATTERNS IN SINGLE FUNCTION ========== */

NOINLINE TARGET_ARM
int test_mixed_patterns(void) {
    int sum = 0;
    int *ptr1 = global_int_array;
    volatile int *vptr = global_int_array;
    
    /* Mix of volatile and non-volatile accesses */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += *ptr1++;        /* Non-volatile post-inc */
        sum += *vptr++;        /* Volatile post-inc */
    }
    
    /* Switch to post-decrement */
    int *ptr2 = &global_int_array[ARRAY_SIZE / 2 - 1];
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += *ptr2--;        /* Post-decrement */
    }
    
    return sum;
}

/* ========== COMPLEX POINTER EXPRESSIONS ========== */

NOINLINE TARGET_ARM
int test_complex_pointer_expr(void) {
    int sum = 0;
    int *base = global_int_array;
    
    /* Complex expression that should simplify to base + 0 */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int *ptr = base + (i * 0);  /* Should become base + 0 */
        sum += *ptr;
        base++;  /* Increment base separately */
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = i;
        global_float_array[i] = i * 1.5f;
        global_double_array[i] = i * 2.5;
        global_struct_array[i].id = i;
        global_struct_array[i].value = i * 3.0f;
        global_struct_array[i].precision = i * 4.0;
        global_struct_array[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            global_matrix[i][j] = i * MATRIX_SIZE + j;
        }
    }
    
    /* Execute all test functions */
    checksum += test_postinc_load_int();
    checksum += (int)test_postinc_load_float();
    checksum += (int)test_postinc_load_double();
    
    test_postinc_store_int(42);
    test_postinc_store_float(3.14f);
    
    checksum += test_postdec_load_int();
    test_postdec_store_int(100);
    
    checksum += test_pointer_stride_load();
    test_pointer_stride_store(77);
    
    checksum += (int)test_struct_array_traversal();
    test_struct_array_store(1000);
    
    checksum += test_2d_array_row_major();
    test_2d_array_nested_loops();
    
    checksum += test_mixed_patterns();
    checksum += test_complex_pointer_expr();
    
    /* Verify results by sampling */
    printf("Sample verification:\n");
    printf("  int_array[0] = %d\n", global_int_array[0]);
    printf("  int_array[100] = %d\n", global_int_array[100]);
    printf("  float_array[50] = %f\n", global_float_array[50]);
    printf("  struct_array[10].id = %d\n", global_struct_array[10].id);
    printf("  matrix[5][5] = %d\n", global_matrix[5][5]);
    printf("  Checksum: %d\n", checksum);
    
    return (checksum != 0) ? 0 : 1;  /* Non-zero checksum indicates execution */
}
