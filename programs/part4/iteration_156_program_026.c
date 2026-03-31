/* Test program to trigger modulo scheduling edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n, int scalar) {
    int result = 0;
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mix integer and floating point operations */
        int temp = arr[i-1] * scalar;
        float ftemp = (float)temp * 1.5f;
        arr[i] = (int)ftemp + arr[i] + (i & 0xF); /* Add some bitwise ops */
        result += arr[i];
    }
    return result;
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2_reduction(volatile int* arr, int n) {
    int sum = 0;
    volatile int* p = arr;
    
    /* Loop with memory dependencies */
    for (int i = 0; i < n; i++) {
        sum = sum * 13 + *p;  /* Recurrence with multiplication */
        sum = (sum << 3) | (sum >> 29);  /* Rotation */
        p = &arr[(i * 7) % n];  /* Pointer chasing */
        
        /* Mix in floating point */
        double dsum = (double)sum * 0.97;
        sum = (int)dsum;
    }
    return sum;
}

/* Function 3: Multiple independent statements with dependent store */
int func3_multidep(volatile int* arr, int n, volatile float* farr) {
    int acc1 = arr[0];
    int acc2 = arr[1];
    float facc = farr[0];
    
    /* Complex loop with multiple dependencies */
    for (int i = 2; i < n; i++) {
        /* Independent computations */
        int t1 = acc1 * 3;
        int t2 = acc2 / 5;
        float ft = facc * 2.3f;
        
        /* Compiler barrier to preserve ordering */
        asm volatile("" : : "r"(t1), "r"(t2) : "memory");
        
        /* Dependent store with carried dependency */
        acc1 = t1 + arr[i] + (acc2 & 0xFF);
        acc2 = t2 ^ (i * 11);
        facc = ft - (float)i * 0.1f;
        
        /* Store result */
        arr[i] = acc1 + (int)facc;
    }
    return acc1 + acc2 + (int)facc;
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested(volatile int* arr, int n, int threshold) {
    int total = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Conditional inner loop */
        if (outer % 2 == 0) {
            /* Inner loop with carried dependency */
            int carry = arr[0];
            for (int i = 1; i < n; i++) {
                /* Mixed operations with dependency chain */
                carry = carry * 17 + arr[i];
                carry = carry - (carry / 4);
                
                /* Floating point in dependency chain */
                float fcarry = (float)carry * 0.8f;
                carry = (int)fcarry | (i << 16);
                
                arr[i] = carry;
                total += carry;
            }
        } else {
            /* Different pattern for odd iterations */
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] ^ (i * 31);
                total ^= arr[i];
            }
        }
    }
    return total;
}

/* Wrapper function to create scheduling context */
void process_loops(int mode) {
    volatile int arr1[SIZE];
    volatile int arr2[SIZE];
    volatile float farr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 1) & 0xFF;
        arr2[i] = (i * 5 + 2) & 0xFF;
        farr[i] = (float)i * 0.5f;
    }
    
    int result = 0;
    
    /* Execute different loops based on mode */
    if (mode & 1) {
        result += func1_carried_dep(arr1, ITERS, 7);
    }
    if (mode & 2) {
        result += func2_reduction(arr2, ITERS);
    }
    if (mode & 4) {
        result += func3_multidep(arr1, ITERS, farr);
    }
    if (mode & 8) {
        result += func4_nested(arr2, ITERS, 50);
    }
    
    /* Use result to prevent elimination */
    sink = result;
}

int main() {
    /* Initialize random seed for variability */
    srand(42);
    
    /* Call loops multiple times with different patterns */
    for (int phase = 0; phase < 3; phase++) {
        int mode = (phase * 7) % 15;
        if (mode == 0) mode = 1;  /* Ensure at least one loop runs */
        
        process_loops(mode);
        
        /* Small computation between phases */
        volatile int temp = sink;
        for (int i = 0; i < 10; i++) {
            temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        sink = temp;
    }
    
    /* Final output to ensure execution */
    printf("Result: %d\n", sink);
    
    return 0;
}
