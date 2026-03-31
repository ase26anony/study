/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force function to not be inlined or optimized away */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int idx4, volatile int idx5, volatile int stride,
    volatile double scale, volatile long offset1, volatile long offset2
) {
    /* Create high register pressure with many live values */
    int a[100];      /* Integer array - pressure on integer registers */
    double b[50];    /* Double array - pressure on floating-point registers */
    int c[75];       /* More integer pressure */
    double d[40];    /* More floating-point pressure */
    
    /* Additional scalars to increase register pressure */
    int temp1, temp2, temp3, temp4, temp5;
    double ftemp1, ftemp2, ftemp3;
    long ltemp1, ltemp2;
    
    /* Pointer variables for complex addressing */
    int *ptr1, *ptr2, **ptr_to_ptr;
    double *dptr1, *dptr2;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) a[i] = i;
    for (int i = 0; i < 50; i++) b[i] = i * 1.5;
    for (int i = 0; i < 75; i++) c[i] = i * 2;
    for (int i = 0; i < 40; i++) d[i] = i * 0.75;
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 1: Complex multi-dimensional array access with volatile indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPUT */
    temp1 = a[idx1 * stride + idx2];
    temp2 = a[idx3 * stride + idx4];
    
    /* More complex: array of array access simulation */
    temp3 = *(a + idx1 * stride + idx2 + idx3);
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 2: Pointer chains for RELOAD_FOR_INPADDR_ADDRESS */
    ptr1 = &a[idx1];
    ptr2 = ptr1 + idx2;
    ptr_to_ptr = &ptr1;
    
    /* Complex address calculation requiring temporary register */
    temp4 = *(*(ptr_to_ptr) + idx3);
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 3: Mixed integer/floating-point operations */
    /* This pressures different register classes */
    ftemp1 = b[idx1] * scale;
    ftemp2 = b[idx2] + ftemp1;
    
    /* Convert float to int and back */
    temp5 = (int)ftemp2;
    ftemp3 = (double)temp5 * scale;
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 4: Inline assembly to force specific reload types */
    /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int asm_result1, asm_result2;
    long asm_addr;
    
    /* Assembly that takes memory address as input */
    __asm__ volatile(
        "mov %[addr], %[out1]\n\t"
        "add $1, %[out1]\n\t"
        : [out1] "=&r" (asm_result1)      /* Early clobber output */
        : [addr] "m" (a[idx1])            /* Memory input - forces address reload */
        : "cc"
    );
    
    /* More complex assembly with multiple constraints */
    __asm__ volatile(
        "lea (%[base], %[index], 4), %[out2]\n\t"
        "mov %[out2], %[addr_out]\n\t"
        : [out2] "=&r" (asm_result2),     /* Early clobber */
          [addr_out] "=m" (asm_addr)      /* Memory output */
        : [base] "r" (&a[0]),             /* Register input */
          [index] "r" (idx2)              /* Register input */
        : "cc"
    );
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 5: Output address reloads */
    /* Complex store address calculation */
    dptr1 = &d[idx1];
    dptr2 = dptr1 + idx2;
    
    /* Store with complex address - may trigger RELOAD_FOR_OUTPUT_ADDRESS */
    *dptr2 = ftemp3 * b[idx3];
    
    /* Another complex store */
    c[idx4 * 2 + idx5] = asm_result1 + asm_result2;
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 6: Multi-level pointer arithmetic with different types */
    char *char_ptr = (char *)a;
    int *int_ptr = (int *)(char_ptr + offset1);
    long *long_ptr = (long *)((char *)int_ptr + offset2);
    
    /* Access through type-punned pointers */
    ltemp1 = *long_ptr;
    temp1 = *int_ptr;
    
    /* More complex: pointer to pointer to pointer */
    int **ptr_arr[5];
    ptr_arr[0] = &ptr1;
    ptr_arr[1] = &ptr2;
    
    /* This should require address computation reloads */
    temp2 = **(ptr_arr[idx1 % 2]);
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 7: RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
    /* Complex expression mixing everything */
    int final_result = 
        a[idx1 * stride + idx2] +
        c[idx3 * 2 + idx4] +
        (int)(b[idx5] * scale) +
        temp1 + temp2 + temp3 + temp4 + temp5 +
        (int)ftemp1 + (int)ftemp2 + (int)ftemp3 +
        asm_result1 + asm_result2 +
        (int)ltemp1;
    
    /* Force all values to be live through the end */
    __asm__ volatile("" : : "r"(a[0]), "r"(b[0]), "r"(c[0]), "r"(d[0]),
                       "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4), "r"(temp5),
                       "r"(ftemp1), "r"(ftemp2), "r"(ftemp3),
                       "r"(ltemp1), "r"(ltemp2) : "memory");
    
    return final_result;
}

int main(void) {
    srand(time(NULL));
    
    /* Volatile variables to prevent constant propagation */
    volatile int idx1 = rand() % 50;
    volatile int idx2 = rand() % 50;
    volatile int idx3 = rand() % 50;
    volatile int idx4 = rand() % 50;
    volatile int idx5 = rand() % 50;
    volatile int stride = (rand() % 10) + 5;  /* Non-power-of-2 stride */
    volatile double scale = (rand() % 100) / 10.0 + 1.0;
    volatile long offset1 = rand() % 100;
    volatile long offset2 = rand() % 100;
    
    printf("Testing with parameters:\n");
    printf("  idx1=%d, idx2=%d, idx3=%d, idx4=%d, idx5=%d\n", 
           idx1, idx2, idx3, idx4, idx5);
    printf("  stride=%d, scale=%.2f, offset1=%ld, offset2=%ld\n",
           stride, scale, offset1, offset2);
    
    /* Call the function multiple times with different volatile values */
    int result1 = trigger_reloads(idx1, idx2, idx3, idx4, idx5, 
                                  stride, scale, offset1, offset2);
    
    /* Change volatile values */
    idx1 = rand() % 40;
    idx2 = rand() % 40;
    stride = (rand() % 8) + 8;
    
    int result2 = trigger_reloads(idx1, idx2, idx3, idx4, idx5,
                                  stride, scale * 2.0, offset1 + 10, offset2 + 20);
    
    printf("Results: %d, %d\n", result1, result2);
    printf("Sum: %d\n", result1 + result2);
    
    return 0;
}
