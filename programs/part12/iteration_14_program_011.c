/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline, noipa))
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Global volatile variables to prevent constant propagation */
volatile int g_volatile_idx1 = 0;
volatile int g_volatile_idx2 = 0;
volatile int g_volatile_idx3 = 0;
volatile int g_volatile_stride = 0;
volatile double g_volatile_scale = 0.0;
volatile long g_volatile_offset1 = 0;
volatile long g_volatile_offset2 = 0;

/* Complex addressing structure */
struct MultiLevelPtr {
    int ***ptr3;
    double **ptr2;
    char *ptr1;
};

NOINLINE static double trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride, volatile double scale,
    volatile long offset1, volatile long offset2)
{
    /* Create high register pressure with many live values */
    int a[100] = {0};
    double b[50] = {0.0};
    int c[75] = {0};
    double d[40] = {0.0};
    int e[60] = {0};
    
    /* Multi-dimensional array simulation with complex addressing */
    int *ptr_array[20];
    double *dbl_ptr_array[15];
    
    /* Complex pointer chains */
    int **ptr_to_ptr = NULL;
    double ***triple_ptr = NULL;
    
    /* Force register pressure across classes */
    int int_reg_pressure1, int_reg_pressure2, int_reg_pressure3;
    int int_reg_pressure4, int_reg_pressure5, int_reg_pressure6;
    double dbl_reg_pressure1, dbl_reg_pressure2, dbl_reg_pressure3;
    double dbl_reg_pressure4, dbl_reg_pressure5;
    
    /* Initialize arrays with volatile values */
    for (int i = 0; i < 50; i++) {
        b[i] = (double)(i * idx1) * scale;
        if (i < 40) d[i] = (double)(i + idx2) / scale;
    }
    
    COMPILER_BARRIER();
    
    /* Complex multi-level array access - triggers RELOAD_FOR_INPUT_ADDRESS */
    for (int i = 0; i < 20; i++) {
        /* Multi-dimensional access pattern */
        int base_idx = i * stride + idx1;
        int offset_idx = base_idx + idx2 * idx3;
        
        /* Complex addressing with multiple volatile components */
        a[base_idx % 100] = c[offset_idx % 75] + e[(i + idx3) % 60];
        
        /* Mixed floating-point operation */
        d[i % 40] = b[base_idx % 50] * scale + d[offset_idx % 40];
    }
    
    COMPILER_BARRIER();
    
    /* Pointer arithmetic with different types - triggers various address reloads */
    char *char_ptr = (char *)a;
    int *int_ptr = (int *)b;  /* Type punning for complexity */
    double *dbl_ptr = (double *)c;
    
    /* Complex address calculation with volatile offsets */
    char_ptr = char_ptr + offset1 * sizeof(int) + offset2;
    int_ptr = (int *)((char *)int_ptr + idx1 * sizeof(double) + idx2 * sizeof(int));
    dbl_ptr = dbl_ptr + (offset1 % 10) * (offset2 % 5);
    
    COMPILER_BARRIER();
    
    /* Inline assembly to force specific reload types */
    
    /* RELOAD_FOR_OPERAND_ADDRESS - address as input to assembly */
    int asm_result1, asm_result2;
    double asm_dbl_result;
    
    /* Assembly with memory input and register output */
    __asm__ volatile (
        "movl (%1), %0\n\t"
        : "=r" (asm_result1)
        : "r" (&a[idx1 * 2 + idx2])
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_OUTPUT_ADDRESS - address as output from assembly */
    int *asm_addr_output;
    __asm__ volatile (
        "leaq (%1, %2, 4), %0\n\t"
        : "=&r" (asm_addr_output)
        : "r" (a), "r" (idx3)
        : "cc"
    );
    
    COMPILER_BARRIER();
    
    /* Mixed register class pressure with inline assembly */
    __asm__ volatile (
        "cvtsi2sd %1, %%xmm0\n\t"
        "addsd %%xmm0, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m" (asm_dbl_result)
        : "r" (idx1 * idx2)
        : "xmm0", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* More complex addressing with pointer chains */
    ptr_to_ptr = &int_ptr;
    for (int i = 0; i < 10; i++) {
        /* Chain dereference with volatile indices */
        int temp = **(ptr_to_ptr + (i * idx1) % 10);
        a[(i + idx2) % 100] = temp * idx3;
    }
    
    COMPILER_BARRIER();
    
    /* Floating-point intensive section for FP register pressure */
    double fp_sum = 0.0;
    for (int i = 0; i < 40; i++) {
        /* Complex floating-point expression */
        d[i] = (b[i % 50] * scale) + (d[(i + idx1) % 40] / scale);
        fp_sum += d[i];
        
        /* Integer-to-float conversion adds pressure */
        a[i % 100] = (int)(d[i] * 100.0) + idx2;
    }
    
    COMPILER_BARRIER();
    
    /* Final complex address calculation */
    /* RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    struct MultiLevelPtr mlp;
    mlp.ptr1 = (char *)&a[0];
    mlp.ptr2 = &dbl_ptr;
    mlp.ptr3 = &ptr_to_ptr;
    
    /* Multi-level indirection with volatile offsets */
    char final_char = *(mlp.ptr1 + offset1 + idx1 * offset2);
    double final_double = **(mlp.ptr2 + (idx2 % 5));
    
    COMPILER_BARRIER();
    
    /* Use inline assembly with multiple constraints */
    int final_result;
    __asm__ volatile (
        "imull %%ecx, %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        : "=&a" (final_result)
        : "a" (idx1), "c" (idx2), "d" (asm_result1)
        : "cc"
    );
    
    /* Return volatile sum to prevent elimination */
    return fp_sum + final_double + final_char + final_result + asm_dbl_result;
}

/* Secondary function to create more reload scenarios */
NOINLINE static int trigger_more_reloads(volatile int *arr, volatile double *darr, 
                                        volatile int idx1, volatile int idx2)
{
    int local_arr[80];
    double local_darr[45];
    
    /* Complex address reload scenarios */
    for (int i = 0; i < 40; i++) {
        /* RELOAD_FOR_OTHER_ADDRESS */
        local_arr[i] = arr[idx1 * i + idx2] + arr[idx2 * i + idx1];
        local_darr[i % 45] = darr[(idx1 + i) % 30] * 2.0;
    }
    
    COMPILER_BARRIER();
    
    /* Pointer-to-pointer with inline assembly */
    int **pptr = (int **)&arr;
    int result;
    
    __asm__ volatile (
        "movq (%1), %%rax\n\t"
        "movl (%2), %%ebx\n\t"
        "movl (%%rax, %%rbx, 4), %0\n\t"
        : "=r" (result)
        : "r" (pptr), "r" (&idx1)
        : "rax", "rbx", "memory"
    );
    
    return result + local_arr[0] + (int)local_darr[0];
}

int main(void)
{
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    g_volatile_idx1 = rand() % 50;
    g_volatile_idx2 = rand() % 50;
    g_volatile_idx3 = rand() % 50;
    g_volatile_stride = (rand() % 10) + 1;
    g_volatile_scale = (double)(rand() % 100) / 10.0 + 0.1;
    g_volatile_offset1 = rand() % 100;
    g_volatile_offset2 = rand() % 100;
    
    /* Create volatile arrays */
    volatile int volatile_arr[100];
    volatile double volatile_darr[50];
    
    for (int i = 0; i < 100; i++) {
        volatile_arr[i] = rand() % 1000;
        if (i < 50) volatile_darr[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Call the reload-triggering functions */
    double result1 = trigger_reloads(
        g_volatile_idx1, g_volatile_idx2, g_volatile_idx3,
        g_volatile_stride, g_volatile_scale,
        g_volatile_offset1, g_volatile_offset2
    );
    
    int result2 = trigger_more_reloads(
        volatile_arr, volatile_darr,
        g_volatile_idx1, g_volatile_idx2
    );
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %f\n", result1);
    printf("Result2: %d\n", result2);
    printf("Combined: %f\n", result1 + result2);
    
    return 0;
}
