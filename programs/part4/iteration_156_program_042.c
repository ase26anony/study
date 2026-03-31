/* Test program for GCC modulo scheduler edge printing coverage */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n, int scalar) {
    int result = 0;
    /* Loop with carried dependency distance 1 */
    for (int i = 1; i < n; i++) {
        /* Mixed operations with different latencies */
        int temp = arr[i-1] * scalar;      /* Integer multiply */
        temp = temp + (temp >> 3);         /* Shift and add */
        arr[i] = temp + arr[i];            /* Dependency chain */
        result ^= arr[i];                  /* XOR accumulation */
    }
    return result;
}

/* Function 2: Floating-point loop with recurrence */
float func2_fp_recurrence(float* arr, int n, float coeff) {
    float acc = arr[0];
    /* FP operations with potential higher latency */
    for (int i = 1; i < n; i++) {
        float val = arr[i] * coeff;        /* FP multiply */
        acc = acc + val;                   /* FP add with recurrence */
        arr[i] = acc * 0.5f;               /* Another FP multiply */
        
        /* Mix with integer ops */
        int idx = i & 0xFF;
        acc += (float)idx * 0.01f;
    }
    return acc;
}

/* Function 3: Pointer chasing pattern */
int func3_pointer_chase(int* base, int n) {
    int* p = base;
    int sum = 0;
    
    /* Create linked-list like access pattern */
    for (int i = 0; i < n; i++) {
        sum += *p;
        /* Pointer chase with stride */
        p = base + ((*p) & 0x3F);  /* Limited range to stay in bounds */
        
        /* Additional operations to create scheduling complexity */
        sum = (sum << 1) | (sum >> 31);  /* Rotation */
        sum ^= i * 0x9E3779B9;           /* Mix with constant */
    }
    return sum;
}

/* Function 4: Multiple independent statements with final dependency */
void func4_multi_dep(int* a, int* b, int* c, int n) {
    /* Outer loop to encourage modulo scheduling */
    for (int j = 0; j < 5; j++) {
        /* Inner loop with complex dependency pattern */
        for (int i = 1; i < n; i++) {
            /* Independent computations */
            int t1 = a[i] * b[i];
            int t2 = a[i-1] + b[i-1];
            int t3 = t1 ^ t2;
            
            /* Conditional to add complexity */
            if (t3 > 0) {
                c[i] = t3 + c[i-1];  /* Carried dependency */
            } else {
                c[i] = t3 - c[i-1];
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile("" : : "r"(c[i]) : "memory");
        }
    }
}

/* Function 5: Nested loops with conditional inner logic */
int func5_nested_conditional(int* arr, int n, int threshold) {
    int total = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 3; outer++) {
        /* Conditional inner loop */
        if (threshold > 0) {
            /* Inner loop with mixed operations */
            for (int i = 1; i < n; i++) {
                /* Complex expression with multiple dependencies */
                int val = arr[i] * 3;
                val += arr[i-1] * 2;      /* Distance 1 dependency */
                val = (val << 2) | (val >> 30);  /* Bit rotation */
                
                /* Floating-point conversion and back */
                float fval = (float)val * 1.5f;
                arr[i] = (int)fval + (arr[i] & 1);  /* Mix with bitwise */
                
                total += arr[i];
            }
        }
        
        /* Modify threshold to affect next iteration */
        threshold ^= total & 1;
    }
    
    return total;
}

/* Main driver */
int main() {
    /* Initialize data */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    int arr4_a[SIZE], arr4_b[SIZE], arr4_c[SIZE];
    int arr5[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 1) & 0xFF;
        arr2[i] = (float)(i * 5 + 2) * 0.1f;
        arr3[i] = (i * 7 + 3) & 0x3F;  /* Keep within bounds for pointer chase */
        arr4_a[i] = i * 11;
        arr4_b[i] = i * 13;
        arr4_c[i] = i * 17;
        arr5[i] = i * 19;
    }
    
    /* Call all functions to ensure they're compiled and executed */
    int res1 = func1_carried_dep(arr1, ITERS, 3);
    float res2 = func2_fp_recurrence(arr2, ITERS, 1.25f);
    int res3 = func3_pointer_chase(arr3, ITERS);
    func4_multi_dep(arr4_a, arr4_b, arr4_c, ITERS);
    int res5 = func5_nested_conditional(arr5, ITERS, 1);
    
    /* Use results to prevent dead code elimination */
    sink = res1 + (int)res2 + res3 + arr4_c[ITERS-1] + res5;
    
    /* Print minimal output */
    printf("Result checksum: %d\n", sink);
    
    return 0;
}
