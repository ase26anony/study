/* Test program for GCC modulo scheduler edge printing coverage */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n) {
    int sum = arr[0];
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with carried dependency */
        int temp = arr[i-1] * 3 + 7;
        arr[i] = (temp >> 2) & 0xFF;
        sum += arr[i];
        
        /* Fake dependency barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 2: Pointer chasing with floating point */
float func2_pointer_chase(float* data, int n) {
    float* p = data;
    float acc = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Floating point operations with pointer chasing */
        float val = *p;
        acc = acc * 1.5f + val * 2.0f;
        
        /* Conditional update to prevent simple analysis */
        if (acc > 100.0f) {
            acc = acc * 0.5f;
        }
        
        /* Update pointer with wrap-around */
        p = data + ((i * 7) % n);
        
        /* Compiler barrier */
        asm volatile("" : "+r"(acc) : : "memory");
    }
    return acc;
}

/* Function 3: Multiple independent statements with final dependent store */
void func3_multi_ops(int* a, int* b, int* c, int n) {
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int x = a[i] * 2;
        int y = b[i] + 5;
        int z = c[i] & 0xFF;
        
        /* Mixed operations creating dependencies */
        x = (x * y) >> 1;
        y = (y + z) * 3;
        z = x ^ y;
        
        /* Carried dependency through array */
        a[i] = a[i-1] + x + y + z;
        
        /* Cross-iteration dependency */
        b[i] = b[i-1] * 2 - a[i];
        
        /* Memory barrier */
        asm volatile("" : : "r"(a[i]), "r"(b[i]) : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested_loops(int* mat, int rows, int cols) {
    int total = 0;
    
    /* Outer loop */
    for (int r = 0; r < rows; r++) {
        int row_start = r * cols;
        
        /* Inner loop with carried dependency */
        for (int c = 1; c < cols; c++) {
            int idx = row_start + c;
            int prev_idx = row_start + c - 1;
            
            /* Complex dependency chain */
            int val = mat[prev_idx] * 3;
            val = (val + mat[idx]) / 2;
            
            /* Conditional that prevents simple analysis */
            if (val > 1000) {
                val = val % 1000;
            }
            
            mat[idx] = val;
            total += val;
        }
        
        /* Barrier between outer loop iterations */
        asm volatile("" : "+r"(total) : : "memory");
    }
    return total;
}

/* Function 5: Reduction with mixed data types */
long long func5_mixed_reduction(short* shorts, int* ints, int n) {
    long long acc = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed-width operations */
        short s = shorts[i];
        int x = ints[i];
        
        /* Type conversions and operations */
        long long temp = (long long)s * (long long)x;
        temp += (long long)shorts[i-1] * (long long)ints[i-1];
        
        /* Conditional update */
        if (temp < 0) {
            temp = -temp;
        }
        
        acc += temp;
        
        /* Update arrays with carried dependency */
        shorts[i] = (short)(acc & 0xFFFF);
        ints[i] = (int)(acc >> 16);
        
        /* Memory barrier */
        asm volatile("" : "+r"(acc) : : "memory");
    }
    return acc;
}

/* Main driver */
int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE], arr4[SIZE], arr5[SIZE];
    short shorts[SIZE];
    int ints[SIZE];
    int matrix[SIZE/2][SIZE/2];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3) % 100 + 1;
        arr2[i] = (float)(i * 0.5f);
        arr3[i] = i * 2;
        arr4[i] = i * 3;
        arr5[i] = i * 5;
        shorts[i] = (short)(i * 7);
        ints[i] = i * 11;
    }
    
    for (int i = 0; i < SIZE/2; i++) {
        for (int j = 0; j < SIZE/2; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Call all functions to ensure they're compiled and executed */
    int result1 = func1_carried_dep(arr1, ITERS);
    float result2 = func2_pointer_chase(arr2, ITERS);
    func3_multi_ops(arr3, arr4, arr5, ITERS);
    int result4 = func4_nested_loops((int*)matrix, SIZE/4, SIZE/4);
    long long result5 = func5_mixed_reduction(shorts, ints, ITERS);
    
    /* Use results to prevent dead code elimination */
    sink = result1 + (int)result2 + result4 + (int)result5;
    
    /* Also use array elements */
    for (int i = 0; i < 10; i++) {
        sink += arr1[i] + arr3[i] + arr4[i];
    }
    
    printf("Result: %d\n", sink);
    return 0;
}
