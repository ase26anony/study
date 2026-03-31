/* test_modulo_sched.c - Test program for GCC modulo scheduler coverage */

#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent dead code elimination */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n) {
    int sum = arr[0];
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with carried dependency */
        int temp = arr[i-1] * 3 + 7;
        arr[i] = (temp >> 2) + arr[i];
        sum += arr[i];
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 2: Loop with pointer chasing pattern */
int func2_pointer_chase(volatile int* arr, int n) {
    int val = arr[0];
    for (int i = 1; i < n; i++) {
        /* Pointer-chasing like pattern */
        val = arr[val & (n-1)];
        arr[i] = val * i + arr[i-1];
        
        /* Mix in floating point operations */
        float fval = (float)val;
        fval = fval * 1.5f + 2.0f;
        val = (int)fval;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(val) : "memory");
    }
    return val;
}

/* Function 3: Loop with multiple independent statements and dependent store */
int func3_multi_ops(volatile int* arr, volatile float* farr, int n) {
    int acc1 = 1, acc2 = 2, acc3 = 3;
    float facc = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Independent computations */
        acc1 = acc1 * 3 + i;
        acc2 = acc2 ^ (acc1 << 2);
        acc3 = acc3 + (acc2 >> 1);
        
        /* Floating point operations */
        facc = facc * 1.1f + (float)i;
        
        /* Dependent store with memory access */
        arr[i] = acc1 + acc2 + acc3 + (int)facc;
        farr[i] = facc;
        
        /* Loop-carried dependency */
        if (i > 0) {
            arr[i] += arr[i-1] / 2;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(acc1), "r"(acc2), "r"(facc) : "memory");
    }
    return acc1 + acc2 + acc3 + (int)facc;
}

/* Function 4: Nested loop with conditional inner logic */
int func4_nested_conditional(volatile int* arr, int n, int threshold) {
    int total = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Conditional inner loop */
        if (threshold > 0) {
            int local_sum = arr[0];
            for (int i = 1; i < n; i++) {
                /* Complex carried dependency */
                int prev = arr[i-1];
                int curr = arr[i];
                
                /* Mixed operations */
                int prod = prev * curr + outer;
                int shifted = prod >> (i & 3);
                
                arr[i] = shifted + (prev % (curr + 1)) + i;
                local_sum += arr[i];
                
                /* Floating point in conditional */
                if (i % 8 == 0) {
                    float ftemp = (float)local_sum;
                    ftemp = ftemp * 0.75f;
                    local_sum = (int)ftemp;
                }
                
                /* Memory barrier */
                asm volatile("" : : "r"(local_sum) : "memory");
            }
            total += local_sum;
        }
    }
    return total;
}

/* Function 5: Reduction loop with multiple dependencies */
float func5_reduction(volatile float* arr, int n) {
    float sum1 = 0.0f, sum2 = 0.0f;
    float prod = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Multiple reduction variables */
        sum1 += arr[i];
        sum2 += arr[i] * (float)i;
        
        /* Dependent multiplication chain */
        prod *= (arr[i] + 1.0f);
        
        /* Integer operations mixed in */
        int int_val = (int)arr[i];
        sum1 += (float)(int_val & 0xFF);
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum1), "r"(prod) : "memory");
    }
    
    /* Cross dependency between reductions */
    return sum1 * 0.3f + sum2 * 0.7f + prod;
}

int main() {
    /* Initialize data arrays */
    volatile int int_arr[SIZE];
    volatile float float_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 3 + 7) & 0xFF;
        float_arr[i] = (float)(i * 2 + 1) * 0.5f;
    }
    
    /* Call all functions to ensure they're compiled */
    int result1 = func1_carried_dep(int_arr, ITERS);
    int result2 = func2_pointer_chase(int_arr, ITERS);
    int result3 = func3_multi_ops(int_arr, float_arr, ITERS);
    int result4 = func4_nested_conditional(int_arr, ITERS, 1);
    float result5 = func5_reduction(float_arr, ITERS);
    
    /* Use results to prevent dead code elimination */
    global_sink = result1 + result2 + result3 + result4 + (int)result5;
    
    /* Print minimal output */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
