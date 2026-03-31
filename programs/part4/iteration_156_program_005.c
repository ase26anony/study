/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency (distance 1) */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int temp = arr[0]; /* Prevent dead store elimination */
    
    /* Loop with carried dependency - each iteration depends on previous */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * scalar + arr[i];
        /* Mix operations with different latencies */
        arr[i] ^= 0x5555;  /* Bitwise operation */
        arr[i] += i * 3;   /* Integer multiplication */
    }
    
    /* Use result */
    asm volatile("" : : "r"(arr[n-1]) : "memory");
    sink += arr[n-1];
}

/* Function 2: Reduction loop with floating-point operations */
void func2_fp_reduction(float *farr, int n) {
    volatile float acc = 1.0f;
    
    /* Loop with floating-point operations (higher latency) */
    for (int i = 0; i < n; i++) {
        acc = acc * 1.01f + farr[i];
        /* Mix integer and FP operations */
        if (i % 2 == 0) {
            acc += (float)(i & 0xFF);
        }
    }
    
    /* Force dependency */
    asm volatile("" : : "r"(acc) : "memory");
    sink += (int)acc;
}

/* Function 3: Loop with pointer chasing pattern */
void func3_pointer_chase(int *arr, int n) {
    volatile int *ptr = arr;
    volatile int sum = 0;
    
    /* Pointer chasing creates memory dependencies */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr = arr + ((*ptr) & (n-1));  /* Next location depends on current value */
        /* Additional operations to increase complexity */
        sum ^= (i << 3);
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    sink += sum;
}

/* Function 4: Loop with multiple independent statements then dependent store */
void func4_mixed_ops(short *sarr, int n, int k) {
    volatile int a = 0, b = 0, c = 0;
    
    /* Complex loop body with multiple dependencies */
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        a = sarr[i] * k;
        b = sarr[i-1] + i;
        c = a ^ b;
        
        /* Dependent store with distance 1 */
        sarr[i] = (short)((c + sarr[i-1]) & 0xFFFF);
        
        /* Additional FP operation every 4 iterations */
        if (i % 4 == 0) {
            float ftemp = (float)a * 0.5f;
            sarr[i] += (short)ftemp;
        }
    }
    
    asm volatile("" : : "r"(sarr[n-1]) : "memory");
    sink += sarr[n-1];
}

/* Function 5: Nested loops with conditional inner logic */
void func5_nested_conditional(int *arr, int n, int threshold) {
    volatile int outer_acc = 0;
    
    /* Outer loop */
    for (int j = 0; j < 5; j++) {
        /* Inner loop with conditional */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Complex carried dependency chain */
                int val = arr[i-1] * 3 + arr[i] * 7;
                arr[i] = (val >> 1) + (i & 1);
                
                /* Memory barrier to prevent reordering */
                asm volatile("" : : : "memory");
            }
        }
        outer_acc += arr[n-1];
    }
    
    asm volatile("" : : "r"(outer_acc) : "memory");
    sink += outer_acc;
}

/* Main driver that calls all functions */
int main() {
    /* Initialize arrays with non-zero values */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    float *arr2 = (float*)malloc(SIZE * sizeof(float));
    int *arr3 = (int*)malloc(SIZE * sizeof(int));
    short *arr4 = (short*)malloc(SIZE * sizeof(short));
    int *arr5 = (int*)malloc(SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 7 + 3) & 0xFF;
        arr2[i] = (float)(i * 0.1f);
        arr3[i] = (i * 11 + 5) & 0xFF;
        arr4[i] = (short)(i & 0x7FFF);
        arr5[i] = (i * 13 + 7) & 0xFF;
    }
    
    /* Call each function multiple times with different parameters */
    for (int iter = 0; iter < ITERS; iter++) {
        func1_carried_dep(arr1, SIZE, iter + 2);
        func2_fp_reduction(arr2, SIZE);
        func3_pointer_chase(arr3, SIZE);
        func4_mixed_ops(arr4, SIZE, iter + 3);
        func5_nested_conditional(arr5, SIZE, iter);
        
        /* Modify inputs slightly each iteration */
        arr1[0] ^= iter;
        arr2[0] += 0.1f;
    }
    
    /* Final use of results to prevent elimination */
    volatile int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += arr1[i] + (int)arr2[i] + arr3[i] + arr4[i] + arr5[i];
    }
    
    printf("Result checksum: %d (sink: %d)\n", final_sum, sink);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return 0;
}
