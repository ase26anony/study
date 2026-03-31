/* Test program to trigger modulo scheduling edge printing */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n, int scalar) {
    int result = 0;
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mixed operations with different latencies */
        int temp = arr[i-1] * scalar;      /* Integer multiply */
        temp = temp + (temp >> 3);         /* Shift and add */
        arr[i] = temp + arr[i];            /* Dependency chain */
        result += arr[i];                  /* Reduction */
    }
    return result;
}

/* Function 2: Pointer chasing with floating point mix */
float func2_pointer_chase(float* farr, int* iarr, int n) {
    float sum = 0.0f;
    int idx = 0;
    
    /* Pointer chasing loop */
    for (int i = 0; i < n; i++) {
        /* Floating point operations (higher latency) */
        float fval = farr[idx] * 1.5f;
        fval = fval + 0.25f;
        
        /* Integer operations mixed in */
        idx = iarr[idx] & (n-1);  /* Modulo via bitmask */
        
        /* Store result with dependency */
        farr[idx] = fval;
        sum += fval;
        
        /* Compiler barrier to preserve operations */
        asm volatile("" : : "r"(idx) : "memory");
    }
    return sum;
}

/* Function 3: Multiple independent statements with final dependency */
void func3_multi_dep(int* arr1, int* arr2, int* arr3, int n, int k) {
    /* Outer loop to encourage modulo scheduling */
    for (int iter = 0; iter < k; iter++) {
        /* Inner loop with complex dependency pattern */
        for (int i = 2; i < n; i++) {
            /* Independent computations */
            int a = arr1[i] * 3;
            int b = arr2[i-1] + 7;
            int c = arr3[i-2] ^ 0xFF;
            
            /* Dependent computation with distance 2 */
            int d = (a + b) * c;
            
            /* Store with carried dependency */
            arr1[i] = d + arr1[i-2];  /* Distance-2 dependency */
            arr2[i] = b - a;
            arr3[i] = c | d;
        }
        
        /* Prevent loop invariant code motion */
        asm volatile("" : : "r"(arr1[n-1]) : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested_conditional(int* matrix, int rows, int cols, int threshold) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        /* Conditional to preserve inner loop */
        if (r % 2 == 0) {
            /* Inner loop with mixed operations */
            for (int c = 1; c < cols; c++) {
                /* Load-use dependency */
                int prev = matrix[r * cols + c - 1];
                
                /* Mixed latency operations */
                int val = prev * 7;
                val = (val << 2) | (val >> 30);  /* Rotate */
                val = val + matrix[r * cols + c];
                
                /* Store with dependency */
                matrix[r * cols + c] = val;
                total += val;
            }
        } else {
            /* Alternative path to avoid dead code elimination */
            for (int c = 0; c < cols; c++) {
                matrix[r * cols + c] += 1;
            }
        }
    }
    return total;
}

/* Function 5: Reduction with floating-point accumulation */
double func5_fp_reduction(double* darr, int n) {
    double sum = 0.0;
    
    /* Loop with FP carried dependency */
    for (int i = 0; i < n; i++) {
        /* High-latency FP operations */
        double val = darr[i] * 2.71828;
        val = val / 1.41421;
        
        /* Dependency chain */
        sum = sum + val;  /* FP accumulation dependency */
        
        /* Store back */
        darr[i] = val;
    }
    return sum;
}

int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    int arr3[SIZE];
    float farr[SIZE];
    double darr[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) & 0xFF;
        arr2[i] = (i * 5 + 11) & 0xFF;
        arr3[i] = (i * 7 + 13) & 0xFF;
        farr[i] = (float)(i * 0.1f + 0.5f);
        darr[i] = (double)(i * 0.01 + 0.25);
    }
    
    int result = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    result += func1_carried_dep((int*)arr1, SIZE, 3);
    
    float fresult = func2_pointer_chase(farr, (int*)arr2, SIZE/2);
    sink = (int)fresult;
    
    func3_multi_dep((int*)arr1, arr2, arr3, SIZE, 5);
    
    result += func4_nested_conditional((int*)arr1, 16, 16, 100);
    
    double dresult = func5_fp_reduction(darr, SIZE);
    sink += (int)dresult;
    
    /* Use results to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        result += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Final sink to prevent dead code elimination */
    sink = result;
    
    printf("Result: %d\n", result);
    return 0;
}
