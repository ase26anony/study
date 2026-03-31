/* reload_coverage.c - Program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would simplify addressing */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Global volatile variables to prevent constant propagation */
volatile int g_idx1 = 7;
volatile int g_idx2 = 13;
volatile int g_idx3 = 19;
volatile long g_offset1 = 23;
volatile long g_offset2 = 29;
volatile double g_scale = 2.5;

/* Complex multi-dimensional array access with volatile indices */
NOOPT double trigger_reloads(volatile int idx1, volatile int idx2, 
                            volatile int idx3, volatile long off1,
                            volatile long off2, volatile double scale) {
    /* Create high register pressure with many live values */
    int int_arr[128];          /* Integer array - pressure on integer regs */
    double dbl_arr[64];        /* Double array - pressure on FP regs */
    long long_arr[32];         /* Long array - mixed pressure */
    char *ptr_chain[16];       /* Pointer chain for complex addressing */
    double result = 0.0;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 128; i++) int_arr[i] = rand() % 100;
    for (int i = 0; i < 64; i++) dbl_arr[i] = (rand() % 100) * 0.01;
    for (int i = 0; i < 32; i++) long_arr[i] = rand() % 1000;
    
    /* Compiler barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* ====== RELOAD_FOR_INPUT_ADDRESS ====== */
    /* Complex addressing: base + index*scale + displacement with volatile */
    int_arr[idx1 * 4 + idx2 + 3] = int_arr[idx3 * 2 - idx1] * scale;
    
    /* Pointer chain dereferencing - forces address computation reloads */
    char *base_ptr = (char *)int_arr;
    char *mid_ptr = base_ptr + off1 * sizeof(int);
    char *final_ptr = mid_ptr + off2 * sizeof(int);
    int_arr[0] = *(int *)final_ptr;
    
    /* ====== RELOAD_FOR_OUTPUT_ADDRESS ====== */
    /* Store with complex address calculation */
    long_arr[(idx1 + idx2) * 3 % 32] = (long)(dbl_arr[idx3 % 64] * 100.0);
    
    /* ====== Mixed register class pressure ====== */
    /* Integer and floating-point computations interleaved */
    for (int i = 0; i < 8; i++) {
        /* Convert int to double (requires moving between register classes) */
        double temp_dbl = (double)int_arr[i * idx1 % 128];
        dbl_arr[i * 7 % 64] = temp_dbl * scale;
        
        /* Convert double to int (another class crossing) */
        int temp_int = (int)(dbl_arr[i * 3 % 64] * 10.0);
        int_arr[i * 5 % 128] = temp_int + idx2;
    }
    
    /* ====== Inline Assembly with constraints ====== */
    /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int asm_input, asm_output;
    
    /* Assembly taking memory address as input */
    __asm__ volatile (
        "movl (%1), %0\n\t"           /* Load from memory address */
        "addl $42, %0\n\t"            /* Modify value */
        : "=r" (asm_output)           /* Output in register */
        : "r" (&int_arr[idx1 % 128])  /* Input: address in register */
        : "memory"
    );
    
    /* Assembly with earlyclobber and memory output */
    int asm_temp;
    __asm__ volatile (
        "movl %%eax, %0\n\t"          /* Save eax */
        "movl $100, %%eax\n\t"        /* Use eax as temp */
        "addl %%eax, %1\n\t"          /* Add to memory */
        "movl %0, %%eax"              /* Restore eax */
        : "=&r" (asm_temp), "+m" (int_arr[10])  /* Earlyclobber + memory */
        : 
        : "eax", "memory"
    );
    
    /* ====== Multi-level indirection ====== */
    /* Create pointer-to-pointer chains */
    int **pptr1 = (int **)malloc(sizeof(int *) * 8);
    int **pptr2 = (int **)malloc(sizeof(int *) * 8);
    
    for (int i = 0; i < 8; i++) {
        pptr1[i] = &int_arr[i * 16];
        pptr2[i] = &int_arr[i * 16 + 8];
    }
    
    /* Complex addressing through multiple pointer levels */
    int val1 = *(pptr1[idx1 % 8] + idx2 % 8);
    int val2 = *(pptr2[idx3 % 8] + idx1 % 8);
    int_arr[20] = val1 + val2;
    
    /* ====== RELOAD_FOR_INPADDR_ADDRESS ====== */
    /* Address of address computation */
    int *addr_ptr = &int_arr[idx1 * idx2 % 128];
    __asm__ volatile (
        "movl (%1), %0\n\t"
        : "=r" (asm_output)
        : "r" (addr_ptr)      /* Address already computed */
        : "memory"
    );
    
    /* ====== RELOAD_FOR_OTHER_ADDRESS ====== */
    /* Complex address in non-standard context */
    {
        volatile int *volatile_ptr = (volatile int *)int_arr;
        volatile_ptr[off1 % 64 + off2 % 32] = 
            volatile_ptr[idx1 % 32] + volatile_ptr[idx2 % 32];
    }
    
    /* Compiler barrier between computations */
    __asm__ volatile("" : : : "memory");
    
    /* Final computation using all values */
    for (int i = 0; i < 16; i++) {
        /* Multi-dimensional access pattern */
        int index = (i * idx1 + idx2 * 3 - idx3) % 128;
        if (index < 0) index = -index;
        
        result += dbl_arr[i % 64] * int_arr[index];
        result -= long_arr[i % 32] * 0.01;
    }
    
    free(pptr1);
    free(pptr2);
    
    return result + asm_output + asm_temp;
}

int main(void) {
    /* Initialize with random values */
    srand(42);
    
    /* Update global volatiles */
    g_idx1 = rand() % 100;
    g_idx2 = rand() % 100;
    g_idx3 = rand() % 100;
    g_offset1 = rand() % 50;
    g_offset2 = rand() % 50;
    g_scale = 1.0 + (rand() % 100) * 0.01;
    
    /* Call function with volatile arguments */
    double result = trigger_reloads(g_idx1, g_idx2, g_idx3, 
                                   g_offset1, g_offset2, g_scale);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", result);
    
    /* Additional calls with different patterns */
    for (int i = 0; i < 3; i++) {
        g_idx1 = (g_idx1 * 13 + 7) % 100;
        result += trigger_reloads(g_idx1, g_idx2 + i, g_idx3 - i,
                                 g_offset1 + i, g_offset2 - i, g_scale * (i+1));
    }
    
    printf("Final result: %f\n", result);
    return 0;
}
