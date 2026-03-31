/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization/inlining on critical function */
#define NO_OPT __attribute__((noinline, noipa, optimize("O0")))

/* Compiler barrier to extend live ranges */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Volatile variables to prevent constant propagation */
static volatile int v1, v2, v3, v4, v5, v6, v7, v8;
static volatile long vl1, vl2;
static volatile double vd1, vd2;

/* Complex addressing structure */
struct nested_ptr {
    int **pptr;
    long *lptr;
    double *dptr;
};

/* Main function to trigger reloads */
NO_OPT static double trigger_reloads(
    volatile int idx1, volatile int idx2, 
    volatile int idx3, volatile int idx4,
    volatile long lval1, volatile long lval2,
    volatile double dval1, volatile double dval2,
    struct nestedptr *nptr)
{
    /* Create high register pressure with many live values */
    int local_arr1[100];
    int local_arr2[50];
    double dbl_arr1[75];
    double dbl_arr2[25];
    long long_arr1[40];
    
    /* Various pointers for address calculations */
    int *ptr1, *ptr2, **pptr1;
    double *dptr1, *dptr2;
    long *lptr1;
    
    /* Scalar variables to increase register pressure */
    int s1, s2, s3, s4, s5, s6, s7, s8, s9, s10;
    double ds1, ds2, ds3, ds4, ds5;
    long ls1, ls2, ls3, ls4;
    
    /* Initialize arrays with volatile values */
    for (int i = 0; i < 50; i++) {
        local_arr1[i * idx1] = idx2 + i;  /* Complex addressing */
        local_arr2[i] = idx3 * i;
        COMPILER_BARRIER();
    }
    
    for (int i = 0; i < 25; i++) {
        dbl_arr1[i] = dval1 * i;
        dbl_arr2[i] = dval2 + i;
        COMPILER_BARRIER();
    }
    
    /* Multi-level pointer arithmetic - triggers RELOAD_FOR_INPUT_ADDRESS */
    ptr1 = &local_arr1[idx1 + idx2];
    pptr1 = &ptr1;
    
    /* Complex address calculation with multiple volatile indices */
    /* This should trigger RELOAD_FOR_INPADDR_ADDRESS */
    ptr2 = &local_arr2[(idx3 * idx4) + (idx1 >> 2)];
    
    /* Mixed-type pointer arithmetic */
    lptr1 = &long_arr1[lval1 % 40];
    dptr1 = &dbl_arr1[(int)(dval1) % 75];
    
    /* Inline assembly with memory constraints - triggers RELOAD_FOR_OPERAND_ADDRESS */
    int asm_result1, asm_result2;
    __asm__ volatile (
        "movl (%1), %0\n\t"
        "addl %2, %0"
        : "=&r" (asm_result1)          /* Early clobber output */
        : "r" (&local_arr1[idx1]),     /* Input address */
          "r" (idx2)                   /* Input register */
        : "memory"
    );
    
    /* Another assembly block with different constraints */
    long asm_result3;
    __asm__ volatile (
        "movq (%1, %2, 4), %0\n\t"     /* Base + index*4 addressing */
        : "=r" (asm_result3)
        : "r" (long_arr1),             /* Base address */
          "r" (lval2)                  /* Index */
        : "memory"
    );
    
    /* Complex floating-point with integer conversion - pressures multiple reg classes */
    for (int i = 0; i < 20; i++) {
        /* Mixed integer/float operations */
        ds1 = (double)local_arr1[i] * dval1;
        ds2 = (double)local_arr2[i] / dval2;
        ds3 = ds1 + ds2;
        
        /* Store with complex addressing */
        dbl_arr1[(idx1 + i) % 75] = ds3;
        COMPILER_BARRIER();
    }
    
    /* Nested array access with volatile indices - triggers various address reloads */
    double sum = 0.0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Multi-dimensional addressing pattern */
            int index = (i * idx1 + j * idx2) % 100;
            sum += local_arr1[index] * dbl_arr1[(i + j) % 75];
            
            /* More complex addressing through pointers */
            if (nptr && nptr->pptr) {
                int **pptr_tmp = nptr->pptr + i;
                if (*pptr_tmp) {
                    sum += **pptr_tmp;
                }
            }
        }
        COMPILER_BARRIER();
    }
    
    /* Inline assembly that takes address of local variable - triggers RELOAD_FOR_OUTADDR_ADDRESS */
    int output_var;
    int *output_ptr = &output_var;
    __asm__ volatile (
        "leaq %1, %%rax\n\t"
        "movl (%%rax), %%ebx\n\t"
        "addl $100, %%ebx\n\t"
        "movl %%ebx, %0"
        : "=m" (*output_ptr)           /* Memory output */
        : "r" (&local_arr2[idx3])      /* Input address */
        : "rax", "rbx", "memory"
    );
    
    /* Additional assembly with output address reload */
    double dbl_output;
    double *dbl_out_ptr = &dbl_output;
    __asm__ volatile (
        "movsd (%1), %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m" (*dbl_out_ptr)
        : "r" (&dbl_arr1[idx4]),       /* Input address */
          "r" (dval1)                  /* Double in register */
        : "xmm0", "memory"
    );
    
    /* Use all local variables to prevent optimization */
    s1 = local_arr1[0];
    s2 = local_arr2[0];
    ds1 = dbl_arr1[0];
    ds2 = dbl_arr2[0];
    ls1 = long_arr1[0];
    
    /* Final computation using all values */
    double final_result = sum + ds1 + ds2 + (double)s1 + (double)s2 + (double)ls1;
    final_result += (double)asm_result1 + (double)asm_result3 + dbl_output;
    
    COMPILER_BARRIER();
    return final_result + (double)output_var;
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
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
    vd1 = (double)(rand() % 1000) / 10.0;
    vd2 = (double)(rand() % 1000) / 10.0;
    
    /* Setup nested pointer structure */
    int *base_ptr = malloc(100 * sizeof(int));
    int **pptr = malloc(10 * sizeof(int*));
    for (int i = 0; i < 10; i++) {
        pptr[i] = &base_ptr[i * 10];
    }
    
    struct nested_ptr nptr = {
        .pptr = pptr,
        .lptr = &vl1,
        .dptr = &vd1
    };
    
    /* Call function with many volatile arguments */
    double result = trigger_reloads(v1, v2, v3, v4, vl1, vl2, vd1, vd2, &nptr);
    
    printf("Result: %f\n", result);
    printf("Volatile values: %d %d %d %d\n", v1, v2, v3, v4);
    
    /* Cleanup */
    free(pptr);
    free(base_ptr);
    
    return 0;
}
