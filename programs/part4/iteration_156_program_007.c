/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERATIONS 100

/* Volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1(volatile int* arr, int n) {
    int result = 0;
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix integer and floating point operations */
        float temp = (float)arr[i-1] * 1.5f + (float)arr[i];
        arr[i] = (int)temp + (arr[i-1] & 0xFF);
        result += arr[i];
        
        /* Memory barrier to preserve operations */
        asm volatile("" : : "r"(arr[i]) : "memory");
    }
    return result;
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2(volatile int* arr, int n) {
    int sum = 0;
    volatile int* p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pointer chasing with dependency */
        int val = *p;
        p = &arr[(val + i) % n];
        
        /* Mixed operations */
        sum += val * 3 - (val >> 2);
        sum = (sum * 7) ^ 0x55AA55AA;
        
        /* Floating point operation */
        float fval = (float)val * 0.25f;
        sum += (int)fval;
    }
    return sum;
}

/* Function 3: Loop with multiple independent statements and dependent store */
int func3(volatile int* arr, volatile float* farr, int n) {
    int total = 0;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int a = arr[i] * 2;
        int b = arr[i-1] + 5;
        float c = farr[i] * 3.14f;
        float d = farr[i-1] / 2.0f;
        
        /* Dependent store with carried dependency */
        arr[i] = a + b + (int)(c + d);
        farr[i] = c - d + (float)(a % 7);
        
        total += arr[i];
        
        /* Compiler barrier */
        asm volatile("" : : "r"(arr[i]), "r"(farr[i]) : "memory");
    }
    return total;
}

/* Function 4: Nested loop with conditional inner logic */
int func4(volatile int* arr, int n, int threshold) {
    int outer_sum = 0;
    
    for (int j = 0; j < 5; j++) {
        int inner_sum = 0;
        
        /* Inner loop with conditional execution */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Complex carried dependency */
                int diff = arr[i] - arr[i-1];
                arr[i] = arr[i-1] + (diff * j) / (i + 1);
                
                /* Mixed integer/float operations */
                float fdiff = (float)diff * 0.1f * (float)j;
                inner_sum += (int)fdiff + (arr[i] & 0xF);
                
                /* Memory dependency */
                asm volatile("" : : "r"(arr[i]) : "memory");
            }
        }
        outer_sum += inner_sum;
    }
    return outer_sum;
}

/* Function 5: Loop with multiple recurrence distances */
int func5(volatile int* arr, int n) {
    int sum = 0;
    
    for (int i = 2; i < n; i++) {
        /* Distance 1 dependency */
        int x = arr[i-1] * 3;
        
        /* Distance 2 dependency */
        int y = arr[i-2] + x;
        
        /* Combined operation with floating point */
        float z = (float)y * 1.1f + (float)arr[i] * 0.9f;
        
        arr[i] = (int)z + (x & y);
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Initialize arrays with non-zero values */
    volatile int arr1[SIZE];
    volatile int arr2[SIZE];
    volatile float farr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 1) % 100;
        arr2[i] = (i * 7 + 3) % 100;
        farr[i] = (float)(i * 11) / 7.0f;
    }
    
    /* Call all functions to ensure they're compiled */
    int result = 0;
    
    result += func1(arr1, SIZE);
    sink = result;  /* Use volatile sink */
    
    result += func2(arr2, SIZE);
    sink = result;
    
    result += func3(arr1, farr, SIZE);
    sink = result;
    
    result += func4(arr2, SIZE, 10);
    sink = result;
    
    result += func5(arr1, SIZE);
    sink = result;
    
    /* Final trivial use of result */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
