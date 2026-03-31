/* Test program for GCC modulo scheduler edge printing coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent dead code elimination */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void loop1_carried_dependency(volatile int* arr, int n) {
    if (n < 2) return;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = arr[i-1] * 3 + 7;
        arr[i] = (temp >> 2) | (arr[i] & 0xFF);
        
        /* Add some floating point operations */
        float ftemp = (float)arr[i] * 1.5f;
        /* Use asm to create artificial dependency */
        asm volatile("" : "+r"(arr[i]) : : "memory");
        arr[i] += (int)ftemp;
    }
}

/* Function 2: Reduction loop with pointer chasing pattern */
int loop2_reduction_chasing(int* data, int n) {
    if (n <= 0) return 0;
    
    int sum = data[0];
    int* p = data;
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with dependency */
        p = data + (i % n);
        sum += *p * i;
        
        /* Mixed operations to create complex latency pattern */
        sum = (sum << 3) | (sum >> 29); /* rotate */
        sum ^= data[i % 16]; /* create dependency on array */
        
        /* Floating point in the middle */
        float fsum = (float)sum * 0.75f;
        sum = (int)fsum + (sum & 0x7FF);
    }
    
    return sum;
}

/* Function 3: Loop with multiple independent statements and dependent store */
void loop3_multiple_deps(double* darr, int* iarr, int n) {
    if (n < 4) return;
    
    for (int i = 3; i < n; i++) {
        /* Multiple independent calculations */
        double d1 = darr[i-1] * 2.5;
        double d2 = darr[i-2] * 1.8;
        double d3 = darr[i-3] * 0.7;
        
        /* Dependent operation with mixed types */
        int i1 = (int)(d1 * 100.0);
        int i2 = (int)(d2 * 50.0);
        int i3 = (int)(d3 * 25.0);
        
        /* Final store with complex dependency */
        iarr[i] = i1 + i2 * i3 - iarr[i-1];
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(iarr[i]), "r"(darr[i-1]) : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
void loop4_nested_conditional(int* arr, int n, int outer_iters) {
    volatile int cond = 1;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Outer loop with varying condition */
        if (cond || (outer % 3 == 0)) {
            /* Inner loop with carried dependency */
            for (int i = 2; i < n; i++) {
                /* Complex dependency chain */
                int val = arr[i-1] * arr[i-2];
                val = (val + 12345) ^ (val >> 4);
                
                /* Floating point with dependency */
                float fval = (float)val * 1.234f;
                arr[i] = (int)fval + arr[i] * 2;
                
                /* Conditional update to create control flow */
                if (val % 7 == 0) {
                    arr[i] += 1000;
                }
            }
        }
        
        /* Toggle condition */
        cond = !cond;
    }
}

/* Function 5: Loop with different distance dependencies */
void loop5_variable_distance(short* sarr, int* iarr, int n) {
    if (n < 10) return;
    
    for (int i = 9; i < n; i++) {
        /* Dependencies with different distances */
        int d1 = iarr[i-1];  /* distance 1 */
        int d2 = iarr[i-3];  /* distance 3 */
        int d4 = iarr[i-7];  /* distance 7 */
        
        /* Complex calculation with mixed operations */
        int result = d1 * d2 + (d4 >> 2);
        result = result * 3 - (result % 17);
        
        /* Store with type conversion */
        sarr[i] = (short)(result & 0xFFFF);
        
        /* Create anti-dependency */
        iarr[i-5] = result ^ iarr[i-5];
    }
}

int main() {
    /* Initialize arrays with non-zero values */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    double darr[SIZE];
    int arr3[SIZE];
    short sarr[SIZE];
    int arr4[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) % 100;
        arr2[i] = (i * 5 + 11) % 200;
        darr[i] = (double)(i * 7 + 13) / 10.0;
        arr3[i] = (i * 11 + 17) % 150;
        sarr[i] = (short)((i * 13 + 19) % 30000);
        arr4[i] = (i * 17 + 23) % 180;
    }
    
    /* Execute all loops to ensure they're compiled */
    loop1_carried_dependency(arr1, SIZE);
    
    int sum2 = loop2_reduction_chasing(arr2, SIZE);
    global_sink += sum2;
    
    loop3_multiple_deps(darr, arr3, SIZE);
    
    loop4_nested_conditional(arr4, SIZE, 5);
    
    loop5_variable_distance(sarr, arr3, SIZE);
    
    /* Use results to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        global_sink += arr1[i] + arr2[i] + arr3[i] + sarr[i] + arr4[i];
    }
    
    /* Simple checksum output */
    printf("Checksum: %d\n", global_sink % 1000);
    
    return 0;
}
