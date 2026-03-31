/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might simplify addressing */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Force register pressure with many live values */
#define FORCE_SPILL(var) __asm__ volatile("" : "+r"(var))

/* Target function with maximum register pressure */
__attribute__((noinline, noipa, optimize("O2")))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8, volatile double v9)
{
    /* Create high register pressure with many live variables */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    int local3 = v4 ^ v5;
    long local4 = v7 >> 3;
    long local5 = v8 * 2;
    double local6 = v9 * 3.14;
    double local7 = v9 / 2.71;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[4][8][16];
    double dbl_arr[32][8];
    char* char_ptr_arr[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                arr3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            dbl_arr[i][j] = i * 1.5 + j * 0.25;
        }
    }
    
    /* Complex pointer chains for address reloads */
    int* ptr1 = &arr3d[v1 % 4][v2 % 8][0];
    int* ptr2 = ptr1 + (v3 % 16);
    int** ptr_to_ptr = &ptr2;
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex addressing mode */
    int val1 = *(*(ptr_to_ptr) + (v4 % 8));
    FORCE_SPILL(val1);
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Store with complex address */
    int*** ptr3d = (int***)char_ptr_arr;
    ptr3d[v5 % 32] = ptr_to_ptr;
    
    /* Mixed register class operations */
    double fp_result = 0.0;
    for (int i = 0; i < (v6 % 8); i++) {
        /* Integer to float conversion causing register class pressure */
        fp_result += (double)arr3d[i][i % 4][i % 8] * dbl_arr[i][i % 4];
    }
    
    /* Inline assembly to force specific reload types */
    int asm_result1, asm_result2;
    long asm_addr;
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Memory operand in assembly */
    __asm__ volatile (
        "mov %[addr], %[mem]\n\t"
        "add $1, %[out1]\n\t"
        : [out1] "=&r" (asm_result1), [addr] "=&r" (asm_addr)
        : [mem] "m" (*ptr2)
        : "cc"
    );
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Output address in assembly */
    int output_buffer[4];
    __asm__ volatile (
        "lea %[buf], %[addr]\n\t"
        "movl $0x12345678, (%[addr])\n\t"
        : [addr] "=&r" (asm_addr)
        : [buf] "m" (output_buffer)
        : "memory"
    );
    
    /* More complex addressing with multiple levels */
    int**** complex_ptr = (int****)malloc(sizeof(int***) * 4);
    for (int i = 0; i < 4; i++) {
        complex_ptr[i] = (int***)malloc(sizeof(int**) * 4);
        for (int j = 0; j < 4; j++) {
            complex_ptr[i][j] = (int**)malloc(sizeof(int*) * 4);
        }
    }
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Address of input address */
    int***** ptr_to_complex = &complex_ptr;
    int deep_value = ****(*ptr_to_complex + (v7 % 3));
    
    /* RELOAD_FOR_OTHER_ADDRESS: Unusual address computation */
    uintptr_t base_addr = (uintptr_t)arr3d;
    uintptr_t offset_addr = base_addr + (v8 % 256) * sizeof(int);
    int* computed_ptr = (int*)offset_addr;
    int computed_val = *computed_ptr;
    
    /* Force all variables to be live simultaneously */
    COMPILER_BARRIER();
    
    /* Use all variables to prevent optimization */
    int result = val1 + asm_result1 + deep_value + computed_val + local1 + 
                 local2 + local3 + (int)local4 + (int)local5 + (int)fp_result;
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            free(complex_ptr[i][j]);
        }
        free(complex_ptr[i]);
    }
    free(complex_ptr);
    
    return result;
}

/* Helper to create volatile values */
__attribute__((noinline))
static void init_volatiles(volatile int* vars, int count) {
    for (int i = 0; i < count; i++) {
        vars[i] = rand() % 100;
    }
}

int main(void) {
    /* Initialize random seed */
    srand(42);
    
    /* Create volatile variables to prevent constant propagation */
    volatile int v[9];
    volatile long vl[2];
    volatile double vd[1];
    
    init_volatiles(v, 9);
    vl[0] = rand() % 1000;
    vl[1] = rand() % 1000;
    vd[0] = (double)(rand() % 1000) / 10.0;
    
    /* Call function with many volatile arguments */
    int result = trigger_reloads(v[0], v[1], v[2], v[3], v[4], 
                                 v[5], vl[0], vl[1], vd[0]);
    
    printf("Result: %d\n", result);
    
    /* Additional test with different patterns */
    v[0] = rand() % 50;
    v[1] = rand() % 50;
    result += trigger_reloads(v[1], v[0], v[2], v[3], v[4], 
                              v[5], vl[0], vl[1], vd[0]);
    
    printf("Final result: %d\n", result);
    return 0;
}
