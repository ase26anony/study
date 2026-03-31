/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix of operations with different latencies */
        int temp = varr[i-1] * scalar;      /* integer multiply */
        temp = temp + (temp >> 3);          /* shift and add */
        temp = temp ^ (temp * 3);           /* xor with multiply */
        varr[i] = temp + varr[i];           /* dependent store */
        
        /* Floating point to increase latency diversity */
        float ftemp = (float)temp * 1.5f;
        if (ftemp > 1000.0f) {
            ftemp = ftemp * 0.9f;
        }
        /* Use result to create fake dependency */
        asm volatile("" : "+r"(temp) : : "memory");
    }
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2_reduction(int *arr, int n) {
    int sum = arr[0];
    volatile int *p = arr;
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent operations */
        int val1 = p[i] * 2;
        int val2 = p[i-1] + 7;
        int val3 = val1 ^ val2;
        
        /* Pointer chasing-like dependency */
        sum = sum + val3;
        sum = sum * 3 - 1;
        
        /* Memory barrier to preserve operations */
        asm volatile("" : : "r"(val1), "r"(val2), "r"(val3) : "memory");
    }
    return sum;
}

/* Function 3: Loop with multiple independent statements and dependent store */
void func3_mixed_ops(float *farr, int *iarr, int n) {
    volatile float *vf = farr;
    volatile int *vi = iarr;
    
    for (int i = 1; i < n; i++) {
        /* Independent floating point operations */
        float f1 = vf[i] * 2.5f;
        float f2 = vf[i-1] + 1.8f;
        float f3 = f1 * f2;
        
        /* Independent integer operations */
        int i1 = vi[i] & 0xFF;
        int i2 = vi[i-1] | 0x3F;
        int i3 = i1 * i2;
        
        /* Dependent store with mixed types */
        vf[i] = f3 + (float)i3;
        vi[i] = i3 + (int)f3;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(f1), "r"(f2), "r"(i1), "r"(i2) : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
void func4_nested(int *arr, int n, int threshold) {
    volatile int *varr = arr;
    
    /* Outer loop */
    for (int j = 0; j < 5; j++) {
        /* Conditional inner loop */
        if (threshold > 0) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < n; i++) {
                int val = varr[i-1];
                val = val * val + 7;        /* square and add */
                val = (val << 2) | (val >> 30); /* rotate */
                varr[i] = val ^ varr[i];
                
                /* Floating point in conditional */
                if (val > 100) {
                    float fval = (float)val * 0.01f;
                    asm volatile("" : : "r"(fval) : "memory");
                }
            }
        }
        /* Small computation between iterations */
        varr[0] = varr[0] + j;
    }
}

/* Function 5: Loop with different distance patterns */
void func5_variable_dist(int *arr, int n, int stride) {
    volatile int *varr = arr;
    
    for (int i = stride; i < n; i++) {
        /* Dependency with variable distance (determined by stride) */
        int base = varr[i - stride];
        int val = base * 3;
        
        /* Multiple uses with different latencies */
        for (int j = 0; j < 2; j++) {
            val = (val + 5) * 2;
        }
        
        varr[i] = val + varr[i];
        
        /* Memory barrier */
        asm volatile("" : : "r"(base), "r"(val) : "memory");
    }
}

int main() {
    /* Initialize arrays with non-zero values */
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *arr2 = (int *)malloc(SIZE * sizeof(int));
    float *arr3 = (float *)malloc(SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = (i * 3) % 97;
        arr3[i] = (float)i * 0.5f;
    }
    
    /* Call all functions to ensure they're compiled */
    func1_carried_dep(arr1, SIZE, 3);
    int sum1 = func2_reduction(arr1, SIZE);
    
    func3_mixed_ops(arr3, arr2, SIZE);
    func4_nested(arr2, SIZE, 1);
    func5_variable_dist(arr1, SIZE, 2);
    
    /* Use results to prevent dead code elimination */
    sink = sum1 + arr1[SIZE-1] + (int)arr3[SIZE-1] + arr2[SIZE-1];
    
    /* Print minimal output to ensure execution */
    printf("Result: %d\n", sink);
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
