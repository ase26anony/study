/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERATIONS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void loop1_carried_dependency(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mixed operations with different latencies */
        int temp = varr[i-1] * scalar;      /* Integer multiplication */
        temp = temp + (temp >> 3);          /* Shift and add */
        temp = temp ^ (temp * 3);           /* XOR with multiplication */
        varr[i] = temp + varr[i];           /* Dependency chain */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(temp) : "memory");
    }
}

/* Function 2: Reduction loop with floating point operations */
float loop2_floating_point_reduction(float *arr, int n) {
    volatile float *varr = (volatile float *)arr;
    float sum = varr[0];
    
    /* Loop with floating point operations */
    for (int i = 1; i < n; i++) {
        /* Floating point operations (higher latency) */
        float prod = sum * 1.5f;            /* FP multiplication */
        sum = prod + varr[i];               /* FP addition with dependency */
        sum = sum * 0.99f;                  /* Another FP multiplication */
        
        /* Integer operations mixed in */
        int idx = (int)sum & (n-1);         /* Bitwise AND */
        sum += (float)idx * 0.1f;
    }
    return sum;
}

/* Function 3: Pointer chasing pattern */
void loop3_pointer_chasing(int **ptr_arr, int n) {
    volatile int **vptr = (volatile int **)ptr_arr;
    int *current = vptr[0];
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with arithmetic */
        int val = *current;
        val = val * 7 + 13;
        *current = val;
        
        /* Update pointer with dependency */
        current = vptr[(val + i) % n];
        
        /* Compiler barrier */
        asm volatile("" : : "r"(val), "r"(current) : "memory");
    }
}

/* Function 4: Multiple independent statements with final dependency */
void loop4_multiple_stmts(int *arr1, int *arr2, int n) {
    volatile int *varr1 = (volatile int *)arr1;
    volatile int *varr2 = (volatile int *)arr2;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int a = varr1[i] * 2;
        int b = varr2[i] + 5;
        int c = a ^ b;
        int d = c << 2;
        
        /* Final dependent store with carried dependency */
        varr1[i] = d + varr1[i-1];
        varr2[i] = varr1[i] * 3;
    }
}

/* Function 5: Nested loops with conditional inner logic */
void loop5_nested_conditional(int *arr, int n, int threshold) {
    volatile int *varr = (volatile int *)arr;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Conditional that prevents dead code elimination */
        if (threshold > outer) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                int base = varr[i-1];
                int scaled = base * (i + outer);
                int masked = scaled & 0xFF;
                varr[i] = masked + (varr[i] >> 1);
                
                /* Memory barrier */
                asm volatile("" : : "r"(scaled) : "memory");
            }
        }
    }
}

/* Main function that exercises all loops */
int main() {
    /* Initialize arrays with non-zero values */
    int arr_int1[SIZE], arr_int2[SIZE];
    float arr_float[SIZE];
    int *ptr_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr_int1[i] = i + 1;
        arr_int2[i] = i * 2 + 1;
        arr_float[i] = (float)i * 0.5f;
        ptr_arr[i] = &arr_int1[i % SIZE];
    }
    
    /* Call each loop function multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        int scalar = iter + 2;
        
        /* Function 1 */
        loop1_carried_dependency(arr_int1, SIZE, scalar);
        
        /* Function 2 */
        float fp_result = loop2_floating_point_reduction(arr_float, SIZE);
        sink += (int)fp_result;
        
        /* Function 3 */
        loop3_pointer_chasing(ptr_arr, SIZE / 4);
        
        /* Function 4 */
        loop4_multiple_stmts(arr_int1, arr_int2, SIZE);
        
        /* Function 5 */
        loop5_nested_conditional(arr_int2, SIZE, iter % 10);
        
        /* Use results to prevent elimination */
        sink += arr_int1[SIZE-1] + arr_int2[SIZE-1];
    }
    
    /* Final checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr_int1[i] + arr_int2[i] + (int)arr_float[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
