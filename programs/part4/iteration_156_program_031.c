/* test_modulo_sched.c - Test program for GCC modulo scheduler coverage */

#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void loop_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix of operations with different latencies */
        int temp = varr[i-1] * scalar;      /* Integer multiply */
        temp = temp + (temp >> 3);          /* Shift and add */
        temp = temp ^ (temp * 3);           /* XOR and multiply */
        varr[i] = temp + varr[i];           /* Dependency chain */
        
        /* Fake dependency barrier */
        asm volatile("" : : "r"(temp) : "memory");
    }
}

/* Function 2: Reduction loop with floating-point operations */
float loop_reduction(float *arr, int n) {
    volatile float *varr = (volatile float *)arr;
    float sum = varr[0];
    
    /* Reduction with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mix of floating-point and integer operations */
        float fp_val = varr[i] * 1.5f;      /* FP multiply */
        int int_part = (int)fp_val;         /* Type conversion */
        sum = sum + fp_val * (float)int_part; /* FP multiply-add */
        
        /* Prevent optimization */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 3: Pointer-chasing pattern with conditional */
int loop_pointer_chase(int *base, int n, int stride) {
    volatile int *vbase = (volatile int *)base;
    int *ptr = (int *)vbase;
    int sum = 0;
    
    /* Conditional inner loop */
    if (stride > 0) {
        for (int i = 0; i < n; i++) {
            /* Pointer chasing with memory dependency */
            int val = *ptr;
            ptr += stride;
            
            /* Complex dependency chain */
            val = val * val + i;            /* Square and add */
            val = (val << 2) | (val >> 30); /* Rotate */
            sum += val;                     /* Reduction */
            
            /* Memory barrier */
            asm volatile("" : : "r"(val) : "memory");
        }
    }
    return sum;
}

/* Function 4: Nested loops with inner recurrence */
void loop_nested_recurrence(int *arr, int n, int m) {
    volatile int *varr = (volatile int *)arr;
    
    /* Outer loop */
    for (int j = 0; j < m; j++) {
        int offset = j * n;
        
        /* Inner loop with recurrence */
        for (int i = 1; i < n; i++) {
            /* Multiple independent statements */
            int a = varr[offset + i - 1];
            int b = varr[offset + i];
            int c = a * b;
            int d = c + (a ^ b);
            
            /* Dependent store with distance */
            varr[offset + i] = d + (i % 8);
            
            /* Compiler barrier */
            asm volatile("" : : "r"(d) : "memory");
        }
    }
}

/* Function 5: Loop with multiple dependency distances */
void loop_multi_distance(double *arr, int n, int pattern) {
    volatile double *varr = (volatile double *)arr;
    
    /* Loop with varying dependency distances */
    for (int i = 4; i < n; i++) {
        /* Distance 1 dependency */
        double d1 = varr[i-1] * 0.5;
        
        /* Distance 2 dependency */
        double d2 = varr[i-2] + d1;
        
        /* Distance 4 dependency */
        double d4 = varr[i-4] * d2;
        
        /* Mix with integer operations */
        int idx = (int)d4 % n;
        varr[i] = d4 * varr[idx] + (double)pattern;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(d4) : "memory");
    }
}

/* Main driver */
int main() {
    /* Initialize arrays with non-zero values */
    int int_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 3 + 1) % 97;
        float_arr[i] = (float)(i * 5 + 2) / 7.0f;
        double_arr[i] = (double)(i * 7 + 3) / 11.0;
    }
    
    /* Call all loop functions multiple times */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Vary parameters to avoid constant propagation */
        int scalar = (iter % 7) + 2;
        int stride = (iter % 5) + 1;
        int pattern = iter % 11;
        
        /* Execute all loop patterns */
        loop_carried_dep(int_arr, SIZE - iter % 32, scalar);
        
        float sum_f = loop_reduction(float_arr, SIZE - iter % 16);
        sink += (int)sum_f;
        
        int sum_i = loop_pointer_chase(int_arr, SIZE / 2, stride);
        sink += sum_i;
        
        loop_nested_recurrence(int_arr, 64, 4);
        
        loop_multi_distance(double_arr, SIZE, pattern);
        
        /* Use results to prevent elimination */
        sink += int_arr[SIZE-1] + (int)double_arr[SIZE-1];
    }
    
    /* Final use of sink */
    printf("Result checksum: %d\n", sink);
    
    return 0;
}
