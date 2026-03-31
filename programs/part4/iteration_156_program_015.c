/* Test program to trigger modulo scheduling edge debugging output */
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
        temp = temp + arr[i];              /* Integer add */
        arr[i] = temp ^ 0x55AA55AA;        /* Bitwise operation */
        result += arr[i];                  /* Reduction */
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : "+r"(result) : : "memory");
    }
    return result;
}

/* Function 2: Pointer chasing pattern with floating point */
float func2_pointer_chase(float* data, int n, float coeff) {
    float* p = data;
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Pointer chasing with floating point operations */
        float val = *p * coeff;            /* FP multiply */
        val = val + 1.0f;                  /* FP add */
        sum += val;                        /* FP reduction */
        
        /* Create artificial dependency */
        if (i % 2 == 0) {
            p = data + (i % 16);
        }
        
        /* Compiler barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 3: Multiple independent statements with dependent store */
void func3_multi_dep(int* out, const int* in, int n, int k) {
    /* Outer loop to encourage modulo scheduling */
    for (int iter = 0; iter < k; iter++) {
        /* Inner loop with complex dependency pattern */
        for (int i = 2; i < n; i++) {
            /* Independent computations */
            int a = in[i] * 3;
            int b = in[i-1] + 7;
            int c = in[i-2] ^ 0xFF;
            
            /* Dependent computation with distance-2 */
            int d = (a + b) * c;
            
            /* Store with carried dependency */
            out[i] = out[i-2] + d;
            
            /* Prevent dead code elimination */
            asm volatile("" : : "r"(out[i]) : "memory");
        }
        
        /* Rotate input to create variation */
        asm volatile("" : : : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
long func4_nested_conditional(int* mat, int rows, int cols, int thresh) {
    long total = 0;
    
    /* Outer loop */
    for (int r = 0; r < rows; r++) {
        /* Inner loop with condition */
        if (r % 3 != 0) {  /* Prevent elimination */
            for (int c = 1; c < cols; c++) {
                /* Distance-1 dependency with condition */
                int prev = mat[r * cols + c - 1];
                int curr = mat[r * cols + c];
                
                if (prev > thresh) {
                    curr = curr * 2 + prev;
                } else {
                    curr = curr / 2 - prev;
                }
                
                mat[r * cols + c] = curr;
                total += curr;
                
                /* Memory barrier */
                asm volatile("" : "+r"(total) : : "memory");
            }
        }
    }
    return total;
}

/* Function 5: Reduction with mixed integer/FP operations */
double func5_mixed_reduction(const double* data, int n) {
    double sum_fp = 0.0;
    int sum_int = 0;
    
    for (int i = 1; i < n; i++) {
        /* Floating point operations */
        double fp_val = data[i] * 1.5;
        fp_val = fp_val + data[i-1];  /* Distance-1 dependency */
        sum_fp += fp_val;
        
        /* Integer operations in same loop */
        int int_val = (int)data[i] * 3;
        int_val = int_val ^ ((int)data[i-1]);  /* Another distance-1 dep */
        sum_int += int_val;
        
        /* Dependency between FP and int */
        if (sum_int % 1000 == 0) {
            sum_fp += 0.1;
        }
        
        /* Compiler barrier */
        asm volatile("" : "+r"(sum_int), "+r"(sum_fp) : : "memory");
    }
    
    return sum_fp + sum_int;
}

int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    int arr4[SIZE * 4];  /* Small matrix */
    double arr5[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) % 100;
        arr2[i] = (float)(i * 0.5);
        arr3[i] = i * 2;
        arr5[i] = (double)(i * 0.3);
    }
    
    for (int i = 0; i < SIZE * 4; i++) {
        arr4[i] = (i * 5) % 256;
    }
    
    /* Call all functions to ensure they're compiled and executed */
    int res1 = func1_carried_dep(arr1, SIZE, 7);
    float res2 = func2_pointer_chase(arr2, SIZE, 2.5f);
    
    func3_multi_dep(arr3, (int*)arr1, SIZE, 3);
    
    long res4 = func4_nested_conditional(arr4, 4, SIZE/4, 100);
    double res5 = func5_mixed_reduction(arr5, SIZE);
    
    /* Use results to prevent dead code elimination */
    sink = res1 + (int)res2 + arr3[SIZE-1] + (int)res4 + (int)res5;
    
    /* Print minimal output */
    printf("Result: %d\n", sink);
    
    return 0;
}
