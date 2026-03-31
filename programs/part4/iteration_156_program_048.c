/* test_modulo_sched.c - Test program for GCC modulo scheduler coverage */

#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITER 100

/* Global volatile sink to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency distance 1 */
    for (int i = 1; i < n; i++) {
        /* Mix of integer operations with different latencies */
        int temp = varr[i-1] * scalar;      /* Multiplication */
        temp = temp + (temp >> 3);          /* Shift and add */
        temp = temp ^ (temp & 0xFF);        /* Bitwise operations */
        varr[i] = temp + varr[i];           /* Dependency chain */
        
        /* Fake dependency barrier */
        asm volatile("" : : "r"(temp) : "memory");
    }
}

/* Function 2: Reduction loop with floating-point operations */
float func2_fp_reduction(float *arr, int n) {
    volatile float sum = 0.0f;
    volatile float *varr = (volatile float *)arr;
    
    /* Loop with floating-point operations */
    for (int i = 0; i < n; i++) {
        /* Mix of FP operations */
        float val = varr[i];
        val = val * 1.5f;                   /* FP multiplication */
        val = val + 0.25f;                  /* FP addition */
        sum = sum + val;                    /* Reduction dependency */
        
        /* Integer operation in FP loop */
        int idx = i & 0xF;
        val = val * (idx + 1);
        
        /* Compiler barrier */
        asm volatile("" : : "r"(val) : "memory");
    }
    return sum;
}

/* Function 3: Pointer chasing pattern */
void func3_pointer_chase(int **ptr_arr, int n) {
    volatile int **vptr = (volatile int **)ptr_arr;
    
    if (n > 1) {
        int *current = vptr[0];
        for (int i = 1; i < n; i++) {
            /* Pointer chasing with arithmetic */
            int offset = (*current) & 0x7;
            current = vptr[offset];
            *current = *current + i;        /* Store with dependency */
            
            /* Memory barrier */
            asm volatile("" : : "r"(current) : "memory");
        }
    }
}

/* Function 4: Multiple independent statements with final dependent store */
void func4_multi_ops(int *arr1, int *arr2, int n, int k) {
    volatile int *varr1 = (volatile int *)arr1;
    volatile int *varr2 = (volatile int *)arr2;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int a = varr1[i] * k;
        int b = varr2[i] + (i << 2);
        int c = a ^ b;
        int d = (c * 3) >> 1;
        
        /* Final dependent store with distance 2 */
        if (i >= 2) {
            varr1[i] = d + varr1[i-2];      /* Distance 2 dependency */
        }
        
        /* Barrier to prevent reordering */
        asm volatile("" : : "r"(d) : "memory");
    }
}

/* Function 5: Nested loops with conditional inner logic */
void func5_nested_loops(int *matrix, int rows, int cols, int threshold) {
    volatile int *vmat = (volatile int *)matrix;
    
    /* Outer loop */
    for (int r = 0; r < rows; r++) {
        int row_start = r * cols;
        
        /* Inner loop with conditional */
        if (r % 2 == 0) {
            for (int c = 1; c < cols; c++) {
                int idx = row_start + c;
                /* Carried dependency within row */
                int prev = vmat[idx - 1];
                int curr = vmat[idx];
                
                /* Mixed operations */
                int result = (prev * 3 + curr * 2) >> 1;
                if (result > threshold) {
                    result = result - threshold;
                }
                vmat[idx] = result;
                
                /* Dependency on previous iteration */
                asm volatile("" : : "r"(result) : "memory");
            }
        }
    }
}

/* Main function that exercises all loops */
int main() {
    /* Initialize data arrays */
    int int_arr[SIZE];
    float float_arr[SIZE];
    int *ptr_arr[SIZE/4];
    int matrix[SIZE/2][SIZE/2];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 3 + 7) & 0xFF;
        float_arr[i] = (float)(i * 0.1 + 0.5);
        if (i < SIZE/4) {
            ptr_arr[i] = &int_arr[i * 4];
        }
    }
    
    for (int i = 0; i < SIZE/2; i++) {
        for (int j = 0; j < SIZE/2; j++) {
            matrix[i][j] = (i * j + i + j) & 0x7F;
        }
    }
    
    /* Execute all functions multiple times */
    for (int iter = 0; iter < ITER; iter++) {
        int scalar = (iter % 7) + 2;
        
        func1_carried_dep(int_arr, SIZE, scalar);
        
        float fp_result = func2_fp_reduction(float_arr, SIZE);
        global_sink += (int)fp_result;
        
        func3_pointer_chase(ptr_arr, SIZE/4);
        
        func4_multi_ops(int_arr, &int_arr[SIZE/2], SIZE/2, scalar);
        
        func5_nested_loops((int *)matrix, SIZE/4, SIZE/4, 64);
        
        /* Use results to prevent elimination */
        int sum = 0;
        for (int i = 0; i < SIZE; i++) {
            sum += int_arr[i];
        }
        global_sink += sum;
    }
    
    /* Final output to ensure execution */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
