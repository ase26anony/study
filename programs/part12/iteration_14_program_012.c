/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force complex addressing and prevent optimizations */
#define NOINLINE __attribute__((noinline, noipa))
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Global volatile variables to force dynamic addressing */
volatile int volatile_idx1 = 0;
volatile int volatile_idx2 = 0;
volatile int volatile_idx3 = 0;
volatile int volatile_stride = 0;
volatile double volatile_scale = 0.0;
volatile long volatile_offset1 = 0;
volatile long volatile_offset2 = 0;

/* Complex multi-dimensional array access with mixed types */
NOINLINE double trigger_reloads(int seed, volatile int* vptr1, volatile int* vptr2) {
    /* Create high register pressure with many live values */
    int a[100];          /* Integer array - pressure on integer registers */
    double b[50];        /* Double array - pressure on FP registers */
    int c[75];           /* More integer pressure */
    double d[40];        /* More FP pressure */
    
    /* Additional scalars to increase register pressure */
    int scalar1, scalar2, scalar3, scalar4, scalar5;
    double fscalar1, fscalar2, fscalar3, fscalar4;
    
    /* Pointer variables for complex addressing */
    int* ptr1;
    int* ptr2;
    double* fptr1;
    double* fptr2;
    
    /* Initialize with seed to prevent constant propagation */
    srand(seed);
    for (int i = 0; i < 100; i++) a[i] = rand() % 1000;
    for (int i = 0; i < 50; i++) b[i] = (double)(rand() % 1000) / 10.0;
    for (int i = 0; i < 75; i++) c[i] = rand() % 1000;
    for (int i = 0; i < 40; i++) d[i] = (double)(rand() % 1000) / 10.0;
    
    COMPILER_BARRIER();
    
    /* Complex addressing pattern 1: Multi-dimensional array access with volatile indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            /* Complex address calculation that needs temporary register */
            int idx = i * volatile_stride + j + volatile_idx1;
            a[idx] = (int)(b[volatile_idx2 + j] * volatile_scale) + c[volatile_idx3 + i];
        }
    }
    
    COMPILER_BARRIER();
    
    /* Complex addressing pattern 2: Pointer chains with mixed types */
    /* This should trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    ptr1 = &a[volatile_idx1] + volatile_idx2;
    ptr2 = ptr1 + volatile_idx3;
    
    /* Force pointer arithmetic that requires address reloads */
    for (int i = 0; i < 20; i++) {
        int* temp_ptr = ptr2 + i * 2;
        *temp_ptr = *temp_ptr + *(ptr1 + i);
    }
    
    COMPILER_BARRIER();
    
    /* Mixed integer/float operations to pressure different register classes */
    /* This should trigger RELOAD_OTHER and general reloads */
    fscalar1 = 0.0;
    for (int i = 0; i < 30; i++) {
        /* Integer to float conversion forces register class transfers */
        fscalar1 += (double)a[i] * b[i % 20];
        scalar1 = (int)fscalar1;  /* Float to int conversion */
        c[i] = scalar1 + a[i + 10];
    }
    
    COMPILER_BARRIER();
    
    /* Inline assembly blocks to force specific reload types */
    /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    
    /* Assembly 1: Memory operand with complex addressing */
    scalar2 = 0;
    __asm__ volatile (
        "movl %[input], %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[output]"
        : [output] "=r" (scalar2)          /* Output in register */
        : [input] "m" (a[volatile_idx1])   /* Memory input with complex address */
        : "%eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Assembly 2: Address computation in input */
    scalar3 = 0;
    int* addr_temp;
    __asm__ volatile (
        "leal (%[base], %[index], 4), %[addr]\n\t"
        "movl (%[addr]), %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r" (scalar3), [addr] "=&r" (addr_temp)
        : [base] "r" (a), [index] "r" (volatile_idx2)
        : "%eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Assembly 3: Output address reload */
    scalar4 = 0;
    int output_addr;
    __asm__ volatile (
        "movl %[val], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        "leal %[out], %[addr]"
        : [out] "=m" (c[volatile_idx3]), [addr] "=r" (output_addr)
        : [val] "r" (scalar2)
        : "%eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* More complex addressing with different pointer types */
    /* This should trigger RELOAD_FOR_OTHER_ADDRESS */
    char* char_ptr = (char*)a;
    for (int i = 0; i < 50; i++) {
        /* Byte access with scaling */
        char_ptr[i * 4 + volatile_idx1] = (char)(b[i % 20] * 10.0);
    }
    
    COMPILER_BARRIER();
    
    /* Nested pointer indirection simulation */
    int** ptr_to_ptr = &ptr1;
    for (int i = 0; i < 10; i++) {
        *(*ptr_to_ptr + i) = *(*ptr_to_ptr + i + 5) + volatile_idx2;
    }
    
    COMPILER_BARRIER();
    
    /* Final computation mixing all types */
    double result = 0.0;
    for (int i = 0; i < 25; i++) {
        /* Complex address calculation */
        int idx1 = (volatile_idx1 + i) % 50;
        int idx2 = (volatile_idx2 + i * 2) % 40;
        
        /* Mixed-type computation */
        result += (double)a[idx1] * d[idx2];
        result -= (double)c[idx1] / (b[idx2] + 1.0);
        
        /* More pointer arithmetic */
        fptr1 = &b[idx1];
        fptr2 = &d[idx2];
        *fptr1 = *fptr1 * 0.95;
        *fptr2 = *fptr2 * 1.05;
    }
    
    COMPILER_BARRIER();
    
    /* Use all volatile parameters to prevent optimization */
    result += *vptr1 + *vptr2;
    
    return result;
}

int main() {
    /* Initialize volatile variables with random values */
    srand(time(NULL));
    
    volatile_idx1 = rand() % 50;
    volatile_idx2 = rand() % 40;
    volatile_idx3 = rand() % 30;
    volatile_stride = 10 + rand() % 20;
    volatile_scale = 1.0 + (double)(rand() % 100) / 50.0;
    volatile_offset1 = rand() % 100;
    volatile_offset2 = rand() % 100;
    
    /* Additional volatile variables for function arguments */
    volatile int varg1 = rand() % 1000;
    volatile int varg2 = rand() % 1000;
    
    /* Call the function that should trigger multiple reload types */
    double result = trigger_reloads(rand(), &varg1, &varg2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", result);
    printf("Indices: %d, %d, %d\n", volatile_idx1, volatile_idx2, volatile_idx3);
    
    return 0;
}
