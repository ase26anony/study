/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization/inlining on the critical function */
#define NO_OPT __attribute__((noinline, noipa, optimize("O0")))

/* Global volatile variables to prevent constant propagation */
volatile int v1, v2, v3, v4, v5, v6, v7, v8;
volatile long vl1, vl2, vl3;
volatile double vd1, vd2;

/* Function to trigger complex reload scenarios */
NO_OPT static double trigger_reloads(
    volatile int idx1, volatile int idx2, 
    volatile int idx3, volatile int idx4,
    volatile long l_offset1, volatile long l_offset2,
    volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with many live values */
    int local_arr1[100];
    int local_arr2[50];
    double dbl_arr1[75];
    double dbl_arr2[40];
    char char_arr[200];
    
    /* Various pointer types for different addressing modes */
    int *ptr1, *ptr2, **ptr_to_ptr;
    double *dptr1, *dptr2;
    char *cptr;
    long *lptr;
    
    /* Intermediate values that must stay alive */
    int temp1, temp2, temp3, temp4, temp5;
    double dtemp1, dtemp2, dtemp3;
    long ltemp1, ltemp2;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 100; i++) local_arr1[i] = rand() % 1000;
    for (int i = 0; i < 50; i++) local_arr2[i] = rand() % 500;
    for (int i = 0; i < 75; i++) dbl_arr1[i] = (rand() % 1000) * 0.001;
    for (int i = 0; i < 40; i++) dbl_arr2[i] = (rand() % 1000) * 0.001;
    for (int i = 0; i < 200; i++) char_arr[i] = rand() % 256;
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 1: Complex multi-dimensional array access - triggers RELOAD_FOR_INPUT_ADDRESS */
    /* arr[volatile_idx1][volatile_idx2 + volatile_idx3] pattern */
    temp1 = local_arr1[idx1 * 25 + (idx2 + idx3)];
    temp2 = local_arr2[(idx4 * 10) + idx1];
    
    /* Pointer chain dereferencing */
    ptr1 = &local_arr1[idx2];
    ptr2 = &local_arr2[idx3];
    ptr_to_ptr = &ptr1;
    
    /* Multi-level indirection */
    temp3 = **ptr_to_ptr;
    temp4 = *(ptr1 + idx4);
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 2: Mixed register class pressure - integer and floating point */
    /* Convert integer to float and back */
    dtemp1 = (double)temp1 * scale1;
    dtemp2 = (double)temp2 * scale2;
    
    /* Floating point array access with volatile indices */
    dtemp3 = dbl_arr1[idx1] * dbl_arr2[idx2];
    
    /* Integer computation using floating point results */
    temp5 = (int)(dtemp1 + dtemp2 + dtemp3);
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 3: Inline assembly with register constraints */
    /* Forces RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    
    /* Assembly that takes memory address as input */
    __asm__ volatile(
        "mov %[addr], %%rax\n\t"
        "mov (%%rax), %[out]\n\t"
        : [out] "=r" (ltemp1)          /* Output in register */
        : [addr] "m" (&local_arr1[idx1]) /* Memory address input */
        : "rax", "memory"
    );
    
    /* Assembly with earlyclobber and memory input */
    __asm__ volatile(
        "lea (%[base], %[index], 4), %[out]\n\t"
        : [out] "=&r" (lptr)           /* Earlyclobber output */
        : [base] "r" (local_arr1),     /* Base in register */
          [index] "r" (idx2)           /* Index in register */
        : "memory"
    );
    
    /* More complex assembly with multiple constraints */
    int asm_out1, asm_out2;
    __asm__ volatile(
        "imul %[in1], %[in2]\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %%edx, %[out2]\n\t"
        : [out1] "=r" (asm_out1), [out2] "=r" (asm_out2)
        : [in1] "r" (temp3), [in2] "r" (temp4)
        : "eax", "edx"
    );
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 4: Complex pointer arithmetic with different types */
    /* char* with scaling */
    cptr = char_arr + idx1 * sizeof(int) + idx2;
    
    /* int* with volatile offset */
    ptr1 = local_arr1 + idx3;
    
    /* double* with double offset calculation */
    dptr1 = dbl_arr1 + (int)(scale1 * 10);
    
    /* Mixed pointer arithmetic */
    ltemp2 = (long)(cptr - char_arr) + (long)(ptr1 - local_arr1);
    
    /* Dereference through computed pointers */
    temp1 = *ptr1;
    temp2 = *(int*)cptr;
    dtemp1 = *dptr1;
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 5: Nested addressing with volatile components */
    /* arr[volatile1][volatile2 + volatile3] with type conversion */
    double complex_result = 
        dbl_arr1[idx1] * (double)local_arr1[idx2 + idx3] +
        dbl_arr2[idx4] * (double)local_arr2[idx1 * 2];
    
    /* Pointer to pointer with offset */
    int **pptr = (int**)&ptr_to_ptr;
    int *deref_ptr = *pptr + idx4;
    temp3 = *deref_ptr;
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* SCENARIO 6: Output address reloads */
    /* Store through computed pointer address */
    int *output_ptr = &local_arr1[idx1 + idx2];
    *output_ptr = temp1 + temp2 + asm_out1;
    
    /* Store to array with complex index */
    local_arr2[(idx3 * idx4) % 50] = temp3 + asm_out2;
    
    /* Store double with conversion */
    dbl_arr2[idx1 % 40] = (double)(temp4 * temp5) * scale1;
    
    /* Final computation using all values */
    double final_result = 
        complex_result + 
        (double)temp1 * scale2 + 
        (double)temp2 * 0.5 +
        dtemp1 * dtemp2 +
        (double)ltemp1 * 0.01 +
        (double)ltemp2 * 0.001;
    
    /* Force all values to be considered live */
    __asm__ volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3), 
                     "r"(temp4), "r"(temp5), "r"(asm_out1), "r"(asm_out2),
                     "r"(ltemp1), "r"(ltemp2) : "memory");
    
    return final_result;
}

int main(void)
{
    srand(time(NULL));
    
    /* Initialize volatile globals */
    v1 = rand() % 100;
    v2 = rand() % 100;
    v3 = rand() % 100;
    v4 = rand() % 100;
    v5 = rand() % 100;
    v6 = rand() % 100;
    v7 = rand() % 100;
    v8 = rand() % 100;
    
    vl1 = rand() % 1000;
    vl2 = rand() % 1000;
    vl3 = rand() % 1000;
    
    vd1 = (rand() % 100) * 0.1;
    vd2 = (rand() % 100) * 0.1;
    
    /* Call the function with volatile arguments */
    double result = trigger_reloads(
        v1, v2, v3, v4,
        vl1, vl2,
        vd1, vd2
    );
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %f\n", result);
    
    /* Additional calls with different arguments to explore more paths */
    result += trigger_reloads(
        v5, v6, v7, v8,
        vl3, vl1,
        vd2, vd1
    );
    
    printf("Final result: %f\n", result);
    
    return 0;
}
