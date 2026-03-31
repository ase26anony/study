/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERATIONS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n) {
    int result = 0;
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix of integer operations with different latencies */
        int temp = arr[i-1] * 3 + arr[i];      /* Multiplication + addition */
        temp = (temp >> 2) | (temp << 30);     /* Bitwise operations */
        arr[i] = temp + i;                     /* Store with index dependency */
        result ^= temp;                        /* Reduction */
    }
    /* Compiler barrier to preserve operations */
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Function 2: Loop with floating-point operations and dependencies */
float func2_fp_dep(volatile float* farr, int n) {
    float sum = farr[0];
    /* Loop with floating-point carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mix of FP operations with higher latencies */
        float temp = farr[i-1] * 1.5f + farr[i];  /* FP multiply-add */
        temp = temp / 2.0f - 1.0f;                /* More FP ops */
        farr[i] = temp * 0.9f;                    /* Store with dependency */
        sum += temp;                              /* FP reduction */
        
        /* Insert integer operation to create mixed dependency graph */
        if ((i & 3) == 0) {
            sum = sum * 0.5f;                     /* Conditional FP op */
        }
    }
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Function 3: Loop with pointer chasing pattern */
int func3_pointer_chase(volatile int* arr, int n) {
    int val = arr[0];
    /* Pointer-chasing like pattern with stride */
    for (int i = 1; i < n; i += 2) {
        /* Load from computed address (creates dependency chain) */
        int next = arr[(val + i) % n];
        /* Arithmetic with dependency */
        val = (val * 1103515245 + 12345) & 0x7fffffff;
        arr[i] = val ^ next;                     /* Store with two dependencies */
        
        /* Additional independent operation to create parallel paths */
        int independent = arr[n-i-1] * 7;
        arr[i+1] = independent;
    }
    asm volatile("" : : "r"(val) : "memory");
    return val;
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested_conditional(volatile int* arr, int n, int threshold) {
    int total = 0;
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with conditional execution */
        if (outer < threshold) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                int a = arr[i-1] * 2;
                int b = a + arr[i];
                int c = b ^ (b >> 4);
                arr[i] = c + outer;              /* Dependency on outer loop var */
                total += c;
                
                /* Conditional operation inside inner loop */
                if (c > 1000) {
                    total -= 500;
                }
            }
        }
    }
    asm volatile("" : : "r"(total) : "memory");
    return total;
}

/* Function 5: Reduction loop with multiple accumulators */
long func5_multiple_reductions(volatile int* arr, int n) {
    long sum1 = 0, sum2 = 0, sum3 = 0;
    /* Unrolled pattern with dependencies */
    for (int i = 0; i < n - 3; i += 4) {
        /* Independent computations followed by dependent store */
        int t1 = arr[i] * 3;
        int t2 = arr[i+1] * 5;
        int t3 = arr[i+2] * 7;
        int t4 = arr[i+3] * 11;
        
        /* Create cross-iteration dependencies */
        arr[i] = t1 + sum1;
        arr[i+1] = t2 + sum2;
        arr[i+2] = t3 + sum3;
        arr[i+3] = t4 + (t1 ^ t2);
        
        /* Multiple reduction chains */
        sum1 += t1;
        sum2 += t2 ^ t3;
        sum3 += t4 * 2;
    }
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    return sum1 + sum2 + sum3;
}

/* Main driver that ensures all loops execute */
int main(int argc, char** argv) {
    /* Initialize data with non-zero values */
    volatile int int_array[SIZE];
    volatile float float_array[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 7 + 3) & 0xFF;
        float_array[i] = (float)(i * 0.1f + 0.5f);
    }
    
    int threshold = (argc > 1) ? atoi(argv[1]) : 3;
    if (threshold < 0) threshold = 0;
    if (threshold > 5) threshold = 5;
    
    /* Execute all functions multiple times to ensure they're not optimized away */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_result ^= func1_carried_dep(int_array, SIZE);
        
        /* Cast result to int for accumulation */
        total_result ^= (int)func2_fp_dep(float_array, SIZE);
        
        total_result ^= func3_pointer_chase(int_array, SIZE);
        
        total_result ^= func4_nested_conditional(int_array, SIZE, threshold);
        
        total_result ^= (int)func5_multiple_reductions(int_array, SIZE);
        
        /* Modify arrays slightly each iteration */
        int_array[iter % SIZE] ^= iter;
        float_array[iter % SIZE] += 0.1f;
    }
    
    /* Use results to prevent dead code elimination */
    sink = total_result;
    
    /* Print minimal output to verify execution */
    printf("Result checksum: %d\n", total_result & 0xFF);
    
    return 0;
}
