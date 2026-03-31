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
        arr[i] = (temp >> 2) | (arr[i] & 0xFF);
        sum += arr[i];
        
        /* Fake dependency barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 2: Loop with floating-point operations and recurrence */
float func2_float_recurrence(float* arr, int n, float scalar) {
    float acc = arr[0];
    for (int i = 1; i < n; i++) {
        /* Floating-point operations with different latencies */
        float fp_temp = arr[i-1] * scalar + 1.5f;
        arr[i] = fp_temp * 0.5f + arr[i];
        acc += arr[i];
        
        /* Integer operations mixed in */
        int idx = i & 0xF;
        arr[idx] = arr[idx] + 1;
    }
    return acc;
}

/* Function 3: Nested loops with conditional inner logic */
int func3_nested_conditional(int* data, int n, int threshold) {
    int total = 0;
    volatile int counter = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        counter++;
        
        /* Inner loop with carried dependency */
        if (threshold > 0) {
            int carry = data[0];
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                carry = (carry * 13 + data[i]) ^ 0x55AA55AA;
                data[i] = carry & 0xFFFF;
                total += data[i];
                
                /* Conditional operation */
                if (i % 7 == 0) {
                    data[i] = data[i] >> 1;
                }
            }
            data[0] = carry;
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    return total;
}

/* Function 4: Reduction loop with pointer chasing pattern */
int func4_pointer_chase(int* base, int n) {
    int* p = base;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pointer chasing with arithmetic */
        int val = *p;
        val = val * 17 + i;
        *p = val;
        sum += val;
        
        /* Update pointer with wrap-around */
        p = base + ((val + i) % n);
        
        /* Barrier to prevent over-optimization */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 5: Loop with multiple independent statements and dependent store */
void func5_multi_dep(volatile int* out, const int* in, int n) {
    int acc1 = in[0];
    int acc2 = in[1];
    int acc3 = in[2];
    
    for (int i = 3; i < n; i++) {
        /* Multiple parallel computations */
        int t1 = acc1 * 3;
        int t2 = acc2 + 5;
        int t3 = acc3 ^ 0xFF;
        
        /* Dependent combination */
        int combined = (t1 + t2) * t3;
        
        /* Store with dependency on all accumulators */
        out[i] = combined + in[i];
        
        /* Update accumulators with carried dependencies */
        acc1 = acc2;
        acc2 = acc3;
        acc3 = combined & 0xFF;
    }
}

int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    int arr4[SIZE];
    volatile int arr5[SIZE];
    int in_data[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = (float)(i * 0.5);
        arr3[i] = i * 3;
        arr4[i] = i * 7;
        arr5[i] = 0;
        in_data[i] = i * 11;
    }
    
    /* Call all functions to ensure they're compiled */
    int result1 = func1_carried_dep(arr1, ITERS);
    float result2 = func2_float_recurrence(arr2, ITERS, 2.5f);
    int result3 = func3_nested_conditional(arr3, ITERS, 1);
    int result4 = func4_pointer_chase(arr4, ITERS);
    func5_multi_dep(arr5, in_data, ITERS);
    
    /* Use results to prevent dead code elimination */
    sink = result1 + (int)result2 + result3 + result4;
    for (int i = 0; i < ITERS; i++) {
        sink += arr5[i];
    }
    
    /* Print minimal output */
    printf("Result checksum: %d\n", sink);
    
    return 0;
}
