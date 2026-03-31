/* reload_coverage.c - Complex program to trigger GCC reload pass edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization across this function */
__attribute__((noinline, noipa, optimize("no-gcse", "no-tree-pre")))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int idx4, volatile int stride1, volatile int stride2,
    volatile double scale1, volatile double scale2)
{
    /* High register pressure with mixed types */
    int local1[50];
    int local2[50];
    double dlocal1[30];
    double dlocal2[30];
    int *ptr1, *ptr2, **ptr_to_ptr;
    double *dptr1, *dptr2;
    int result = 0;
    volatile int vol_temp;
    
    /* Initialize arrays to create live ranges */
    for (int i = 0; i < 50; i++) {
        local1[i] = i * idx1;
        local2[i] = i * idx2;
    }
    for (int i = 0; i < 30; i++) {
        dlocal1[i] = i * scale1;
        dlocal2[i] = i * scale2;
    }
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS ===== */
    /* Complex addressing requiring temporary for address computation */
    ptr1 = &local1[idx1 * stride1 + idx2];
    ptr2 = &local2[idx3 * stride2 + idx4];
    
    /* Multi-level pointer arithmetic */
    ptr_to_ptr = &ptr1;
    
    /* ===== RELOAD_FOR_INPUT and RELOAD_OTHER ===== */
    /* Mixed integer/float operations with many live values */
    for (vol_temp = 0; vol_temp < 10; vol_temp++) {
        /* Complex array access with volatile indices */
        int temp1 = local1[vol_temp * stride1 + idx1];
        int temp2 = local2[vol_temp * stride2 + idx2];
        double dtemp1 = dlocal1[vol_temp];
        double dtemp2 = dlocal2[vol_temp];
        
        /* Force register pressure across classes */
        result += temp1 * temp2;
        result += (int)(dtemp1 * dtemp2);
        
        /* Pointer chain dereference - triggers address reloads */
        int chain_val = *(*(ptr_to_ptr) + vol_temp);
        result += chain_val;
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS ===== */
    /* Inline assembly with memory output */
    int output1, output2;
    __asm__ volatile(
        "movl %[input1], %[output1]\n\t"
        "movl %[input2], %[output2]"
        : [output1] "=r" (output1), [output2] "=r" (output2)
        : [input1] "m" (local1[idx1]), [input2] "m" (local2[idx2])
        : "memory"
    );
    result += output1 + output2;
    
    /* ===== RELOAD_FOR_OPERAND_ADDRESS ===== */
    /* Assembly with address operand */
    int addr_result;
    __asm__ volatile(
        "movl (%[addr]), %[result]"
        : [result] "=r" (addr_result)
        : [addr] "r" (&local1[idx3 * 2])
        : "memory"
    );
    result += addr_result;
    
    /* ===== RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Assembly that computes an output address */
    int *out_addr;
    __asm__ volatile(
        "leal (%[base], %[index], 4), %[out]"
        : [out] "=r" (out_addr)
        : [base] "r" (local1), [index] "r" (idx4)
        : "cc"
    );
    result += *out_addr;
    
    /* ===== RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Complex address as input to another address computation */
    int **complex_ptr = &ptr1;
    int complex_idx = idx1 + idx2;
    __asm__ volatile(
        "movl (%[ptr]), %%eax\n\t"
        "movl (%%eax, %[idx], 4), %%eax\n\t"
        "movl %%eax, %[res]"
        : [res] "=r" (vol_temp)
        : [ptr] "r" (complex_ptr), [idx] "r" (complex_idx)
        : "eax", "memory"
    );
    result += vol_temp;
    
    /* ===== RELOAD_FOR_OTHER_ADDRESS ===== */
    /* Mixed floating point and integer with address computations */
    dptr1 = &dlocal1[idx1];
    dptr2 = &dlocal2[idx2];
    
    for (int i = 0; i < 5; i++) {
        /* Complex addressing mode */
        double val1 = *(dptr1 + i * stride1);
        double val2 = *(dptr2 + i * stride2);
        
        /* Convert to int with truncation - forces register moves */
        int ival1 = (int)val1;
        int ival2 = (int)val2;
        
        /* More pointer arithmetic */
        int *int_ptr = &local1[ival1 & 0xF];
        result += *int_ptr + ival2;
    }
    
    /* ===== RELOAD_FOR_OPADDR_ADDR ===== */
    /* Nested addressing in assembly */
    int final_result;
    __asm__ volatile(
        "movl %[idx1], %%ecx\n\t"
        "movl %[idx2], %%edx\n\t"
        "leal (%[base], %%ecx, 4), %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        "movl (%%eax), %[result]"
        : [result] "=r" (final_result)
        : [base] "r" (local1), [idx1] "r" (idx3), [idx2] "r" (idx4)
        : "eax", "ecx", "edx", "memory"
    );
    
    result += final_result;
    
    /* Keep all values live until the end */
    __asm__ volatile("" : : "r" (local1), "r" (local2), 
                      "r" (dlocal1), "r" (dlocal2), "r" (ptr1), 
                      "r" (ptr2), "r" (ptr_to_ptr) : "memory");
    
    return result;
}

int main(void)
{
    srand(time(NULL));
    
    /* Volatile variables to prevent constant propagation */
    volatile int v1 = rand() % 20;
    volatile int v2 = rand() % 20;
    volatile int v3 = rand() % 20;
    volatile int v4 = rand() % 20;
    volatile int stride1 = 4 + (rand() % 3);
    volatile int stride2 = 3 + (rand() % 4);
    volatile double scale1 = 1.0 + (rand() % 100) / 100.0;
    volatile double scale2 = 1.0 + (rand() % 100) / 100.0;
    
    printf("Testing complex reload scenarios...\n");
    
    int result = trigger_reloads(v1, v2, v3, v4, stride1, stride2, scale1, scale2);
    
    printf("Result: %d\n", result);
    printf("Volatile values: %d %d %d %d\n", v1, v2, v3, v4);
    
    return 0;
}
