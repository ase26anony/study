/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n, int scalar) {
    int result = 0;
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix integer and floating point operations */
        float temp = (float)arr[i-1] * 1.5f + (float)arr[i];
        arr[i] = (int)temp + scalar;
        
        /* Additional integer operations to create more scheduling opportunities */
        result ^= arr[i] * 3;
        result += (arr[i-1] & 0xFF) | 0x1;
    }
    
    /* Compiler barrier to preserve dependencies */
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2_reduction(volatile int* arr, int n) {
    int sum = arr[0];
    volatile int* p = arr;
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with arithmetic */
        int val = *p;
        p = &arr[i];
        
        /* Mixed operations with different latencies */
        sum += val * 7;
        sum -= arr[i] / 3;
        
        /* Floating point operation in the mix */
        float fsum = (float)sum * 0.25f;
        sum = (int)fsum + (val & 0xF);
    }
    
    /* Force dependency chain */
    asm volatile("" : "+r"(sum) : : "memory");
    return sum;
}

/* Function 3: Loop with multiple independent statements and dependent store */
int func3_multi_dep(volatile int* arr, volatile float* farr, int n) {
    int acc1 = arr[0];
    int acc2 = arr[1];
    float facc = farr[0];
    
    for (int i = 2; i < n; i++) {
        /* Independent computations */
        int tmp1 = acc1 * 11;
        int tmp2 = acc2 + arr[i-1];
        float ftmp = facc * 2.7f;
        
        /* Dependent store with carried dependency */
        acc1 = tmp2 ^ 0xAA;
        acc2 = tmp1 - (int)ftmp;
        facc = (float)arr[i] * 0.33f;
        
        /* Store with dependency on previous iteration */
        arr[i] = acc1 + acc2 + (int)facc;
        
        /* Additional operation to create more edges */
        farr[i] = ftmp + (float)(arr[i-2] & 0xFF);
    }
    
    asm volatile("" : : "r"(acc1), "r"(acc2), "r"(facc) : "memory");
    return acc1 + acc2 + (int)facc;
}

/* Function 4: Nested loop with conditional inner logic */
int func4_nested(volatile int* arr, int n, int threshold) {
    int total = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 3; outer++) {
        /* Conditional to prevent dead code elimination */
        if (outer % 2 == 0) {
            /* Inner loop with carried dependency */
            int inner_acc = arr[0];
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                inner_acc = inner_acc * 3 + arr[i];
                
                /* Conditional operation inside loop */
                if (inner_acc > threshold) {
                    inner_acc -= 100;
                } else {
                    inner_acc += arr[i-1] & 0x7;
                }
                
                /* Floating point in conditional path */
                float ftemp = (float)inner_acc * 0.123f;
                arr[i] = (int)ftemp + outer;
            }
            total += inner_acc;
        }
    }
    
    asm volatile("" : "+r"(total) : : "memory");
    return total;
}

/* Function 5: Loop with varying dependency distances */
int func5_varying_dist(volatile int* arr, int n) {
    int result = 0;
    
    /* Loop with potential distance > 1 dependencies */
    for (int i = 4; i < n; i++) {
        /* Multiple dependencies with different distances */
        int dep1 = arr[i-1];  /* distance 1 */
        int dep2 = arr[i-3];  /* distance 3 */
        int dep4 = arr[i-4];  /* distance 4 */
        
        /* Complex expression mixing dependencies */
        result = (dep1 * dep2) + (dep4 ^ 0x55);
        result = result * 5 - (dep1 & 0xF);
        
        /* Store creating anti-dependencies */
        arr[i] = result + i;
        
        /* Floating point to increase latency diversity */
        float fcalc = (float)result * 1.618f;
        result = (int)fcalc | (dep2 & 0xFF);
    }
    
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

int main() {
    /* Initialize data arrays */
    volatile int int_arr[SIZE];
    volatile float float_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 3 + 7) & 0xFF;
        float_arr[i] = (float)i * 0.5f;
    }
    
    int scalar = 42;
    int threshold = 1000;
    
    /* Call all functions to ensure they're compiled and executed */
    int r1 = func1_carried_dep(int_arr, ITERS, scalar);
    int r2 = func2_reduction(int_arr, ITERS);
    int r3 = func3_multi_dep(int_arr, float_arr, ITERS);
    int r4 = func4_nested(int_arr, ITERS, threshold);
    int r5 = func5_varying_dist(int_arr, ITERS);
    
    /* Use results to prevent dead code elimination */
    sink = r1 + r2 + r3 + r4 + r5;
    
    /* Print minimal output to verify execution */
    printf("Result checksum: %d\n", sink);
    
    return 0;
}
