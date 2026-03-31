/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization/inlining on the target function */
#define NO_OPT __attribute__((noinline, noipa, optimize("O0")))

/* Compiler barrier to prevent reordering */
#define BARRIER() __asm__ volatile("" : : : "memory")

/* Complex inline assembly to force specific reloads */
#define FORCE_ADDRESS_RELOAD(addr, out) \
    __asm__ volatile("movq %1, %0\n\t" \
                     "addq $1, %0" \
                     : "=&r" (out) \
                     : "m" (*(volatile long*)(addr)) \
                     : "cc")

#define FORCE_OUTPUT_ADDRESS_RELOAD(val, addr) \
    __asm__ volatile("movq %1, %%rax\n\t" \
                     "movq %%rax, %0" \
                     : "=m" (*(volatile long*)(addr)) \
                     : "r" (val) \
                     : "rax", "cc")

/* Main function that triggers multiple reload types */
NO_OPT static long trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int idx4, volatile int idx5, volatile int idx6,
    volatile long offset1, volatile long offset2, volatile long offset3,
    volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with many live values */
    int local_arr1[128];      /* Integer array - pressure on integer regs */
    double local_arr2[64];    /* Double array - pressure on FP regs */
    long local_arr3[32];      /* Long array - mixed pressure */
    
    /* Multiple pointer variables for complex addressing */
    int *ptr1, *ptr2, *ptr3;
    double *dptr1, *dptr2;
    long *lptr1, **lptr2;
    
    /* Many scalar variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Initialize arrays with some values */
    for (int i = 0; i < 128; i++) local_arr1[i] = i * 3;
    for (int i = 0; i < 64; i++) local_arr2[i] = i * 1.5;
    for (int i = 0; i < 32; i++) local_arr3[i] = i * 7L;
    
    BARRIER();
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Complex multi-level addressing with volatile indices */
    ptr1 = &local_arr1[idx1 + idx2];  /* Base pointer calculation */
    
    /* Multi-dimensional access simulation with pointer chains */
    lptr1 = &local_arr3[idx3];
    lptr2 = &lptr1;  /* Pointer to pointer */
    
    /* Complex address calculation requiring temporary register */
    ptr2 = &local_arr1[(idx1 * idx2) + (idx3 << 2) + idx4];
    
    /* Mixed type pointer arithmetic */
    dptr1 = &local_arr2[idx5] + idx6;
    
    BARRIER();
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Store through complex computed addresses */
    *(ptr1 + idx3) = idx1 * idx2;  /* Output address reload */
    
    /* Double indirection with output */
    **lptr2 = offset1 + offset2;
    
    /* Complex output address with scaling */
    local_arr1[(idx4 * 3) + idx5] = idx6 * 7;
    
    BARRIER();
    
    /* ===== RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR ===== */
    /* Inline assembly that forces operand address reloads */
    long asm_out1, asm_out2;
    FORCE_ADDRESS_RELOAD(&local_arr1[idx1 + idx2], asm_out1);
    FORCE_ADDRESS_RELOAD(&local_arr3[idx3 * 2], asm_out2);
    
    /* Another assembly forcing output address reload */
    long output_val = asm_out1 + asm_out2;
    FORCE_OUTPUT_ADDRESS_RELOAD(output_val, &local_arr3[idx4]);
    
    BARRIER();
    
    /* ===== RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER ===== */
    /* Mixed integer/floating point computations to pressure all reg classes */
    d1 = local_arr2[idx1] * scale1;
    d2 = local_arr2[idx2] * scale2;
    d3 = d1 + d2;
    
    /* Integer computations interleaved with FP */
    v1 = local_arr1[idx3];
    v2 = local_arr1[idx4];
    v3 = v1 * v2;
    
    /* Convert between types to force register moves */
    d4 = (double)v3 + d3;
    v4 = (int)d4;
    
    /* More complex addressing with mixed types */
    dptr2 = (double*)((char*)local_arr2 + (idx5 * sizeof(double)) + idx6);
    d5 = *dptr2 * 2.0;
    
    /* Pointer chasing with different types */
    char* cptr = (char*)local_arr1;
    cptr += idx1 * sizeof(int) + idx2;
    ptr3 = (int*)cptr;
    v5 = *ptr3;
    
    BARRIER();
    
    /* ===== Create overlapping live ranges ===== */
    /* Use all variables in a complex expression */
    l1 = (long)v1 + (long)v2 + (long)v3 + (long)v4 + (long)v5;
    l2 = (long)(d1 * 100.0) + (long)(d2 * 200.0) + (long)(d3 * 300.0);
    l3 = asm_out1 * asm_out2;
    l4 = offset1 * offset2 * offset3;
    
    /* Complex final computation using all values */
    long result = l1 + l2 + l3 + l4 + 
                  (long)d4 + (long)d5 + 
                  local_arr3[idx6] + 
                  (long)(*dptr1) + 
                  (long)output_val;
    
    BARRIER();
    
    /* Force all variables to appear live */
    volatile long sink = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                         (long)d1 + (long)d2 + (long)d3 + (long)d4 + (long)d5 + (long)d6 +
                         l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
    (void)sink;  /* Prevent unused variable warning */
    
    return result;
}

int main(void)
{
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    volatile int idx1 = rand() % 50;
    volatile int idx2 = rand() % 50;
    volatile int idx3 = rand() % 50;
    volatile int idx4 = rand() % 50;
    volatile int idx5 = rand() % 50;
    volatile int idx6 = rand() % 50;
    
    volatile long offset1 = rand() % 100;
    volatile long offset2 = rand() % 100;
    volatile long offset3 = rand() % 100;
    
    volatile double scale1 = (rand() % 100) / 10.0;
    volatile double scale2 = (rand() % 100) / 10.0;
    
    /* Call the function multiple times to ensure compilation */
    long total = 0;
    for (int i = 0; i < 3; i++) {
        total += trigger_reloads(idx1 + i, idx2, idx3, idx4, idx5, idx6,
                                offset1, offset2, offset3,
                                scale1, scale2);
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
