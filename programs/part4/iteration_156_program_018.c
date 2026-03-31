/* test_modulo_sched.c - Program to trigger GCC's modulo scheduler edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 128
#define ITERS 100

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

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

/* Function 2: Reduction loop with floating point and integer mix */
float func2_reduction(float *farr, int *iarr, int n) {
    volatile float acc = 0.0f;
    volatile int int_acc = 0;
    
    for (int i = 0; i < n; i++) {
        /* Floating point operations (higher latency) */
        float fval = farr[i] * 1.5f;
        fval = fval + farr[(i+1) % n] * 0.5f;
        
        /* Integer operations mixed in */
        int ival = iarr[i] + (iarr[i] << 1);
        ival = ival ^ (ival >> 3);
        
        /* Cross-type dependency */
        acc = acc + fval * (float)ival;
        int_acc += ival & 0xFF;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(fval), "r"(ival) : "memory");
    }
    
    return acc + (float)int_acc;
}

/* Function 3: Pointer chasing with conditional inner logic */
int func3_pointer_chase(int *base, int n) {
    volatile int *current = base;
    int sum = 0;
    
    /* Outer loop to encourage modulo scheduling */
    for (int outer = 0; outer < 5; outer++) {
        current = base;
        
        /* Inner loop with pointer chasing */
        for (int i = 0; i < n; i++) {
            /* Conditional inside loop */
            if (current != NULL) {
                sum += *current;
                /* Pointer arithmetic with dependency */
                current = base + ((*current) % (n-1));
                
                /* Mixed operations on the value */
                sum = (sum * 3) ^ (sum >> 1);
            }
            
            /* Additional computation to increase loop body size */
            sum += (i & 1) ? (sum << 2) : (sum >> 2);
        }
        
        /* Prevent outer loop elimination */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Function 4: Multiple independent statements with final dependent store */
void func4_multiple_deps(double *darr, int *iarr, int n) {
    volatile double d1, d2, d3;
    volatile int i1, i2;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        d1 = darr[i-1] * 1.7;
        d2 = darr[i] * 2.3;
        i1 = iarr[i-1] * 3;
        i2 = iarr[i] * 5;
        
        /* Some cross dependencies */
        d3 = d1 + d2 * (double)(i1 ^ i2);
        
        /* Final store with multiple dependencies */
        darr[i] = d3 + (double)((i1 + i2) & 0xFF);
        
        /* Memory barrier */
        asm volatile("" : : "r"(d3), "r"(i1), "r"(i2) : "memory");
    }
}

/* Function 5: Nested loops with complex index calculations */
void func5_nested_complex(int *arr, int n) {
    volatile int temp;
    
    /* Outer loop */
    for (int j = 0; j < 3; j++) {
        /* Inner loop with complex addressing */
        for (int i = 1; i < n; i++) {
            /* Multiple array accesses with different patterns */
            int idx1 = (i * 7) % n;
            int idx2 = (i * 13) % n;
            int idx3 = (i * 23) % n;
            
            /* Computation with carried dependency */
            temp = arr[idx1] + arr[idx2];
            temp = temp * arr[idx3];
            temp = temp ^ (temp << (i & 7));
            
            /* Store with dependency */
            arr[i] = temp + arr[i-1];
            
            /* Additional floating point to increase latency diversity */
            float ftemp = (float)temp * 0.5f;
            asm volatile("" : : "r"(ftemp) : "memory");
        }
        
        /* Prevent outer loop elimination */
        asm volatile("" : : "r"(temp) : "memory");
    }
}

int main() {
    /* Initialize data arrays */
    int int_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 3 + 7) & 0xFF;
        float_arr[i] = (float)(i * 5 + 11) * 0.1f;
        double_arr[i] = (double)(i * 7 + 13) * 0.01;
    }
    
    /* Call all functions to ensure they're compiled and executed */
    func1_carried_dep(int_arr, SIZE, 3);
    
    float fresult = func2_reduction(float_arr, int_arr, SIZE);
    
    int iresult = func3_pointer_chase(int_arr, SIZE/2);
    
    func4_multiple_deps(double_arr, int_arr, SIZE);
    
    func5_nested_complex(int_arr, SIZE);
    
    /* Use results to prevent dead code elimination */
    global_sink = int_arr[SIZE-1] + (int)fresult + iresult + (int)double_arr[SIZE-1];
    
    /* Print minimal output to ensure execution */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
