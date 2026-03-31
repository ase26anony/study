/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n, int scalar) {
    int result = 0;
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mix integer and floating point operations */
        int temp = arr[i-1] * scalar;
        float ftemp = (float)temp * 1.5f;
        arr[i] = (int)ftemp + arr[i] + (i & 0xF); /* Bitwise operation */
        
        /* Create compiler barrier */
        asm volatile("" : "+r"(arr[i]) : : "memory");
    }
    
    /* Use result to prevent elimination */
    for (int i = 0; i < n; i++) {
        result += arr[i];
    }
    return result;
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2_reduction(volatile int* arr, int n) {
    int sum = arr[0];
    volatile int* ptr = arr;
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with dependency */
        int val = *ptr;
        ptr = &arr[i];
        
        /* Mixed operations */
        sum += val * 3 + (sum >> 2); /* Use previous sum */
        
        /* Floating point in reduction */
        float fsum = (float)sum * 0.75f;
        sum = (int)fsum + (arr[i] & 0x7);
        
        /* Memory barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 3: Loop with multiple independent then dependent operations */
void func3_mixed_ops(volatile float* farr, volatile int* iarr, int n) {
    float acc = 1.0f;
    int iacc = 1;
    
    for (int i = 0; i < n; i++) {
        /* Independent floating point operations */
        float f1 = farr[i] * 2.5f;
        float f2 = farr[(i+1) % n] * 1.8f;
        
        /* Independent integer operations */
        int i1 = iarr[i] + (i << 2);
        int i2 = iarr[(i+2) % n] * 3;
        
        /* Dependent operation mixing both */
        acc = acc * f1 + f2;  /* Carried dependency on acc */
        iacc = (iacc & i1) | i2;  /* Carried dependency on iacc */
        
        /* Store results with dependency */
        farr[i] = acc;
        iarr[i] = iacc;
        
        /* Compiler barrier */
        asm volatile("" : "+r"(acc), "+r"(iacc) : : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested(volatile int* arr, int n, int threshold) {
    int total = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with carried dependency */
        int inner_sum = arr[0];
        for (int i = 1; i < n; i++) {
            /* Conditional operation inside loop */
            if (arr[i] > threshold) {
                inner_sum = inner_sum * 2 + arr[i-1]; /* Distance-1 dependency */
            } else {
                inner_sum = inner_sum / 2 - arr[i-1];
            }
            
            /* Mix in some floating point */
            float ftemp = (float)inner_sum * 1.1f;
            arr[i] = (int)ftemp + (i % 8);
            
            /* Barrier */
            asm volatile("" : "+r"(inner_sum) : : "memory");
        }
        total += inner_sum;
    }
    return total;
}

/* Function 5: Complex dependency pattern with multiple recurrences */
void func5_complex(volatile int* arr1, volatile float* arr2, int n) {
    int dep1 = arr1[0];
    float dep2 = arr2[0];
    
    for (int i = 1; i < n; i++) {
        /* Two independent carried dependencies */
        dep1 = dep1 * 3 + arr1[i-1];  /* Distance 1 */
        dep2 = dep2 * 1.7f + arr2[i-1] * 0.5f;  /* Distance 1 */
        
        /* Cross dependency between int and float */
        int cross = (int)dep2 + dep1;
        float fcross = (float)dep1 * dep2;
        
        /* Store with dependencies */
        arr1[i] = cross & 0xFF;
        arr2[i] = fcross * 0.9f;
        
        /* Strong barrier */
        asm volatile("" : "+r"(dep1), "+r"(cross) : : "memory");
    }
}

int main() {
    /* Initialize data arrays */
    volatile int int_array[SIZE];
    volatile float float_array[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 3 + 1) % 100;
        float_array[i] = (float)(i * 2 + 1) / 10.0f;
    }
    
    int result = 0;
    
    /* Call each function multiple times with different parameters */
    for (int iter = 0; iter < ITERS; iter++) {
        result += func1_carried_dep(int_array, SIZE, iter + 2);
        result += func2_reduction(int_array, SIZE);
        
        func3_mixed_ops(float_array, int_array, SIZE);
        result += func4_nested(int_array, SIZE, 50);
        
        func5_complex(int_array, float_array, SIZE);
        
        /* Modify arrays slightly each iteration */
        int_array[iter % SIZE] = result % 1000;
        float_array[iter % SIZE] = (float)(result % 100) / 10.0f;
    }
    
    /* Final sink to prevent elimination */
    global_sink = result;
    
    /* Print minimal result to ensure execution */
    printf("Result: %d\n", global_sink);
    
    return 0;
}
