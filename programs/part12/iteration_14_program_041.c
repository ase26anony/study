/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization/inlining on the critical function */
__attribute__((noinline, noipa, optimize("no-gcse", "no-web", "no-tree-pre")))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2, volatile int scale,
    volatile long offset1, volatile long offset2, volatile char offset3)
{
    /* Create high register pressure with mixed types */
    int int_arr[100];
    double dbl_arr[50];
    long long_arr[75];
    char char_arr[200];
    
    /* Additional scalars to increase pressure */
    int temp1, temp2, temp3, temp4, temp5;
    double ftemp1, ftemp2, ftemp3;
    long ltemp1, ltemp2;
    
    /* Complex pointer chains */
    int *ptr1, *ptr2, **ptr_to_ptr;
    double *dptr1, *dptr2;
    char *cptr1, *cptr2;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) int_arr[i] = i * 3;
    for (int i = 0; i < 50; i++) dbl_arr[i] = i * 1.5;
    for (int i = 0; i < 75; i++) long_arr[i] = i * 7L;
    for (int i = 0; i < 200; i++) char_arr[i] = i % 128;
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* 1. RELOAD_FOR_INPUT_ADDRESS: Complex array addressing with volatile indices */
    /* Multi-dimensional access simulation: arr[row*stride + col] */
    temp1 = int_arr[idx1 * stride1 + idx2];
    temp2 = int_arr[idx2 * stride2 + idx3];
    
    /* 2. RELOAD_FOR_OUTPUT_ADDRESS: Store with complex address */
    int_arr[idx3 * stride1 + idx1] = temp1 + temp2;
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* 3. RELOAD_FOR_INPADDR_ADDRESS: Address of address computation */
    ptr1 = &int_arr[idx1 * stride1];
    ptr2 = &int_arr[idx2 * stride2];
    
    /* Pointer arithmetic creating complex address */
    int * volatile_ptr = ptr1 + idx3;
    temp3 = *volatile_ptr;
    
    /* 4. RELOAD_FOR_OUTADDR_ADDRESS: Output address reload */
    /* Inline assembly that takes address as input, produces output */
    int result1;
    __asm__ volatile(
        "movl (%1), %0\n\t"
        "addl $42, %0"
        : "=r" (result1)          /* Output in register */
        : "r" (&int_arr[idx1])    /* Input address in register */
        : "memory"
    );
    
    /* 5. RELOAD_FOR_OPERAND_ADDRESS: Mixed-type pointer chain */
    cptr1 = char_arr + offset1;
    cptr2 = cptr1 + offset2;
    
    /* Multi-level indirection */
    ptr_to_ptr = &ptr1;
    **ptr_to_ptr = idx1;
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* 6. RELOAD_FOR_OPADDR_ADDR: Complex address in inline asm */
    long addr_result;
    __asm__ volatile(
        "leaq (%1, %2, 4), %0\n\t"
        "addq %3, %0"
        : "=&r" (addr_result)     /* Early clobber output */
        : "r" (long_arr),         /* Base address */
          "r" ((long)idx1),       /* Index */
          "r" (offset1)           /* Displacement */
        : "cc"
    );
    
    /* 7. RELOAD_FOR_OTHER_ADDRESS: Floating point with address reloads */
    ftemp1 = dbl_arr[idx1];
    ftemp2 = dbl_arr[idx2];
    
    /* Mixed integer/float operations forcing moves between reg classes */
    temp4 = (int)(ftemp1 * scale);
    ftemp3 = ftemp2 * temp4;
    
    /* Store with complex address */
    dbl_arr[idx3] = ftemp3;
    
    /* 8. RELOAD_OTHER: Additional pressure with more variables */
    ltemp1 = long_arr[idx1];
    ltemp2 = long_arr[idx2];
    
    /* Complex expression with multiple uses */
    temp5 = (int)(ltemp1 * ltemp2 / (idx3 + 1));
    
    /* Another inline asm with memory constraint */
    int result2;
    __asm__ volatile(
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax"
        : "=a" (result2)
        : "a" (temp1), "b" (temp2), "c" (temp3)
        : "cc"
    );
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* Use all variables to ensure they're live */
    volatile int final_result = 
        temp1 + temp2 + temp3 + temp4 + temp5 + 
        result1 + result2 + 
        (int)ftemp1 + (int)ftemp2 + (int)ftemp3 +
        (int)ltemp1 + (int)ltemp2 +
        *cptr1 + *cptr2;
    
    return final_result;
}

int main(void)
{
    srand(time(NULL));
    
    /* Volatile variables to prevent constant propagation */
    volatile int idx1 = rand() % 50;
    volatile int idx2 = rand() % 50;
    volatile int idx3 = rand() % 50;
    volatile int stride1 = rand() % 10 + 5;
    volatile int stride2 = rand() % 10 + 5;
    volatile int scale = rand() % 100 + 1;
    volatile long offset1 = rand() % 100;
    volatile long offset2 = rand() % 100;
    volatile char offset3 = rand() % 50;
    
    printf("Testing with: idx1=%d, idx2=%d, idx3=%d\n", idx1, idx2, idx3);
    
    int result = trigger_reloads(idx1, idx2, idx3, stride1, stride2, scale,
                                offset1, offset2, offset3);
    
    printf("Result: %d\n", result);
    
    return 0;
}
