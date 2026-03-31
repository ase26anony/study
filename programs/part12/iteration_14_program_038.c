/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int v_idx1, v_idx2, v_idx3, v_idx4, v_stride1, v_stride2;
volatile long v_offset1, v_offset2, v_offset3;
volatile double v_scale1, v_scale2;

/* Force register pressure with many live values */
#define NOINLINE __attribute__((noinline, noipa))

/* Complex addressing function that triggers multiple reload types */
NOINLINE static double trigger_reloads(
    int idx1, int idx2, int idx3, int idx4,
    int stride1, int stride2,
    long offset1, long offset2, long offset3,
    double scale1, double scale2)
{
    /* Local arrays to create register pressure */
    int int_arr[128];      /* Integer register pressure */
    double dbl_arr[64];    /* FP register pressure */
    long long_arr[96];     /* More integer pressure */
    float flt_arr[80];     /* Mixed FP pressure */
    
    /* Complex pointer chains */
    int *ptr1, *ptr2, **ptr_to_ptr;
    double *dptr1, *dptr2;
    long *lptr1, *lptr2;
    
    /* Intermediate values with overlapping live ranges */
    int temp1, temp2, temp3, temp4, temp5;
    double dtemp1, dtemp2, dtemp3;
    long ltemp1, ltemp2;
    
    /* Initialize arrays */
    for (int i = 0; i < 128; i++) int_arr[i] = i * 3;
    for (int i = 0; i < 64; i++) dbl_arr[i] = i * 1.5;
    for (int i = 0; i < 96; i++) long_arr[i] = i * 5L;
    for (int i = 0; i < 80; i++) flt_arr[i] = i * 0.7f;
    
    /* Compiler barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS ===== */
    /* Complex multi-dimensional addressing with volatile indices */
    temp1 = int_arr[idx1 * stride1 + idx2];
    temp2 = int_arr[idx3 * stride2 + idx4];
    
    /* Pointer chain with arithmetic */
    ptr1 = &int_arr[idx1];
    ptr2 = ptr1 + idx2;
    temp3 = *ptr2;
    
    /* More complex: *(*(base + offset1) + offset2) style */
    ptr_to_ptr = &ptr1;
    temp4 = **ptr_to_ptr;
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS ===== */
    /* Store with complex address computation */
    int_arr[idx1 * stride1 + idx2 * stride2] = temp1 + temp2;
    
    /* Pointer store with offset */
    lptr1 = &long_arr[idx3];
    *(lptr1 + idx4) = offset1 + offset2;
    
    /* ===== Mixed register class pressure ===== */
    /* Integer to float conversion and back */
    dtemp1 = (double)temp1 * scale1;
    dtemp2 = (double)temp2 * scale2;
    
    /* FP array access with complex addressing */
    dtemp3 = dbl_arr[idx1] * dbl_arr[idx2];
    
    /* Convert back to integer */
    temp5 = (int)(dtemp1 + dtemp2 + dtemp3);
    
    /* ===== Inline assembly to force specific reloads ===== */
    /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int asm_out1, asm_out2;
    long asm_out3;
    
    /* Assembly with memory input and register output */
    __asm__ volatile(
        "movl (%1), %0\n\t"
        : "=r" (asm_out1)
        : "r" (&int_arr[idx1 + idx2])
        : "memory"
    );
    
    /* Assembly with complex address input */
    __asm__ volatile(
        "movq (%1, %2, 4), %0\n\t"
        : "=r" (asm_out3)
        : "r" (long_arr), "r" (offset1)
        : "memory"
    );
    
    /* Assembly that clobbers many registers */
    __asm__ volatile(
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=&r" (asm_out2)
        : "r" (temp3), "r" (temp4)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
    
    /* ===== More complex addressing modes ===== */
    /* Different pointer types and scales */
    char *cptr = (char *)int_arr;
    cptr += offset1 * sizeof(int) + offset2;
    temp1 = *(int *)cptr;
    
    /* Multi-level indirection */
    int **pptr = &ptr1;
    int ***ppptr = &pptr;
    temp2 = ***ppptr;
    
    /* ===== RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Address of address computation */
    int *addr_of_addr = &int_arr[idx1 * stride1];
    temp3 = *addr_of_addr;
    
    /* ===== RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Store through computed address pointer */
    int *store_ptr = &int_arr[idx2 * stride2];
    *store_ptr = temp1 + temp2 + temp3;
    
    /* ===== RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS ===== */
    /* Complex expression with multiple memory accesses */
    double result = 
        dbl_arr[idx1] * scale1 +
        dbl_arr[idx2] * scale2 +
        (double)int_arr[idx3] / scale1 +
        (double)long_arr[idx4] / scale2;
    
    /* Mix integer and FP operations */
    for (int i = 0; i < 16; i++) {
        /* Force register pressure with many live values */
        int_arr[i] = (int)(dbl_arr[i] * (double)long_arr[i]);
        dbl_arr[i] = (double)int_arr[i] * scale1;
        long_arr[i] = (long)(dbl_arr[i] * scale2);
        
        /* Compiler barrier to extend live ranges */
        if (i % 4 == 0) {
            __asm__ volatile("" : : : "memory");
        }
    }
    
    /* Final complex computation using all variables */
    result += (double)(asm_out1 + asm_out2 + asm_out3) * 0.5;
    result += (double)(temp1 * temp2 * temp3 * temp4 * temp5) * 0.01;
    
    /* Use volatile to force all computations */
    volatile double final_result = result;
    
    return final_result;
}

int main(void) {
    /* Initialize volatile variables with random values */
    srand(42);
    
    v_idx1 = rand() % 32;
    v_idx2 = rand() % 32;
    v_idx3 = rand() % 32;
    v_idx4 = rand() % 32;
    v_stride1 = 32 + (rand() % 16);
    v_stride2 = 32 + (rand() % 16);
    v_offset1 = rand() % 64;
    v_offset2 = rand() % 64;
    v_offset3 = rand() % 64;
    v_scale1 = 1.0 + (rand() % 100) / 100.0;
    v_scale2 = 1.0 + (rand() % 100) / 100.0;
    
    /* Call the function multiple times to ensure coverage */
    double total = 0.0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(
            v_idx1 + i, v_idx2 + i, v_idx3 + i, v_idx4 + i,
            v_stride1, v_stride2,
            v_offset1 + i, v_offset2 + i, v_offset3 + i,
            v_scale1, v_scale2
        );
    }
    
    printf("Result: %f\n", total);
    
    return 0;
}
