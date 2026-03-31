/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency (distance 1) */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        varr[i] = varr[i-1] * scalar + varr[i];
        
        /* Mix in some floating point operations */
        float ftmp = (float)varr[i] * 1.5f;
        varr[i] += (int)ftmp;
    }
    
    /* Compiler barrier to preserve operations */
    asm volatile("" : : "r"(varr) : "memory");
}

/* Function 2: Reduction loop with mixed operations */
int func2_reduction(float *farr, int n) {
    volatile float sum = 0.0f;
    volatile int isum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Mix integer and floating point operations */
        isum += (int)farr[i];
        sum += farr[i] * 2.5f;
        
        /* Conditional to create complex control flow */
        if (i % 3 == 0) {
            sum = sum * 0.9f;
            isum ^= (i << 2);
        }
    }
    
    /* Create dependency between iterations */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] + farr[i] * 0.1f;
    }
    
    asm volatile("" : : "r"(sum), "r"(isum) : "memory");
    return isum + (int)sum;
}

/* Function 3: Pointer chasing pattern */
void func3_pointer_chase(int **ptr_arr, int n) {
    volatile int val = 0;
    
    /* Pointer chasing creates memory dependencies */
    for (int i = 0; i < n; i++) {
        val = **ptr_arr;
        *ptr_arr = (int *)((size_t)*ptr_arr + sizeof(int));
        
        /* Mix in bitwise operations */
        val = (val << 3) | (val >> 29);
        val ^= 0xABCD1234;
    }
    
    asm volatile("" : : "r"(val) : "memory");
}

/* Function 4: Multiple independent statements with dependent store */
void func4_mixed_ops(double *darr, int *iarr, int n) {
    volatile double dacc = 1.0;
    volatile int iacc = 1;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        double dtmp1 = darr[i] * 3.14159;
        double dtmp2 = darr[i-1] * 2.71828;
        int itmp1 = iarr[i] * 7;
        int itmp2 = iarr[i-1] * 11;
        
        /* Dependent store with carried dependency */
        darr[i] = dtmp1 + dtmp2 + dacc;
        iarr[i] = itmp1 ^ itmp2 + iacc;
        
        /* Update accumulators with dependency */
        dacc = darr[i] * 0.99;
        iacc = iarr[i] & 0xFF;
        
        /* Memory barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
    }
}

/* Function 5: Nested loops with conditional inner logic */
int func5_nested_loops(int *arr, int n, int m) {
    volatile int result = 0;
    
    /* Outer loop */
    for (int j = 0; j < m; j++) {
        /* Conditional that prevents dead code elimination */
        if (j % 2 == 0) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < n; i++) {
                arr[i] = arr[i-1] + arr[i] * (j + 1);
                
                /* Mix operations */
                float ftmp = (float)arr[i] / (j + 2.0f);
                arr[i] += (int)(ftmp * 100.0f);
            }
            result += arr[n-1];
        } else {
            /* Alternative path */
            for (int i = n-1; i > 0; i--) {
                arr[i-1] = arr[i] - arr[i-1] * 2;
            }
        }
    }
    
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Main function that exercises all patterns */
int main() {
    /* Initialize data arrays */
    int int_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    int *ptr_arr[SIZE/4];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i * 3 + 1;
        float_arr[i] = (float)i * 1.7f;
        double_arr[i] = (double)i * 2.3;
        if (i % 4 == 0 && i/4 < SIZE/4) {
            ptr_arr[i/4] = &int_arr[i];
        }
    }
    
    /* Call all functions to ensure they're compiled */
    func1_carried_dep(int_arr, SIZE, 3);
    
    int res2 = func2_reduction(float_arr, SIZE);
    global_sink += res2;
    
    func3_pointer_chase(ptr_arr, SIZE/4);
    
    func4_mixed_ops(double_arr, int_arr, SIZE);
    
    int res5 = func5_nested_loops(int_arr, SIZE, 5);
    global_sink += res5;
    
    /* Use results to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += int_arr[i];
        checksum += (int)float_arr[i];
        checksum += (int)double_arr[i];
    }
    
    /* Print minimal output */
    printf("Result: %d (checksum: %d)\n", global_sink, checksum % 1000);
    
    return 0;
}
