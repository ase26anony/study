/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency (distance 1) */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with carried dependency */
        varr[i] = varr[i-1] * scalar + varr[i];
        varr[i] ^= (varr[i] << 3);  /* Additional operation to increase complexity */
    }
}

/* Function 2: Loop with floating-point and integer mix */
void func2_mixed_ops(float *farr, int *iarr, int n) {
    volatile float *vfarr = (volatile float *)farr;
    volatile int *viarr = (volatile int *)iarr;
    
    for (int i = 1; i < n; i++) {
        /* Floating-point operation with dependency */
        vfarr[i] = vfarr[i-1] * 1.5f + vfarr[i];
        
        /* Integer operation with separate dependency */
        viarr[i] = viarr[i-1] + viarr[i] * 2;
        
        /* Cross-type dependency to complicate scheduling */
        asm volatile("" : : "r"(vfarr[i]), "r"(viarr[i]) : "memory");
    }
}

/* Function 3: Reduction loop with multiple dependencies */
int func3_reduction(const int *arr, int n) {
    int sum = arr[0];
    volatile int vsum = sum;
    
    for (int i = 1; i < n; i++) {
        /* Multiple dependent operations */
        int temp = arr[i] * 3;
        vsum = vsum + temp;
        vsum = vsum ^ (vsum >> 2);
        
        /* Memory barrier to preserve ordering */
        asm volatile("" : : "r"(vsum) : "memory");
    }
    return vsum;
}

/* Function 4: Nested loops with conditional inner logic */
void func4_nested_conditional(int *arr, int n, int threshold) {
    volatile int *varr = (volatile int *)arr;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with conditional */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                varr[i] = varr[i-1] * varr[i] + outer;
                varr[i] = (varr[i] << 1) | (varr[i] >> 31);  /* Rotation */
                
                /* Conditional operation */
                if (varr[i] > threshold) {
                    varr[i] -= threshold;
                }
            }
        }
    }
}

/* Function 5: Pointer chasing pattern */
void func5_pointer_chase(int *arr, int n) {
    volatile int *varr = (volatile int *)arr;
    int *ptr = &varr[0];
    
    for (int i = 0; i < n-1; i++) {
        /* Pointer chasing with dependency */
        int val = *ptr;
        ptr = &varr[i+1];
        *ptr = val * 7 + i;
        
        /* Additional operation to increase ILP opportunity */
        varr[i] = varr[i] ^ (val << i);
    }
}

/* Function 6: Loop with multiple independent statements */
void func6_multiple_independent(int *arr1, int *arr2, int *arr3, int n) {
    volatile int *v1 = (volatile int *)arr1;
    volatile int *v2 = (volatile int *)arr2;
    volatile int *v3 = (volatile int *)arr3;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int t1 = v1[i-1] * 3;
        int t2 = v2[i-1] + 5;
        int t3 = v3[i-1] ^ 0xFF;
        
        /* Synchronization point */
        asm volatile("" : : "r"(t1), "r"(t2), "r"(t3) : "memory");
        
        /* Dependent store */
        v1[i] = t1 + t2;
        v2[i] = t2 * t3;
        v3[i] = t1 ^ t3;
    }
}

int main() {
    /* Initialize arrays with non-zero values */
    int int_arr[SIZE];
    float float_arr[SIZE];
    int arr2[SIZE], arr3[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 3 + 7) & 0xFF;
        float_arr[i] = (float)i * 0.5f;
        arr2[i] = i * 2;
        arr3[i] = i * 5;
    }
    
    /* Call all functions to ensure they're compiled */
    func1_carried_dep(int_arr, SIZE, 3);
    func2_mixed_ops(float_arr, int_arr, SIZE);
    
    int sum1 = func3_reduction(int_arr, SIZE);
    func4_nested_conditional(int_arr, SIZE, 100);
    func5_pointer_chase(int_arr, SIZE);
    func6_multiple_independent(int_arr, arr2, arr3, SIZE);
    
    /* Use results to prevent dead code elimination */
    global_sink += sum1;
    for (int i = 0; i < SIZE; i++) {
        global_sink += int_arr[i];
        global_sink += (int)float_arr[i];
        global_sink += arr2[i];
        global_sink += arr3[i];
    }
    
    /* Print minimal output to verify execution */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
