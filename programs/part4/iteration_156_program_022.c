/* test_modulo_sched.c - Test program for GCC modulo scheduler coverage */

#include <stdlib.h>
#include <stdio.h>

#define SIZE 128
#define ITERS 100

/* Global volatile sink to prevent dead code elimination */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void loop1_carried_dependency(volatile int* arr, int n, int scalar) {
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = arr[i-1] * scalar;
        temp = temp + (temp >> 3);        /* Bitwise operation */
        arr[i] = temp + arr[i] * 7;       /* Dependency on previous iteration */
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : : "r"(arr[i]) : "memory");
    }
}

/* Function 2: Reduction loop with floating point operations */
float loop2_reduction_mixed(const float* data, int n) {
    volatile float acc = 0.0f;
    float temp = 1.0f;
    
    /* Loop with floating-point and integer mix */
    for (int i = 0; i < n; i++) {
        /* Floating point operations (higher latency) */
        float fval = data[i] * 1.5f;
        acc = acc + fval;
        
        /* Integer operations interleaved */
        int ival = (int)fval;
        temp = temp * (float)(ival & 0xFF);  /* Bitwise and cast */
        
        /* Memory barrier */
        asm volatile("" : : "r"(acc), "r"(temp) : "memory");
    }
    
    return acc + temp;
}

/* Function 3: Pointer chasing with conditional inner logic */
int loop3_pointer_chasing(int* base, int n) {
    volatile int* current = base;
    int sum = 0;
    
    /* Outer loop to encourage modulo scheduling */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with pointer chasing pattern */
        for (int i = 0; i < n; i++) {
            /* Conditional inside loop */
            if (current != NULL) {
                sum += *current;
                /* Pointer chase with dependency */
                current = base + (*current % (n-1));
                
                /* Mixed operations */
                sum = (sum * 3) ^ 0xAAAA;  /* XOR operation */
            }
            
            /* Prevent optimization */
            asm volatile("" : : "r"(sum) : "memory");
        }
        
        /* Reset for next outer iteration */
        current = base + (outer % 3);
    }
    
    return sum;
}

/* Function 4: Multiple independent statements with final dependent store */
void loop4_multiple_stmts(int* dst, const int* src1, const int* src2, int n) {
    /* Loop with parallel work then convergence */
    for (int i = 0; i < n; i++) {
        /* Independent computations */
        int a = src1[i] * 11;
        int b = src2[i] * 13;
        int c = a ^ b;                     /* Bitwise XOR */
        int d = (a + b) >> 2;              /* Shift operation */
        
        /* Dependent store with carried dependency */
        if (i > 0) {
            dst[i] = dst[i-1] + c * d;     /* Distance-1 dependency */
        } else {
            dst[i] = c * d;
        }
        
        /* Memory barrier */
        asm volatile("" : : "r"(dst[i]) : "memory");
    }
}

/* Function 5: Nested loops with complex index calculations */
void loop5_nested_complex(float* matrix, int rows, int cols) {
    volatile float acc = 0.0f;
    
    /* Outer loop */
    for (int i = 1; i < rows; i++) {
        /* Inner loop with dependency */
        for (int j = 1; j < cols; j++) {
            /* Complex addressing with mixed operations */
            float val = matrix[i*cols + j-1] * 2.5f;  /* Distance-1 in inner loop */
            val = val + matrix[(i-1)*cols + j] * 1.5f; /* Distance-cols in outer loop */
            
            /* Integer work mixed in */
            int ival = (int)val;
            val = val * (float)(ival % 256);
            
            matrix[i*cols + j] = val;
            
            /* Accumulate to volatile to prevent removal */
            acc += val;
            
            /* Compiler barrier */
            asm volatile("" : : "r"(acc) : "memory");
        }
    }
}

/* Main function that exercises all loops */
int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    float farr[SIZE];
    int chase_arr[SIZE];
    int src1[SIZE], src2[SIZE], dst[SIZE];
    float matrix[10][10];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3) % 97 + 1;
        arr2[i] = (i * 5) % 89 + 1;
        farr[i] = (float)((i * 7) % 101) * 0.1f;
        chase_arr[i] = (i + 1) % (SIZE - 1);
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (float)(i * 10 + j) * 0.5f;
        }
    }
    
    /* Execute all loop functions multiple times */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify scalar to create varying patterns */
        int scalar = (iter % 7) + 2;
        
        /* Function 1: Basic carried dependency */
        loop1_carried_dependency(arr1, SIZE, scalar);
        
        /* Function 2: Mixed float/int reduction */
        float result2 = loop2_reduction_mixed(farr, SIZE);
        global_sink += (int)result2;
        
        /* Function 3: Pointer chasing */
        int result3 = loop3_pointer_chasing(chase_arr, SIZE / 4);
        global_sink += result3;
        
        /* Function 4: Multiple statements */
        loop4_multiple_stmts(dst, src1, src2, SIZE);
        for (int i = 0; i < SIZE; i++) {
            global_sink += dst[i];
        }
        
        /* Function 5: Nested complex */
        loop5_nested_complex((float*)matrix, 10, 10);
        global_sink += (int)matrix[5][5];
        
        /* Rotate data to create varying patterns */
        int temp = arr1[0];
        for (int i = 0; i < SIZE - 1; i++) {
            arr1[i] = arr1[i + 1];
        }
        arr1[SIZE - 1] = temp;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
