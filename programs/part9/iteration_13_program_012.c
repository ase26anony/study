/* test_auto_inc_dec.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Target-specific attributes for different architectures */
#ifdef __ARM_ARCH
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

/* Global arrays to prevent complete optimization */
static int int_array[ARRAY_SIZE];
static float float_array[ARRAY_SIZE];
static double double_array[ARRAY_SIZE];
static TestStruct struct_array[ARRAY_SIZE];

/* ========== INTEGER TESTS ========== */

NOINLINE TARGET_ARM
void test_int_postinc_load() {
    volatile int* vptr = int_array;
    int sum = 0;
    
    /* Simple post-increment load with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Prevent dead code elimination */
    int_array[0] = sum;
}

NOINLINE TARGET_ARM
void test_int_postinc_store() {
    int* ptr = int_array;
    
    /* Post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = i * 2;
    }
}

NOINLINE TARGET_ARM
void test_int_postdec_load() {
    int* ptr = &int_array[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    
    int_array[1] = sum;
}

NOINLINE TARGET_ARM
void test_int_postdec_store() {
    int* ptr = &int_array[ARRAY_SIZE - 1];
    
    /* Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = i * 3;
    }
}

NOINLINE TARGET_ARM
void test_int_pointer_arithmetic() {
    int* ptr = int_array;
    
    /* Pointer arithmetic with constant stride */
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        *(ptr + 0) = i;
        *(ptr + 1) = i + 1;
        *(ptr + 2) = i + 2;
        *(ptr + 3) = i + 3;
        ptr += 4;
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE TARGET_ARM
void test_float_postinc_load() {
    volatile float* vptr = float_array;
    float sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    float_array[0] = sum;
}

NOINLINE TARGET_ARM
void test_float_postinc_store() {
    float* ptr = float_array;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = i * 1.5f;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE TARGET_ARM
void test_double_postinc_load() {
    volatile double* vptr = double_array;
    double sum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    double_array[0] = sum;
}

NOINLINE TARGET_ARM
void test_double_postinc_store() {
    double* ptr = double_array;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = i * 2.5;
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE TARGET_ARM
void test_struct_member_access() {
    TestStruct* ptr = struct_array;
    
    /* Access struct members with non-trivial offsets */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->val = i;
        ptr->fval = i * 1.1f;
        ptr->dval = i * 2.2;
        ptr++;
    }
}

NOINLINE TARGET_ARM
void test_struct_pointer_arithmetic() {
    TestStruct* ptr = struct_array;
    
    /* Explicit pointer arithmetic with structure size */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        (ptr + i)->val = i * 2;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE TARGET_ARM
void test_2d_array_traversal() {
    int matrix[16][16];
    int* ptr = &matrix[0][0];
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < 16 * 16; i++) {
        *ptr++ = i;
    }
}

NOINLINE TARGET_ARM
void test_nested_loop_reset() {
    int data[8][32];
    
    /* Nested loops with pointer reset each iteration */
    for (int i = 0; i < 8; i++) {
        int* ptr = data[i];
        for (int j = 0; j < 32; j++) {
            *ptr++ = i * 32 + j;
        }
    }
}

/* ========== MIXED PATTERN TESTS ========== */

NOINLINE TARGET_ARM
void test_mixed_load_store() {
    int* src = int_array;
    int* dst = &int_array[ARRAY_SIZE/2];
    
    /* Mixed load/store with post-increment */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        int val = *src++;
        *dst++ = val * 2;
    }
}

NOINLINE TARGET_ARM
void test_volatile_nonvolatile_mix() {
    volatile int* vptr = int_array;
    int* ptr = &int_array[ARRAY_SIZE/2];
    
    /* Mix volatile and non-volatile accesses */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        int temp = *vptr++;  /* volatile load */
        *ptr++ = temp;       /* non-volatile store */
    }
}

/* ========== COMPLEX LOOP PATTERNS ========== */

NOINLINE TARGET_ARM
void test_unrolled_loop() {
    int* ptr = int_array;
    
    /* Manually unrolled loop */
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        *ptr++ = i;
        *ptr++ = i + 1;
        *ptr++ = i + 2;
        *ptr++ = i + 3;
    }
}

NOINLINE TARGET_ARM
void test_while_loop_postinc() {
    int* ptr = int_array;
    int count = ARRAY_SIZE;
    
    /* While loop with post-increment */
    while (count--) {
        *ptr++ = count;
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.5f;
        double_array[i] = i * 0.25;
        struct_array[i].val = i;
        struct_array[i].fval = i * 0.1f;
        struct_array[i].dval = i * 0.01;
    }
    
    /* Run tests multiple times to ensure pattern recognition */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        test_int_postinc_load();
        test_int_postinc_store();
        test_int_postdec_load();
        test_int_postdec_store();
        test_int_pointer_arithmetic();
        
        /* Float tests */
        test_float_postinc_load();
        test_float_postinc_store();
        
        /* Double tests */
        test_double_postinc_load();
        test_double_postinc_store();
        
        /* Structure tests */
        test_struct_member_access();
        test_struct_pointer_arithmetic();
        
        /* Multi-dimensional tests */
        test_2d_array_traversal();
        test_nested_loop_reset();
        
        /* Mixed pattern tests */
        test_mixed_load_store();
        test_volatile_nonvolatile_mix();
        
        /* Complex patterns */
        test_unrolled_loop();
        test_while_loop_postinc();
    }
    
    /* Verify results */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_array[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return 0;
}
