/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization or inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Global volatile variables to prevent optimization */
volatile int volatile_idx1, volatile_idx2, volatile_idx3, volatile_idx4;
volatile int volatile_stride, volatile_offset, volatile_scale;
volatile double volatile_dscale;

/* Complex addressing structure */
struct MultiLevel {
    int *level1;
    int **level2;
    int ***level3;
};

NOINLINE static long trigger_reloads(
    int v1, int v2, int v3, int v4,
    int stride, int offset, int scale,
    double dscale)
{
    /* High register pressure with mixed types */
    int a[100];          /* Integer array - pressure on integer regs */
    double b[50];        /* Double array - pressure on FP regs */
    int c[75];           /* More integer pressure */
    double d[25];        /* More FP pressure */
    
    /* Pointer variables for complex addressing */
    int *ptr1, *ptr2, **ptr3, ***ptr4;
    double *dptr1, *dptr2;
    
    /* Mixed scalar variables to increase live ranges */
    int scalar1, scalar2, scalar3, scalar4, scalar5;
    double dscalar1, dscalar2, dscalar3;
    long result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) a[i] = i * scale;
    for (int i = 0; i < 50; i++) b[i] = i * dscale;
    for (int i = 0; i < 75; i++) c[i] = i + offset;
    for (int i = 0; i < 25; i++) d[i] = i / dscale;
    
    COMPILER_BARRIER();
    
    /* 1. RELOAD_FOR_INPUT_ADDRESS - Complex array addressing */
    /* Multi-dimensional access simulation with volatile indices */
    scalar1 = a[v1 * stride + v2];          /* Base + index * scale */
    scalar2 = a[v3 * 4 + v4];               /* Different scale factor */
    
    /* Chain of pointer arithmetic */
    ptr1 = &a[0];
    ptr2 = ptr1 + v1 * stride + v2;         /* Complex pointer offset */
    scalar3 = *ptr2;
    
    COMPILER_BARRIER();
    
    /* 2. RELOAD_FOR_OUTPUT_ADDRESS - Store with complex addressing */
    /* Store to array with complex index calculation */
    a[(v1 + v2) * stride - v3] = scalar1 + scalar2;
    c[v1 * 3 + v2 * 2 + v3] = scalar3 * scale;
    
    /* Pointer-based store with offset */
    ptr1 = &c[25];
    *(ptr1 + v1 - v2) = v3 * v4;
    
    COMPILER_BARRIER();
    
    /* 3. Mixed integer/float operations - pressure different register classes */
    for (int i = 0; i < 10; i++) {
        /* Integer to float conversion */
        dscalar1 = (double)a[i * stride + v1];
        dscalar2 = b[i + v2];
        dscalar3 = dscalar1 * dscalar2 * dscale;
        
        /* Float to integer conversion */
        scalar4 = (int)(dscalar3 / dscale);
        a[i] = scalar4 + c[i + v3];
        
        /* Keep values live */
        result += scalar4;
    }
    
    COMPILER_BARRIER();
    
    /* 4. Inline assembly to force specific reload types */
    
    /* RELOAD_FOR_OPERAND_ADDRESS - Address as input to asm */
    int asm_input, asm_output;
    asm_input = v1 * v2 + v3;
    
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_output)      /* Output in register */
        : "m" (asm_input)        /* Input from memory - forces address reload */
        : "%eax", "memory"
    );
    result += asm_output;
    
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_OUTADDR_ADDRESS - Address as output from asm */
    int *addr_out;
    __asm__ volatile (
        "leaq %1, %%rax\n\t"
        "addq $16, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (addr_out)        /* Output address in register */
        : "m" (a[10])            /* Input memory operand */
        : "%rax", "memory"
    );
    scalar5 = *addr_out;
    result += scalar5;
    
    COMPILER_BARRIER();
    
    /* 5. Multi-level indirection - triggers various address reloads */
    int **level2_ptr = (int **)&a[50];
    int ***level3_ptr = (int ***)&level2_ptr;
    
    /* Complex address calculation with multiple levels */
    scalar1 = *(*(level2_ptr + v1) + v2);
    scalar2 = ***(level3_ptr + v1);
    
    /* Pointer chain dereference */
    ptr3 = &ptr1;
    *ptr3 = &a[v1 * stride + v2];
    scalar3 = **ptr3;
    
    COMPILER_BARRIER();
    
    /* 6. RELOAD_FOR_INPADDR_ADDRESS - Address of address */
    int *addr_array[10];
    for (int i = 0; i < 10; i++) {
        addr_array[i] = &a[i * stride + v1];
    }
    
    /* Access through address array with volatile index */
    scalar4 = *addr_array[v2];
    scalar5 = *addr_array[v3];
    
    /* Compute with address values */
    long addr_diff = (long)addr_array[v4] - (long)addr_array[v1];
    result += addr_diff / sizeof(int);
    
    COMPILER_BARRIER();
    
    /* 7. RELOAD_OTHER - Mixed operations that don't fit other categories */
    /* Complex expression with multiple live values */
    result += ((long)a[v1] << 3) | (c[v2] & 0xFF);
    result += (long)(b[v3] * dscale) * scalar1;
    result += (long)scalar2 * scalar3 * scalar4 * scalar5;
    
    /* Floating point array with complex addressing */
    for (int i = 0; i < 5; i++) {
        dscalar1 = b[i * 2 + v1] * d[v2 + i];
        dscalar2 = b[i * 3 + v3] / dscale;
        result += (long)(dscalar1 + dscalar2);
    }
    
    COMPILER_BARRIER();
    
    /* 8. RELOAD_FOR_OPADDR_ADDR - Operand address of address */
    struct MultiLevel ml;
    ml.level1 = &a[0];
    ml.level2 = &ml.level1;
    ml.level3 = &ml.level2;
    
    /* Chain of address taking */
    scalar1 = ***ml.level3;
    scalar2 = **(ml.level2 + v1);
    
    /* Address computation involving structure fields */
    long addr_sum = (long)ml.level1 + (long)*ml.level2 + (long)**ml.level3;
    result += addr_sum;
    
    COMPILER_BARRIER();
    
    /* 9. RELOAD_FOR_OTHER_ADDRESS - Other address computations */
    /* Complex address expression with multiple operations */
    int *complex_addr = &a[0] + (v1 * v2) / scale + (v3 << 2) - v4;
    scalar1 = *complex_addr;
    
    /* Address computation with floating point conversion */
    long addr_as_long = (long)(&b[0]) + (long)(dscale * 100.0);
    dptr1 = (double *)addr_as_long;
    if (dptr1 >= &b[0] && dptr1 < &b[50]) {
        dscalar1 = *dptr1;
        result += (long)dscalar1;
    }
    
    COMPILER_BARRIER();
    
    /* Final mixed computation to keep all values live */
    result += scalar1 + scalar2 + scalar3 + scalar4 + scalar5;
    result += (long)(dscalar1 + dscalar2 + dscalar3);
    
    return result;
}

int main(void)
{
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    volatile_idx1 = rand() % 50;
    volatile_idx2 = rand() % 50;
    volatile_idx3 = rand() % 50;
    volatile_idx4 = rand() % 50;
    volatile_stride = 10 + rand() % 20;
    volatile_offset = rand() % 100;
    volatile_scale = 1 + rand() % 5;
    volatile_dscale = 1.0 + (rand() % 100) / 10.0;
    
    printf("Testing complex reload scenarios...\n");
    printf("Indices: %d, %d, %d, %d\n", 
           volatile_idx1, volatile_idx2, volatile_idx3, volatile_idx4);
    printf("Stride: %d, Offset: %d, Scale: %d, DScale: %.2f\n",
           volatile_stride, volatile_offset, volatile_scale, volatile_dscale);
    
    long result = trigger_reloads(
        volatile_idx1, volatile_idx2, volatile_idx3, volatile_idx4,
        volatile_stride, volatile_offset, volatile_scale, volatile_dscale);
    
    printf("Result: %ld\n", result);
    
    /* Verify with simple calculation */
    long check = volatile_idx1 + volatile_idx2 + volatile_idx3 + volatile_idx4;
    check *= volatile_scale;
    printf("Simple check: %ld\n", check);
    
    return 0;
}
