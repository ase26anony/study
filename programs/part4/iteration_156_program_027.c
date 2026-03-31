/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency (distance 1) */
int func1_carried_dep(volatile int* arr, int n) {
    int sum = arr[0];
    for (int i = 1; i < n; i++) {
        /* Loop-carried dependency: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * 3 + arr[i];
        sum += arr[i];
        
        /* Mix integer operations with different latencies */
        sum = (sum << 2) | (sum >> 30);  /* Rotation */
        sum = sum * 7 + i;               /* Multiplication */
    }
    return sum;
}

/* Function 2: Mixed integer/float operations with dependency */
double func2_mixed_ops(double* darr, int* iarr, int n) {
    double acc = 1.0;
    int int_acc = iarr[0];
    
    for (int i = 1; i < n; i++) {
        /* Floating point operations (higher latency) */
        acc = acc * 1.01 + darr[i];
        
        /* Integer operations with carried dependency */
        int_acc = int_acc * 2 + iarr[i];
        
        /* Memory access pattern */
        darr[i] = acc * 0.5;
        iarr[i] = int_acc & 0xFF;
        
        /* Compiler barrier to preserve operations */
        asm volatile("" : : "r"(acc), "r"(int_acc) : "memory");
    }
    return acc + int_acc;
}

/* Function 3: Pointer chasing pattern */
int func3_pointer_chase(int* base, int n) {
    int* p = base;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pointer chasing creates dependencies */
        sum += *p;
        p = base + (sum & (n-1));  /* Non-linear access */
        
        /* Additional arithmetic to increase complexity */
        sum = sum * 13 + i;
        sum = sum ^ (sum >> 16);
    }
    return sum;
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested_conditional(int* arr, int n, int threshold) {
    int outer_sum = 0;
    
    /* Outer loop */
    for (int j = 0; j < 5; j++) {
        int inner_sum = arr[0];
        
        /* Inner loop with conditional execution */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Multiple independent statements followed by dependent store */
                int temp1 = arr[i] * 2;
                int temp2 = arr[i-1] + 1;
                int temp3 = temp1 - temp2;
                
                /* Dependent operation */
                arr[i] = inner_sum + temp3;
                inner_sum = arr[i] * 3;
                
                /* Conditional operation inside loop */
                if (i % 8 == 0) {
                    inner_sum = inner_sum >> 1;
                }
            }
        }
        outer_sum += inner_sum;
    }
    return outer_sum;
}

/* Function 5: Reduction with multiple accumulators */
long func5_multiple_reductions(int* arr, int n) {
    long sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Independent accumulations */
        sum1 += arr[i];
        sum2 += arr[i] * i;
        sum3 += arr[i] * arr[i];
        
        /* Cross-dependency every few iterations */
        if (i % 4 == 0) {
            sum1 = sum1 ^ sum2;
            sum2 = sum2 + sum3;
        }
        
        /* Prevent vectorization with volatile */
        volatile int barrier = arr[i];
        (void)barrier;
    }
    return sum1 + sum2 + sum3;
}

int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    double darr[SIZE];
    int arr3[SIZE];
    int arr4[SIZE];
    int arr5[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = (i * 3) % 100;
        darr[i] = i * 0.1;
        arr3[i] = (i * 7) % 256;
        arr4[i] = i * 11;
        arr5[i] = (i * 13) % 512;
    }
    
    /* Execute all functions to ensure they're compiled */
    int result1 = func1_carried_dep((int*)arr1, ITERS);
    double result2 = func2_mixed_ops(darr, arr2, ITERS);
    int result3 = func3_pointer_chase(arr3, ITERS);
    int result4 = func4_nested_conditional(arr4, ITERS, 1);
    long result5 = func5_multiple_reductions(arr5, ITERS);
    
    /* Use results to prevent dead code elimination */
    global_sink = result1 + (int)result2 + result3 + result4 + (int)result5;
    
    /* Print minimal output */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
