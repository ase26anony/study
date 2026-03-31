/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = varr[i-1] * scalar;
        temp = temp + (temp >> 3);      /* Bitwise operation */
        temp = temp ^ (temp << 2);      /* More bitwise ops */
        varr[i] = temp + varr[i];
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : : "r"(temp) : "memory");
    }
}

/* Function 2: Reduction loop with floating point operations */
float func2_reduction(float *farr, int n) {
    volatile float *vfarr = (volatile float *)farr;
    float sum = vfarr[0];
    
    /* Reduction with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mix of float and int operations */
        sum = sum * 1.01f + vfarr[i];
        
        /* Integer operation in the middle */
        int idx = i & 0xFF;
        sum += (float)idx * 0.5f;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 3: Pointer chasing pattern */
void func3_pointer_chase(int **parr, int n) {
    volatile int **vparr = (volatile int **)parr;
    
    /* Pointer chasing creates dependencies */
    int *current = vparr[0];
    for (int i = 1; i < n; i++) {
        /* Load through pointer */
        int val = *current;
        
        /* Some computation */
        val = val * 3 + i;
        val = val | (val << 16);
        
        /* Store and update pointer */
        *current = val;
        current = vparr[i % n];
        
        asm volatile("" : : "r"(val) : "memory");
    }
}

/* Function 4: Multiple independent statements with final dependency */
void func4_mixed_ops(double *darr, int n) {
    volatile double *vdarr = (volatile double *)darr;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        double a = vdarr[i] * 2.5;
        double b = vdarr[i-1] * 1.5;
        int c = (int)(a * 100) & 0xFF;
        
        /* Dependent computation */
        double result = a + b * (c % 7);
        
        /* Store with dependency */
        vdarr[i] = result + (double)i;
        
        /* Memory barrier */
        asm volatile("" : : "r"(result), "r"(c) : "memory");
    }
}

/* Function 5: Nested loops with conditional inner logic */
void func5_nested_conditional(int *arr, int n, int threshold) {
    volatile int *varr = (volatile int *)arr;
    
    /* Outer loop */
    for (int outer = 0; outer < 3; outer++) {
        /* Conditional inner loop */
        if (threshold > 0) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                int val = varr[i-1];
                val = (val * 7 + 12345) & 0xFFFF;
                val = val ^ (val >> 8);
                varr[i] = val + (i * outer);
                
                /* Mix in some floating point */
                float fval = (float)val * 0.1f;
                varr[i] += (int)fval;
                
                asm volatile("" : : "r"(val), "r"(fval) : "memory");
            }
        }
        threshold--;
    }
}

/* Main function that exercises all patterns */
int main() {
    /* Initialize arrays with non-zero values */
    int int_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    int *ptr_arr[SIZE];
    int data_pool[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i + 1;
        float_arr[i] = (float)(i * 1.5);
        double_arr[i] = (double)(i * 2.5);
        data_pool[i] = i * 3;
        ptr_arr[i] = &data_pool[i % SIZE];
    }
    
    /* Call all functions to ensure they're compiled */
    func1_carried_dep(int_arr, SIZE, 7);
    float sum = func2_reduction(float_arr, SIZE);
    func3_pointer_chase(ptr_arr, SIZE / 4);
    func4_mixed_ops(double_arr, SIZE);
    func5_nested_conditional(int_arr, SIZE, 2);
    
    /* Use results to prevent dead code elimination */
    sink += int_arr[SIZE-1];
    sink += (int)sum;
    sink += *ptr_arr[0];
    sink += (int)double_arr[SIZE-1];
    
    /* Print minimal output */
    printf("Result checksum: %d\n", sink);
    
    return 0;
}
