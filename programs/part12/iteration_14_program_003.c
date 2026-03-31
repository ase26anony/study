/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline, noipa))
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Global volatile variables to prevent optimization */
volatile int g_volatile_idx1 = 0;
volatile int g_volatile_idx2 = 0;
volatile int g_volatile_idx3 = 0;
volatile int g_volatile_stride = 0;
volatile double g_volatile_scale = 0.0;
volatile long g_volatile_offset1 = 0;
volatile long g_volatile_offset2 = 0;

/* Complex addressing structure */
struct MultiLevel {
    int *level1;
    int **level2;
    int ***level3;
};

/* Function to trigger multiple reload types */
NOINLINE static double trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride, volatile double scale,
    volatile long off1, volatile long off2)
{
    /* Create high register pressure with many live values */
    int a[100];                     /* Integer array - pressure on integer regs */
    double b[50];                   /* Double array - pressure on FP regs */
    int c[75];                      /* More integer pressure */
    double d[40];                   /* More FP pressure */
    
    /* Complex pointer variables with different types */
    char *char_ptr;
    int *int_ptr;
    long *long_ptr;
    double *double_ptr;
    
    /* Multi-level pointers */
    int **ptr2;
    int ***ptr3;
    
    /* Temporary results */
    int temp_int[10];
    double temp_double[10];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 100; i++) a[i] = rand() % 1000;
    for (int i = 0; i < 50; i++) b[i] = (double)(rand() % 1000) / 10.0;
    for (int i = 0; i < 75; i++) c[i] = rand() % 1000;
    for (int i = 0; i < 40; i++) d[i] = (double)(rand() % 1000) / 10.0;
    
    COMPILER_BARRIER();
    
    /* ===== SCENARIO 1: RELOAD_FOR_INPUT_ADDRESS ===== */
    /* Complex multi-dimensional array access with volatile indices */
    /* This forces address computation into temporary registers */
    int result1 = a[idx1 * stride + idx2];
    result1 += a[idx2 * stride + idx3];
    result1 += a[idx3 * stride + idx1];
    
    /* More complex: arr[volatile_idx1][volatile_idx2 + volatile_idx3] pattern */
    /* Using pointer arithmetic to simulate 2D array */
    int *row_ptr = a + (idx1 * 10);  /* Assume 10 columns */
    int result2 = row_ptr[idx2 + idx3];
    result2 += row_ptr[idx1 + idx3];
    
    COMPILER_BARRIER();
    
    /* ===== SCENARIO 2: RELOAD_FOR_OUTPUT_ADDRESS ===== */
    /* Complex address computation for store operations */
    /* Store to computed address with volatile offset */
    int_ptr = a + off1;
    *int_ptr = result1 + result2;  /* Store to computed address */
    
    /* Chain of pointer dereferences */
    ptr2 = &int_ptr;
    **ptr2 = result1 - result2;    /* Another store through pointer chain */
    
    COMPILER_BARRIER();
    
    /* ===== SCENARIO 3: RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Inline assembly that takes complex address as input */
    int asm_input = 0;
    int asm_output = 0;
    
    /* Assembly with memory input and register output */
    __asm__ volatile (
        "movl (%1), %0\n\t"        /* Load from memory address */
        : "=r" (asm_output)        /* Output in register */
        : "r" (&a[idx1 + idx2])    /* Input: complex address in register */
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* ===== SCENARIO 4: RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Assembly that outputs to memory address */
    int asm_value = 42;
    
    __asm__ volatile (
        "movl %1, (%0)\n\t"        /* Store to memory address */
        :                         /* No outputs */
        : "r" (&c[idx2 + idx3]),  /* Output address in register */
          "r" (asm_value)         /* Value to store */
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* ===== SCENARIO 5: RELOAD_FOR_OPERAND_ADDRESS ===== */
    /* Mixed integer/float operations with address computations */
    double_ptr = b + idx1;
    double temp1 = *double_ptr * scale;
    
    /* Complex address for float array */
    double_ptr = d + (idx2 * 5 + idx3);
    double temp2 = *double_ptr / scale;
    
    /* Convert between types - forces moves between register classes */
    temp_int[0] = (int)temp1;
    temp_int[1] = (int)temp2;
    
    COMPILER_BARRIER();
    
    /* ===== SCENARIO 6: RELOAD_FOR_OPADDR_ADDR ===== */
    /* Pointer chains with different types */
    char_ptr = (char *)a;
    char_ptr += off1 * sizeof(int);  /* Byte offset calculation */
    
    int_ptr = (int *)char_ptr;
    *int_ptr = asm_output;
    
    /* Another level of indirection */
    long_ptr = (long *)&int_ptr;
    **((int **)long_ptr) = *int_ptr + 1;
    
    COMPILER_BARRIER();
    
    /* ===== SCENARIO 7: RELOAD_FOR_OTHER_ADDRESS ===== */
    /* Complex addressing with multiple volatile components */
    for (int i = 0; i < 10; i++) {
        /* Varying address calculation in loop */
        int index = (idx1 * i + idx2) % 100;
        temp_double[i] = b[index] * scale + d[(idx3 + i) % 40];
        
        /* Store with complex address */
        int store_idx = (off1 + i * off2) % 100;
        a[store_idx] = (int)temp_double[i];
    }
    
    COMPILER_BARRIER();
    
    /* ===== SCENARIO 8: RELOAD_OTHER ===== */
    /* Mixed operations creating various reload needs */
    double final_result = 0.0;
    
    /* Force spills by using many temporaries */
    for (int i = 0; i < 20; i++) {
        double t1 = b[(idx1 + i) % 50];
        double t2 = d[(idx2 + i) % 40];
        int t3 = a[(idx3 + i) % 100];
        int t4 = c[(off1 + i) % 75];
        
        /* Mixed-type computation */
        final_result += t1 * t2 + (double)t3 / (double)(t4 + 1);
        
        /* Complex address store */
        int *store_ptr = a + ((off1 * i + off2) % 100);
        *store_ptr = (int)(t1 + t2 + t3 + t4);
    }
    
    COMPILER_BARRIER();
    
    /* Additional inline assembly with multiple constraints */
    int asm_in1 = a[idx1];
    int asm_in2 = c[idx2];
    int asm_out1, asm_out2;
    
    __asm__ volatile (
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "imull %2, %3\n\t"
        "movl %3, %1\n\t"
        : "=r" (asm_out1), "=r" (asm_out2)
        : "r" (asm_in1), "r" (asm_in2)
        : "%eax", "memory"
    );
    
    /* Use results to prevent elimination */
    final_result += (double)asm_out1 + (double)asm_out2;
    
    /* Return volatile sum to prevent dead code elimination */
    return final_result + (double)idx1 + (double)idx2 + scale;
}

int main(void)
{
    srand(time(NULL));
    
    /* Initialize volatile globals with random values */
    g_volatile_idx1 = rand() % 50;
    g_volatile_idx2 = rand() % 50;
    g_volatile_idx3 = rand() % 50;
    g_volatile_stride = 10 + rand() % 20;
    g_volatile_scale = 1.0 + (double)(rand() % 100) / 10.0;
    g_volatile_offset1 = rand() % 30;
    g_volatile_offset2 = rand() % 30;
    
    /* Call the function multiple times with different volatile values */
    double total = 0.0;
    
    for (int i = 0; i < 5; i++) {
        /* Vary parameters each iteration */
        total += trigger_reloads(
            g_volatile_idx1 + i,
            g_volatile_idx2 + i * 2,
            g_volatile_idx3 + i * 3,
            g_volatile_stride + i,
            g_volatile_scale + (double)i,
            g_volatile_offset1 + i * 4,
            g_volatile_offset2 + i * 5
        );
        
        /* Modify globals */
        g_volatile_idx1 = (g_volatile_idx1 + 7) % 50;
        g_volatile_idx2 = (g_volatile_idx2 + 11) % 50;
        g_volatile_offset1 = (g_volatile_offset1 + 3) % 30;
    }
    
    printf("Result: %f\n", total);
    
    return 0;
}
