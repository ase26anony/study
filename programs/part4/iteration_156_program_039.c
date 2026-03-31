/* Test program to trigger modulo scheduling edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n) {
    int result = 0;
    /* Loop with carried dependency distance 1 */
    for (int i = 1; i < n; i++) {
        /* Mixed operations with different latencies */
        int temp = arr[i-1] * 3;      /* integer multiply */
        temp = temp + (arr[i] >> 2);  /* shift + add */
        arr[i] = temp ^ 0x5555;       /* bitwise XOR */
        result += arr[i];             /* accumulation */
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : "+r"(result) : : "memory");
    }
    return result;
}

/* Function 2: Pointer chasing with floating point */
float func2_pointer_chase(float* data, int n) {
    float* ptr = data;
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Pointer chasing creates dependency chain */
        float val = *ptr;
        val = val * 1.5f;           /* FP multiply */
        val = val + 2.0f;           /* FP add */
        *ptr = val;
        sum += val;
        
        /* Move pointer with dependency */
        ptr = data + ((int)val) % (n-1);
        
        /* Compiler barrier */
        asm volatile("" : "+r"(ptr) : : "memory");
    }
    return sum;
}

/* Function 3: Multiple independent statements with final dependent store */
void func3_multi_dep(int* a, int* b, int* c, int n) {
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int t1 = a[i] * b[i];
        int t2 = a[i-1] + c[i];
        int t3 = b[i] ^ c[i-1];
        
        /* Dependent store with distance 1 */
        a[i] = (t1 + t2) * t3;
        
        /* Memory barrier */
        asm volatile("" : : "r"(t1), "r"(t2), "r"(t3) : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested(int* mat, int rows, int cols) {
    int total = 0;
    volatile int cond = 1;  /* Prevent dead code elimination */
    
    for (int r = 0; r < rows; r++) {
        if (cond) {
            /* Inner loop with carried dependency */
            for (int c = 1; c < cols; c++) {
                int idx = r * cols + c;
                int prev_idx = r * cols + (c-1);
                
                /* Complex dependency chain */
                mat[idx] = mat[prev_idx] * 7;
                mat[idx] += (mat[idx] >> 3) & 0xFF;
                total ^= mat[idx];
            }
        }
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(total) : : "memory");
    }
    return total;
}

/* Function 5: Reduction with mixed integer/float */
double func5_reduction(double* arr, int n) {
    double sum = 0.0;
    double prod = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Floating point operations */
        double val = arr[i] * 0.75;
        val = val + (double)(i % 8);
        
        /* Integer operations in parallel */
        int ival = (int)val;
        ival = (ival * 13) % 97;
        
        /* Dependent accumulation */
        sum += val * (double)ival;
        prod *= val + 1.0;
        
        /* Dependency between iterations */
        arr[i] = sum / (prod + 1.0);
    }
    return sum + prod;
}

/* Main function that exercises all loops */
int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE], arr4[SIZE], arr5[SIZE];
    double arr6[SIZE];
    int matrix[SIZE/4][4];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) % 97;
        arr2[i] = (float)(i * 0.5);
        arr3[i] = i * 2;
        arr4[i] = i ^ 0xAA;
        arr5[i] = i + 1;
        arr6[i] = (double)i * 1.25;
    }
    
    /* Call all functions to ensure they're compiled */
    int r1 = func1_carried_dep((int*)arr1, ITERS);
    float r2 = func2_pointer_chase(arr2, ITERS);
    func3_multi_dep(arr3, arr4, arr5, ITERS);
    int r4 = func4_nested((int*)matrix, SIZE/4, 4);
    double r5 = func5_reduction(arr6, ITERS);
    
    /* Use results to prevent dead code elimination */
    sink = r1 + (int)r2 + r4 + (int)r5;
    
    /* Print minimal output */
    printf("Result: %d\n", sink);
    
    return 0;
}
