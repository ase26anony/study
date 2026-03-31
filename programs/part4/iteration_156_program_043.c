/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n, int scalar) {
    int sum = 0;
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mixed operations with different latencies */
        int temp = arr[i-1] * scalar;      /* Integer multiply */
        temp = temp + (temp >> 2);         /* Shift and add */
        arr[i] = temp + arr[i] * 3;        /* Another multiply */
        sum += arr[i];                     /* Reduction */
    }
    return sum;
}

/* Function 2: Pointer chasing pattern with floating point */
float func2_pointer_chase(float* data, int n, float scalar) {
    float* p = data;
    float result = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Pointer chasing with FP operations */
        float val = *p;
        val = val * scalar + 1.5f;         /* FP multiply-add */
        result += val;
        
        /* Conditional update to prevent simple analysis */
        if (val > 0) {
            p = data + (i % 16);
        } else {
            p = data + ((i + 1) % 16);
        }
        
        /* Memory barrier to preserve dependencies */
        asm volatile("" : : "r"(p) : "memory");
    }
    return result;
}

/* Function 3: Multiple independent statements with final dependency */
void func3_multi_dep(int* out, const int* in, int n, int k) {
    /* Outer loop to encourage modulo scheduling */
    for (int iter = 0; iter < k; iter++) {
        int acc1 = in[0];
        int acc2 = in[1];
        
        /* Inner loop with multiple accumulators */
        for (int i = 2; i < n; i++) {
            /* Independent computations */
            int t1 = acc1 * 3;
            int t2 = acc2 + in[i];
            
            /* Bitwise operations */
            t1 = t1 ^ (t1 << 3);
            t2 = t2 | 0x7F;
            
            /* Final dependent store */
            out[i] = t1 + t2 * iter;
            
            /* Update accumulators with carried dependency */
            acc1 = t2;
            acc2 = t1;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(out[n-1]) : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
double func4_nested_loops(double* matrix, int rows, int cols) {
    double total = 0.0;
    
    /* Outer loop */
    for (int r = 1; r < rows; r++) {
        /* Conditional to make analysis non-trivial */
        if (r % 2 == 0) {
            /* Inner loop with FP operations */
            for (int c = 1; c < cols; c++) {
                /* Distance-1 dependency in both dimensions */
                double val = matrix[(r-1)*cols + c] * 1.5;
                val += matrix[r*cols + (c-1)] * 0.5;
                val = val * val;  /* Square operation */
                matrix[r*cols + c] = val;
                total += val;
            }
        } else {
            /* Alternative path with different operations */
            for (int c = cols-2; c >= 0; c--) {
                matrix[r*cols + c] = matrix[r*cols + c+1] * 0.8;
                total -= matrix[r*cols + c];
            }
        }
    }
    return total;
}

/* Function 5: Reduction with mixed integer/FP and complex indexing */
long func5_complex_reduction(int* data, int n) {
    long sum = 0;
    float fp_acc = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Complex indexing pattern */
        int idx1 = (i * 7) % n;
        int idx2 = (i * 13) % n;
        
        /* Mixed integer operations */
        int val = data[idx1] + data[idx2];
        val = (val * 3) / 2;
        
        /* Floating point conversion and operation */
        fp_acc += (float)val * 0.25f;
        
        /* Integer reduction with dependency */
        sum += val + (int)fp_acc;
        
        /* Store back with carried dependency */
        data[idx1] = val % 256;
    }
    
    return sum + (long)fp_acc;
}

int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    float farr[SIZE];
    double matrix[16][16];
    int data[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 1) % 100;
        arr2[i] = (i * 5 + 2) % 100;
        farr[i] = (float)(i * 0.7);
        data[i] = (i * 11) % 256;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (double)(i * 16 + j) * 0.1;
        }
    }
    
    /* Call all functions to ensure they're compiled */
    int result1 = func1_carried_dep(arr1, SIZE, 3);
    float result2 = func2_pointer_chase(farr, SIZE, 1.25f);
    func3_multi_dep(arr2, (int*)arr1, SIZE, 5);
    double result4 = func4_nested_loops((double*)matrix, 16, 16);
    long result5 = func5_complex_reduction(data, SIZE);
    
    /* Use results to prevent dead code elimination */
    sink = result1 + (int)result2 + (int)result4 + (int)result5;
    
    /* Print minimal output */
    printf("Result: %d\n", sink);
    
    return 0;
}
