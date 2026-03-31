/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERATIONS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        varr[i] = varr[i-1] * scalar + varr[i];
        /* Mix in some bitwise operations */
        varr[i] ^= (varr[i] >> 3);
    }
    
    /* Create artificial dependency to prevent dead code elimination */
    asm volatile("" : : "r"(varr[n-1]) : "memory");
}

/* Function 2: Reduction loop with floating point operations */
float func2_reduction(float *farr, int n) {
    volatile float *vfarr = (volatile float *)farr;
    float sum = vfarr[0];
    
    /* Reduction with carried dependency */
    for (int i = 1; i < n; i++) {
        sum = sum * 1.01f + vfarr[i];
        /* Mix integer and float operations */
        int temp = (int)sum;
        sum += (temp & 0xFF) * 0.5f;
    }
    
    /* Compiler barrier */
    asm volatile("" : "+r"(sum) : : "memory");
    return sum;
}

/* Function 3: Pointer chasing pattern */
void func3_pointer_chase(int *arr, int n) {
    if (n < 2) return;
    
    int *p = arr;
    volatile int *vp = (volatile int *)p;
    
    /* Pointer chasing creates unpredictable dependencies */
    for (int i = 0; i < n-1; i++) {
        vp[i+1] = vp[i] + (vp[i] * 3);
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(vp[i]) : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
void func4_nested_loops(int *arr, int n, int threshold) {
    volatile int *varr = (volatile int *)arr;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with carried dependency */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Complex expression with multiple dependencies */
                varr[i] = (varr[i-1] * 7 + varr[i] * 3) / 5;
                /* Conditional operation */
                if (varr[i] & 1) {
                    varr[i] ^= 0xAAAA;
                }
            }
        }
        /* Small compiler barrier */
        asm volatile("" : : "r"(varr[n-1]) : "memory");
    }
}

/* Function 5: Multiple independent statements with final dependency */
void func5_multi_dep(int *arr, int n) {
    volatile int *varr = (volatile int *)arr;
    int temp1, temp2, temp3;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        temp1 = varr[i] * 2;
        temp2 = varr[i] + 100;
        temp3 = varr[i] ^ 0x55;
        
        /* Final dependent store with distance 1 */
        varr[i] = varr[i-1] + temp1 + temp2 + temp3;
    }
}

/* Main function that exercises all patterns */
int main() {
    /* Initialize arrays with non-zero values */
    int int_arr[SIZE];
    float float_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 3 + 7) & 0xFF;
        float_arr[i] = (float)(i * 2 + 1) * 0.5f;
    }
    
    /* Call each function multiple times with different parameters */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        int scalar = (iter % 7) + 2;
        
        func1_carried_dep(int_arr, SIZE - iter % 32, scalar);
        
        float result = func2_reduction(float_arr, SIZE - iter % 16);
        sink += (int)result;
        
        func3_pointer_chase(int_arr, SIZE - iter % 64);
        
        func4_nested_loops(int_arr, SIZE - iter % 48, iter % 3);
        
        func5_multi_dep(int_arr, SIZE - iter % 24);
    }
    
    /* Compute checksum to use results */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= int_arr[i];
        checksum += (int)float_arr[i];
    }
    
    /* Volatile sink to prevent dead code elimination */
    sink = checksum;
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
