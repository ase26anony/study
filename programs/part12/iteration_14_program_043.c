/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization/inlining on the target function */
#define NO_OPT __attribute__((noinline, noipa, optimize(0)))
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Global volatile variables to prevent constant propagation */
volatile int g_volatile_idx1 = 0;
volatile int g_volatile_idx2 = 0;
volatile int g_volatile_idx3 = 0;
volatile int g_volatile_stride = 0;
volatile double g_volatile_scale = 0.0;
volatile long g_volatile_offset1 = 0;
volatile long g_volatile_offset2 = 0;

/* Function to trigger multiple reload types */
NO_OPT static double trigger_reloads(
    volatile int idx1, 
    volatile int idx2, 
    volatile int idx3,
    volatile int stride,
    volatile double scale,
    volatile long off1,
    volatile long off2)
{
    /* Create high register pressure with many live values */
    int local_arr1[100];      /* Integer array - pressure on integer regs */
    double local_arr2[50];    /* FP array - pressure on FP regs */
    int local_arr3[75];       /* More integer pressure */
    double local_arr4[40];    /* More FP pressure */
    
    /* Various pointer types for complex addressing */
    char *char_ptr;
    int *int_ptr;
    long *long_ptr;
    double *double_ptr;
    
    /* Temporary variables with overlapping live ranges */
    int temp1, temp2, temp3, temp4, temp5;
    double ftemp1, ftemp2, ftemp3, ftemp4;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) local_arr1[i] = i;
    for (int i = 0; i < 50; i++) local_arr2[i] = i * 0.5;
    for (int i = 0; i < 75; i++) local_arr3[i] = i * 2;
    for (int i = 0; i < 40; i++) local_arr4[i] = i * 1.5;
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Complex multi-level addressing that requires address reloads */
    
    /* 1. Multi-dimensional array access with volatile indices */
    /* This creates: base + idx1*stride + idx2 addressing */
    temp1 = local_arr1[idx1 * stride + idx2];
    COMPILER_BARRIER();
    
    /* 2. Pointer chain with different types */
    char_ptr = (char *)local_arr1;
    int_ptr = (int *)(char_ptr + off1);  /* char* + offset needs scaling */
    long_ptr = (long *)((char *)int_ptr + off2); /* Another offset */
    
    /* 3. Complex address computation requiring temporary register */
    temp2 = *((int *)((char *)long_ptr + idx3 * sizeof(int)));
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Store operations with complex addressing */
    
    /* 4. Store with complex address calculation */
    double_ptr = &local_arr2[idx1] + idx2;  /* Base + index calculation */
    *double_ptr = scale * local_arr4[idx3];
    COMPILER_BARRIER();
    
    /* 5. Even more complex store address */
    local_arr3[((idx1 * 3) + (idx2 * 7)) % 75] = 
        local_arr1[((idx2 * 5) + (idx3 * 11)) % 100];
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR ===== */
    /* Use inline assembly to force specific reload types */
    
    /* 6. Inline asm with memory address input */
    int asm_input = 42;
    int asm_output;
    __asm__ volatile(
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (asm_output)      /* Output in register */
        : "m" (asm_input)        /* Memory input - forces address reload */
        : "%eax"
    );
    temp3 = asm_output;
    COMPILER_BARRIER();
    
    /* 7. Another asm with complex address constraint */
    int *addr_ptr = &local_arr1[idx1];
    int asm_output2;
    __asm__ volatile(
        "movl (%1), %%ebx\n\t"
        "imull $2, %%ebx\n\t"
        "movl %%ebx, %0"
        : "=&r" (asm_output2)    /* Early clobber output */
        : "r" (addr_ptr)         /* Register containing address */
        : "%ebx"
    );
    temp4 = asm_output2;
    COMPILER_BARRIER();
    
    /* ===== RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS ===== */
    /* Mixed register class operations to force various reloads */
    
    /* 8. Integer to float conversion and back */
    ftemp1 = (double)local_arr1[idx2];
    ftemp2 = ftemp1 * scale;
    temp5 = (int)ftemp2;
    COMPILER_BARRIER();
    
    /* 9. Complex floating point with integer addressing */
    for (int i = 0; i < 10; i++) {
        /* Mixed operations in loop - creates overlapping live ranges */
        ftemp3 = local_arr2[i] * scale;
        ftemp4 = ftemp3 + (double)local_arr1[i * stride];
        local_arr4[i] = ftemp4;
        
        /* Integer computation using same indices */
        local_arr3[i] = local_arr1[idx1 + i] + local_arr1[idx2 + i];
    }
    COMPILER_BARRIER();
    
    /* 10. Pointer arithmetic with multiple indirections */
    int **ptr_to_ptr = (int **)malloc(20 * sizeof(int *));
    if (ptr_to_ptr) {
        for (int i = 0; i < 20; i++) {
            ptr_to_ptr[i] = &local_arr1[i * 5];
        }
        
        /* Complex indirection: *(*(base + idx1) + idx2) */
        int complex_result = *(ptr_to_ptr[idx1] + idx2);
        temp5 += complex_result;
        
        free(ptr_to_ptr);
    }
    
    /* ===== Create artificial register pressure ===== */
    /* Use all local variables in final computation to extend live ranges */
    double final_result = 0.0;
    
    /* Integer computations */
    final_result += (double)(temp1 + temp2 + temp3 + temp4 + temp5);
    
    /* Floating point computations */
    final_result += ftemp1 + ftemp2 + ftemp3 + ftemp4;
    
    /* Array accesses with complex addressing */
    for (int i = 0; i < 5; i++) {
        /* Multi-dimensional addressing pattern */
        int complex_idx = (idx1 * i + idx2 * (i + 1) + idx3 * (i + 2)) % 100;
        final_result += (double)local_arr1[complex_idx];
        final_result += local_arr2[i * stride % 50];
        final_result += (double)local_arr3[(i * idx1) % 75];
        final_result += local_arr4[(i * idx2) % 40];
    }
    
    /* One more inline asm with memory operand */
    double asm_final;
    __asm__ volatile(
        "fldl %1\n\t"
        "fadd %%st(0), %%st(0)\n\t"
        "fstpl %0"
        : "=m" (asm_final)
        : "m" (final_result)
        : "memory"
    );
    
    return asm_final;
}

int main(void)
{
    srand(time(NULL));
    
    /* Initialize volatile globals with random values */
    g_volatile_idx1 = rand() % 50;
    g_volatile_idx2 = rand() % 50;
    g_volatile_idx3 = rand() % 50;
    g_volatile_stride = (rand() % 10) + 1;
    g_volatile_scale = (rand() % 100) / 10.0 + 0.1;
    g_volatile_offset1 = rand() % 100;
    g_volatile_offset2 = rand() % 100;
    
    /* Call the function with volatile arguments */
    double result = trigger_reloads(
        g_volatile_idx1,
        g_volatile_idx2,
        g_volatile_idx3,
        g_volatile_stride,
        g_volatile_scale,
        g_volatile_offset1,
        g_volatile_offset2
    );
    
    printf("Result: %f\n", result);
    printf("Indices: %d, %d, %d\n", 
           g_volatile_idx1, g_volatile_idx2, g_volatile_idx3);
    
    return 0;
}
