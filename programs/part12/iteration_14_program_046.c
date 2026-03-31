/* reload_coverage.c - Program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force complex address calculations and prevent optimizations */
#define NOINLINE __attribute__((noinline, noipa))
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Global volatile variables to prevent constant propagation */
volatile int g_volatile_idx1 = 0;
volatile int g_volatile_idx2 = 0;
volatile int g_volatile_idx3 = 0;
volatile long g_volatile_offset1 = 0;
volatile long g_volatile_offset2 = 0;
volatile int g_volatile_stride = 0;
volatile double g_volatile_scale = 0.0;

NOINLINE double trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile long off1, volatile long off2,
    volatile int stride, volatile double scale)
{
    /* Create high register pressure with many live values */
    int a[100];          /* Integer array - pressure on integer regs */
    double b[50];        /* Double array - pressure on FP regs */
    int c[30];           /* More integer pressure */
    double d[25];        /* More FP pressure */
    
    /* Additional scalars to increase live ranges */
    int scalar1, scalar2, scalar3, scalar4, scalar5;
    double fscalar1, fscalar2, fscalar3, fscalar4;
    
    /* Complex pointer variables for multi-level addressing */
    int *ptr1, *ptr2, **ptr_to_ptr;
    double *fptr1, *fptr2;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) a[i] = i;
    for (int i = 0; i < 50; i++) b[i] = i * 1.5;
    for (int i = 0; i < 30; i++) c[i] = i * 2;
    for (int i = 0; i < 25; i++) d[i] = i * 0.75;
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS ===== */
    /* Complex array access with volatile indices - requires address computation */
    scalar1 = a[idx1 * stride + idx2];
    COMPILER_BARRIER();
    
    /* Multi-dimensional access simulation with pointer arithmetic */
    ptr1 = &a[idx1];
    ptr2 = ptr1 + idx2;  /* Address calculation needing temporary register */
    scalar2 = *ptr2;
    
    /* ===== RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Address of address calculation */
    ptr_to_ptr = &ptr1;
    scalar3 = **ptr_to_ptr;
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS ===== */
    /* Store with complex address calculation */
    a[idx2 * 3 + idx3] = scalar1 + scalar2;
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Complex address in store operation */
    *(ptr1 + idx3) = scalar3;
    
    /* ===== Mixed register class pressure ===== */
    /* Integer to float conversion and computation */
    fscalar1 = (double)scalar1 * scale;
    fscalar2 = (double)scalar2 / scale;
    
    /* Floating point array access with complex addressing */
    fptr1 = &b[idx1];
    fptr2 = fptr1 + idx2;
    fscalar3 = *fptr2 * scale;
    
    COMPILER_BARRIER();
    
    /* ===== Inline assembly to force specific reload types ===== */
    
    /* RELOAD_FOR_OPERAND_ADDRESS - address as input to asm */
    int asm_result1, asm_result2;
    double asm_fresult;
    
    /* Assembly taking memory address as input */
    __asm__ volatile (
        "movl (%1), %0\n\t"          /* Load from memory address */
        : "=r" (asm_result1)         /* Output in register */
        : "r" (&a[idx1 + idx2])      /* Input: computed address in register */
        : "memory"
    );
    
    /* RELOAD_FOR_OPADDR_ADDR - more complex address in asm */
    __asm__ volatile (
        "leal (%1, %2, 4), %0\n\t"   /* Complex address calculation */
        : "=r" (asm_result2)
        : "r" (&a[0]), "r" (idx3)    /* Two inputs for address calc */
        : "cc"
    );
    
    /* Assembly with output address constraint */
    __asm__ volatile (
        "movsd (%1), %%xmm0\n\t"
        "mulsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m" (b[idx3])             /* Output to memory address */
        : "r" (&b[idx2]),            /* Input address */
          "x" (scale)                /* FP register input */
        : "xmm0", "memory"
    );
    
    /* ===== RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS ===== */
    /* Complex pointer chain with multiple indirections */
    int **pptr = (int **)&ptr_to_ptr;
    int ***ppptr = &pptr;
    scalar4 = ***ppptr;
    
    /* Address calculation spanning multiple statements */
    long complex_offset = off1 * 2 + off2 * 3;
    char *char_ptr = (char *)a;
    char_ptr += complex_offset;
    scalar5 = *(int *)char_ptr;
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPUT ===== */
    /* Use all computed values in final expression to keep them live */
    double result = fscalar1 + fscalar2 + fscalar3;
    result += (double)(scalar1 + scalar2 + scalar3 + scalar4 + scalar5);
    result += (double)(asm_result1 + asm_result2);
    result += b[idx3];  /* Final memory access with volatile index */
    
    /* Force all values to be considered live simultaneously */
    COMPILER_BARRIER();
    
    return result * scale;
}

int main(void)
{
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    g_volatile_idx1 = rand() % 50;
    g_volatile_idx2 = rand() % 50;
    g_volatile_idx3 = rand() % 50;
    g_volatile_offset1 = rand() % 100;
    g_volatile_offset2 = rand() % 100;
    g_volatile_stride = (rand() % 10) + 1;
    g_volatile_scale = (rand() % 100) / 10.0 + 0.1;
    
    /* Call the function multiple times with different values */
    double total = 0.0;
    for (int i = 0; i < 10; i++) {
        /* Modify volatiles slightly each iteration */
        g_volatile_idx1 += i;
        g_volatile_idx2 += i * 2;
        g_volatile_idx3 += i * 3;
        
        double result = trigger_reloads(
            g_volatile_idx1, g_volatile_idx2, g_volatile_idx3,
            g_volatile_offset1, g_volatile_offset2,
            g_volatile_stride, g_volatile_scale);
        
        total += result;
        printf("Iteration %d: result = %f\n", i, result);
    }
    
    printf("Total: %f\n", total);
    return 0;
}
