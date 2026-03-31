/* Test program for GCC modulo scheduler edge printing coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITER 100

/* Global volatile to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int temp = arr[0]; /* Force dependency */
    
    /* Loop with carried dependency distance 1 */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int val = arr[i-1] * scalar + arr[i];
        arr[i] = (val >> 3) & 0xFF; /* Bitwise ops */
        
        /* Floating point operations (if supported) */
        float fval = (float)val * 1.5f;
        arr[i] += (int)fval;
        
        /* Memory barrier to preserve ordering */
        asm volatile("" : : "r"(arr[i]) : "memory");
    }
}

/* Function 2: Reduction loop with floating point */
float func2_reduction(float *arr, int n) {
    volatile float sum = 0.0f;
    
    /* Reduction with dependency */
    for (int i = 0; i < n; i++) {
        sum = sum + arr[i] * 2.5f;
        
        /* Integer operations in parallel */
        int idx = i & 0xF;
        arr[i] = sum + (float)idx;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(sum), "r"(arr[i]) : "memory");
    }
    return sum;
}

/* Function 3: Pointer chasing pattern */
void func3_pointer_chase(int **ptr_arr, int n) {
    volatile int *current = ptr_arr[0];
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with arithmetic */
        int val = *current + i;
        *current = val;
        
        /* Conditional update */
        if (val > 100) {
            current = ptr_arr[i % n];
        } else {
            current = &ptr_arr[i % n][0];
        }
        
        /* Mixed operations */
        float fcalc = (float)val * 0.75f;
        ptr_arr[i % n][0] += (int)fcalc;
        
        /* Memory barrier */
        asm volatile("" : : "r"(current), "r"(val) : "memory");
    }
}

/* Function 4: Multiple independent statements with final dependency */
void func4_multi_ops(int *a, int *b, int *c, int n) {
    volatile int acc = 0;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int t1 = a[i] * 3;
        int t2 = b[i] + 7;
        float t3 = (float)a[i-1] * 1.8f;
        
        /* Dependent operation */
        c[i] = t1 + t2 + (int)t3 + acc;
        
        /* Update accumulator with dependency */
        acc = c[i] >> 1;
        
        /* Barrier */
        asm volatile("" : : "r"(t1), "r"(t2), "r"(acc) : "memory");
    }
}

/* Function 5: Nested loops with conditional inner logic */
void func5_nested(int *arr, int n, int outer_iters) {
    volatile int cond = outer_iters > 5;
    
    for (int k = 0; k < outer_iters; k++) {
        /* Conditional inner loop */
        if (cond || (k % 3 == 0)) {
            /* Inner loop with carried dependency */
            for (int i = 2; i < n; i++) {
                /* Complex dependency chain */
                int val1 = arr[i-1] * arr[i-2];
                float val2 = (float)val1 * 0.33f;
                arr[i] = (int)val2 + k;
                
                /* Additional integer ops */
                arr[i] = (arr[i] << 2) | (arr[i] >> 30); /* Rotation */
                
                /* Barrier */
                asm volatile("" : : "r"(arr[i]), "r"(val1) : "memory");
            }
        }
        
        /* Modify condition to prevent dead code elimination */
        cond = cond ^ (k & 1);
    }
}

/* Main function that exercises all patterns */
int main() {
    /* Initialize arrays with non-zero values */
    int arr1[SIZE];
    float arr2[SIZE];
    int *ptr_arr[SIZE];
    int arr3[SIZE], arr4[SIZE], arr5[SIZE];
    int arr6[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) & 0xFF;
        arr2[i] = (float)(i * 2 + 1) * 0.5f;
        arr3[i] = i * 5;
        arr4[i] = i * 7;
        arr5[i] = 0;
        arr6[i] = i * 11;
        
        /* Allocate for pointer chasing */
        ptr_arr[i] = (int*)malloc(sizeof(int) * 4);
        for (int j = 0; j < 4; j++) {
            ptr_arr[i][j] = (i * 4 + j) * 3;
        }
    }
    
    /* Execute all functions multiple times */
    for (int iter = 0; iter < ITER; iter++) {
        int scalar = iter + 2;
        
        /* Call function 1 */
        func1_carried_dep(arr1, SIZE - iter % 32, scalar);
        
        /* Call function 2 */
        float sum = func2_reduction(arr2, SIZE - iter % 16);
        sink += (int)sum;
        
        /* Call function 3 */
        func3_pointer_chase(ptr_arr, 64);
        
        /* Call function 4 */
        func4_multi_ops(arr3, arr4, arr5, SIZE - iter % 8);
        
        /* Call function 5 */
        func5_nested(arr6, 128, 10 + iter % 5);
        
        /* Modify inputs slightly each iteration */
        arr1[0] += iter;
        arr2[0] += (float)iter;
    }
    
    /* Compute checksum to prevent elimination */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + (int)arr2[i] + arr5[i] + arr6[i];
        if (i < 64) {
            checksum += ptr_arr[i][0];
        }
    }
    
    sink = checksum;
    
    /* Cleanup */
    for (int i = 0; i < SIZE; i++) {
        free(ptr_arr[i]);
    }
    
    printf("Result: %d\n", sink);
    return 0;
}
