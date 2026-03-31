/* Test program for GCC modulo scheduler edge printing coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(volatile int* arr, int n, int scalar) {
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = arr[i-1] * scalar;
        temp = temp + (temp >> 3);      /* Bitwise operation */
        temp = temp ^ (temp << 2);      /* Another bitwise op */
        arr[i] = arr[i] + temp;         /* Store with dependency */
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : : "r"(arr[i]) : "memory");
    }
}

/* Function 2: Reduction loop with floating point operations */
float func2_reduction(float* farr, int n) {
    float sum = farr[0];
    volatile float vsum = sum;  /* Volatile to preserve operations */
    
    for (int i = 1; i < n; i++) {
        /* Floating point operations with different latencies */
        float prod = farr[i] * 1.5f;
        sum = sum + prod;               /* Carried dependency */
        
        /* Mix in some integer operations */
        int idx = (int)prod % 16;
        sum += (float)idx * 0.1f;
        
        /* Compiler barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum + vsum;
}

/* Function 3: Pointer chasing pattern */
void func3_pointer_chase(int** ptr_arr, int n) {
    int* current = ptr_arr[0];
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with arithmetic */
        int val = *current;
        val = val * 3 + i;
        *current = val;
        
        /* Update pointer with dependency */
        current = ptr_arr[(val + i) % n];
        
        /* Prevent optimization */
        asm volatile("" : : "r"(current) : "memory");
    }
}

/* Function 4: Multiple independent statements with final dependency */
void func4_multi_dep(int* arr1, int* arr2, int n, int k) {
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int a = arr1[i] * k;
        int b = arr2[i] + i;
        int c = a ^ b;
        int d = (a + b) << 2;
        
        /* Final store with complex dependency */
        arr1[i-1] = (a + c) * (b - d) + arr1[i-1];
        
        /* Memory barrier */
        asm volatile("" : : "r"(arr1[i-1]) : "memory");
    }
}

/* Function 5: Nested loops with conditional inner logic */
void func5_nested_conditional(int* data, int rows, int cols, int threshold) {
    volatile int cond = threshold;
    
    for (int r = 0; r < rows; r++) {
        /* Outer loop */
        int base = r * cols;
        
        /* Inner loop with conditional execution */
        if (cond > 0) {
            for (int c = 1; c < cols; c++) {
                /* Loop with carried dependency */
                int prev = data[base + c - 1];
                int curr = data[base + c];
                
                /* Mixed operations */
                int diff = prev - curr;
                int scaled = diff * 7;
                int result = (scaled >> 1) + (curr * 3);
                
                data[base + c] = result + (r * c);
                
                /* Dependency on outer loop variable */
                asm volatile("" : : "r"(result), "r"(r) : "memory");
            }
        }
    }
}

/* Main driver that ensures all loops execute */
int main() {
    /* Initialize arrays with non-zero values */
    volatile int arr_int[SIZE];
    float arr_float[SIZE];
    int* ptr_arr[SIZE];
    int arr1[SIZE], arr2[SIZE];
    int nested_data[SIZE * 4];
    
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = (i * 3 + 7) & 0xFF;
        arr_float[i] = (float)(i * 0.7);
        arr1[i] = i * 2;
        arr2[i] = i * 3 + 1;
        ptr_arr[i] = &arr1[i % 128];
    }
    
    for (int i = 0; i < SIZE * 4; i++) {
        nested_data[i] = i % 100;
    }
    
    /* Execute all functions multiple times to ensure coverage */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Call function 1 with carried dependency */
        func1_carried_dep(arr_int, SIZE, iter + 2);
        
        /* Call function 2 with floating point reduction */
        float fresult = func2_reduction(arr_float, SIZE);
        sink += (int)fresult;
        
        /* Call function 3 with pointer chasing */
        func3_pointer_chase(ptr_arr, SIZE / 2);
        
        /* Call function 4 with multiple dependencies */
        func4_multi_dep(arr1, arr2, SIZE, iter + 1);
        
        /* Call function 5 with nested conditional loops */
        func5_nested_conditional(nested_data, 4, SIZE, iter);
        
        /* Modify inputs slightly each iteration */
        arr_int[0] += iter;
        arr_float[0] += (float)iter * 0.1f;
    }
    
    /* Compute a checksum to use results */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr_int[i] + arr1[i] + arr2[i] + (int)arr_float[i];
    }
    checksum += nested_data[SIZE];
    
    /* Use checksum to prevent dead code elimination */
    sink = checksum;
    printf("Result checksum: %d\n", checksum % 1000);
    
    return 0;
}
