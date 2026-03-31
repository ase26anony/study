/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n) {
    int sum = arr[0];
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with carried dependency */
        int temp = arr[i-1] * 3 + 7;
        arr[i] = (temp >> 2) + arr[i];
        sum += arr[i];
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 2: Loop with floating-point operations and recurrence */
float func2_fp_recurrence(volatile float* arr, int n) {
    float acc = arr[0];
    for (int i = 1; i < n; i++) {
        /* Floating-point operations with different latencies */
        float fp_temp = acc * 1.5f + arr[i];
        acc = fp_temp - (float)i * 0.1f;
        
        /* Integer operations mixed in */
        int idx = i & 0xFF;
        acc += (float)idx * 0.01f;
        
        /* Memory barrier */
        asm volatile("" : "+f"(acc) : : "memory");
    }
    return acc;
}

/* Function 3: Pointer chasing pattern with conditional */
int func3_pointer_chase(volatile int* arr, int n) {
    volatile int* ptr = arr;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pointer chasing with dependency */
        int val = *ptr;
        ptr = &arr[val & (n-1)];
        
        /* Conditional operation */
        if (val > 100) {
            total += val * 2;
        } else {
            total += val / 2;
        }
        
        /* Mix of operations */
        total = (total << 1) | (total >> 31);
        
        /* Compiler barrier */
        asm volatile("" : "+r"(total) : "r"(ptr) : "memory");
    }
    return total;
}

/* Function 4: Nested loops with outer loop */
void func4_nested_loops(volatile int* arr, int n) {
    for (int outer = 0; outer < 5; outer++) {
        int base = outer * 10;
        for (int i = 0; i < n; i++) {
            /* Multiple independent statements */
            int a = arr[i] + base;
            int b = arr[(i+1) % n] * 2;
            int c = a ^ b;
            
            /* Dependent store with distance 1 */
            arr[(i+2) % n] = c + arr[i];
            
            /* Memory operation */
            asm volatile("" : : "r"(c) : "memory");
        }
    }
}

/* Function 5: Reduction with multiple accumulators */
long func5_multiple_reductions(volatile int* arr, int n) {
    long sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple accumulators with different operations */
        sum1 += arr[i] * 3L;
        sum2 += (long)arr[i] << 2;
        sum3 += sum1 ^ sum2;
        
        /* Rotate values to create complex dependency */
        long temp = sum1;
        sum1 = sum2;
        sum2 = sum3;
        sum3 = temp;
        
        /* Barrier */
        asm volatile("" : "+r"(sum1), "+r"(sum2), "+r"(sum3) : : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Main function that exercises all loops */
int main(int argc, char** argv) {
    /* Initialize data with non-zero values */
    volatile int int_arr[SIZE];
    volatile float float_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 3 + 7) & 0xFF;
        float_arr[i] = (float)i * 0.5f + 1.0f;
    }
    
    /* Execute all functions to ensure they're compiled */
    int result1 = func1_carried_dep(int_arr, SIZE);
    float result2 = func2_fp_recurrence(float_arr, SIZE);
    int result3 = func3_pointer_chase(int_arr, SIZE / 2);
    func4_nested_loops(int_arr, SIZE);
    long result5 = func5_multiple_reductions(int_arr, SIZE);
    
    /* Use results to prevent elimination */
    sink = result1 + (int)result2 + result3 + (int)result5;
    
    /* Print minimal output to show program ran */
    if (argc > 1) {
        printf("Results: %d %f %d %ld\n", result1, result2, result3, result5);
    }
    
    return 0;
}
