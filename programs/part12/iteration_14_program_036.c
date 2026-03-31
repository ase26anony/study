/* reload_coverage.c - Program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2, volatile int scale,
    volatile long offset1, volatile long offset2, volatile char offset3
) {
    /* High register pressure with mixed types */
    int a[100];
    double b[50];
    float c[75];
    long d[60];
    char e[200];
    
    /* Complex pointer chains for address reloads */
    int *ptr1 = &a[idx1];
    int **ptr2 = &ptr1;
    int ***ptr3 = &ptr2;
    
    double *dptr1 = &b[idx2];
    double **dptr2 = &dptr1;
    
    /* Multi-dimensional access simulation */
    int result = 0;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS with complex addressing */
    for (volatile int i = 0; i < 10; i++) {
        /* Complex address calculation requiring temporary register */
        a[i * stride1 + idx3] = i * scale;
        
        /* Multi-level pointer dereference */
        result += ***(ptr3 + i % 5);
        
        /* Mixed register class pressure */
        b[i] = (double)a[i * stride2] * 1.5;
        c[i] = (float)b[i] + 0.5f;
        
        /* Memory barrier to prevent optimization */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Inline assembly to force specific reload types */
    int asm_out1, asm_out2;
    long asm_addr;
    
    /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    __asm__ volatile(
        "mov %[addr], %[out1]\n\t"
        "add $1, %[out1]\n\t"
        "mov %[out1], %[out2]"
        : [out1] "=&r" (asm_out1), [out2] "=r" (asm_out2)
        : [addr] "m" (a[idx1 + idx2])  /* Memory input */
        : "cc"
    );
    
    /* RELOAD_FOR_OUTPUT_ADDRESS with complex output */
    __asm__ volatile(
        "lea (%[base], %[idx], 4), %[out]"
        : [out] "=r" (asm_addr)
        : [base] "r" (a), [idx] "r" (idx1 * stride1 + idx2)
        : "cc"
    );
    
    /* More complex addressing with different types */
    char *char_ptr = e + offset3;
    int *int_ptr = (int *)(char_ptr + offset1);
    long *long_ptr = (long *)(int_ptr + offset2);
    
    /* Force RELOAD_FOR_INPADDR_ADDRESS */
    for (volatile int i = 0; i < 5; i++) {
        /* Complex address as input to another computation */
        d[i] = *(long_ptr + i * 2) + (long)(*(int_ptr + i));
        
        /* Floating-point operation to pressure FP registers */
        b[10 + i] = b[10 + i] * 2.0 + (double)d[i];
        
        __asm__ volatile("" : : : "memory");
    }
    
    /* Nested addressing for RELOAD_FOR_OTHER_ADDRESS */
    int ***complex_ptr = ptr3 + (idx1 % 3);
    int value = ***complex_ptr;
    
    /* Mixed integer/float conversions */
    double float_sum = 0.0;
    for (volatile int i = 0; i < 20; i++) {
        float_sum += (double)a[i] + b[i % 50] - (double)c[i % 75];
    }
    
    /* Final computation using all variables */
    result += (int)float_sum + asm_out1 + asm_out2 + (int)asm_addr + value;
    
    /* Use all arrays to prevent elimination */
    for (volatile int i = 0; i < 10; i++) {
        result += a[i] + (int)b[i] + (int)c[i] + (int)d[i] + e[i];
    }
    
    return result;
}

/* Second function with different patterns */
__attribute__((noinline, noipa))
static int trigger_more_reloads(
    volatile int base_idx, volatile int offset,
    volatile double scale_factor
) {
    /* Different array sizes for varied addressing */
    int arr1[150];
    double arr2[80];
    int *ptr_arr[30];
    
    /* Initialize */
    for (int i = 0; i < 150; i++) arr1[i] = i;
    for (int i = 0; i < 80; i++) arr2[i] = i * 1.1;
    
    /* Create pointer chain */
    for (int i = 0; i < 30; i++) {
        ptr_arr[i] = &arr1[i * 5];
    }
    
    int sum = 0;
    
    /* Complex array indexing with multiple dimensions simulated */
    for (volatile int i = 0; i < 25; i++) {
        /* Multi-level array access */
        int idx = base_idx + i * offset;
        sum += ptr_arr[i % 30][idx % 10];
        
        /* Floating-point computation */
        arr2[i % 80] = arr2[i % 80] * scale_factor + (double)sum;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Inline assembly with multiple constraints */
    int asm_result;
    double dbl_result;
    
    /* Force various reload types */
    __asm__ volatile(
        "cvtsi2sd %[input], %[dbl]\n\t"
        "mulsd %[scale], %[dbl]\n\t"
        "cvttsd2si %[dbl], %[out]"
        : [out] "=r" (asm_result), [dbl] "=x" (dbl_result)
        : [input] "r" (sum), [scale] "x" (scale_factor)
        : "cc"
    );
    
    return asm_result + sum;
}

int main(void) {
    srand(time(NULL));
    
    /* Volatile variables to prevent constant propagation */
    volatile int idx1 = rand() % 50;
    volatile int idx2 = rand() % 50;
    volatile int idx3 = rand() % 50;
    volatile int stride1 = rand() % 10 + 1;
    volatile int stride2 = rand() % 10 + 1;
    volatile int scale = rand() % 100;
    volatile long offset1 = rand() % 100;
    volatile long offset2 = rand() % 100;
    volatile char offset3 = rand() % 100;
    volatile double scale_factor = (double)(rand() % 100) / 10.0;
    
    printf("Testing reload coverage...\n");
    
    /* Call first function */
    int result1 = trigger_reloads(
        idx1, idx2, idx3, stride1, stride2, scale,
        offset1, offset2, offset3
    );
    
    /* Call second function with different parameters */
    int result2 = trigger_more_reloads(
        idx1 + idx2, idx3, scale_factor
    );
    
    int final_result = result1 + result2;
    printf("Result: %d\n", final_result);
    
    /* Additional test with different register pressure */
    {
        volatile int v1 = rand() % 100;
        volatile int v2 = rand() % 100;
        volatile double v3 = (double)rand() / RAND_MAX;
        
        /* Small function with high pressure */
        __attribute__((noinline))
        int small_test(volatile int x, volatile int y, volatile double z) {
            int arr[50];
            double darr[25];
            
            for (int i = 0; i < 50; i++) arr[i] = i + x;
            for (int i = 0; i < 25; i++) darr[i] = (double)i * z;
            
            int sum = 0;
            for (int i = 0; i < 25; i++) {
                sum += arr[i * 2] + (int)darr[i];
            }
            
            /* Assembly with memory address input */
            int out;
            __asm__ volatile(
                "mov (%[addr]), %[out]\n\t"
                "imul %[val], %[out]"
                : [out] "=r" (out)
                : [addr] "r" (&arr[y % 50]), [val] "r" (x)
                : "memory", "cc"
            );
            
            return sum + out;
        }
        
        int result3 = small_test(v1, v2, v3);
        printf("Additional result: %d\n", result3);
    }
    
    return 0;
}
