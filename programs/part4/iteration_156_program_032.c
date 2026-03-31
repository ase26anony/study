/* Test program for GCC modulo scheduler edge printing coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int local_sink = 0;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix of integer operations with different latencies */
        int temp = arr[i-1] * scalar;      /* Multiplication */
        temp = temp + (arr[i] & 0xFF);     /* Bitwise AND + addition */
        temp = temp ^ (temp >> 3);         /* Shift + XOR */
        arr[i] = temp + i;                 /* Index-dependent store */
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : "+r"(temp) : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        local_sink += arr[i];
    }
    sink += local_sink;
}

/* Function 2: Reduction loop with floating-point operations */
void func2_reduction_mixed(float *farr, int *iarr, int n) {
    volatile float fsum = 0.0f;
    volatile int isum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        farr[i] = (float)i * 1.5f;
        iarr[i] = i * 2;
    }
    
    /* Reduction with mixed int/float operations */
    for (int i = 1; i < n; i++) {
        /* Floating-point operations (higher latency) */
        float ftemp = farr[i-1] * 2.0f;
        ftemp = ftemp + farr[i] * 0.5f;
        
        /* Integer operations interleaved */
        int itemp = iarr[i-1] + iarr[i];
        itemp = itemp * 3;
        
        /* Cross-type dependency */
        farr[i] = ftemp + (float)itemp;
        iarr[i] = itemp + (int)ftemp;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(ftemp), "r"(itemp) : "memory");
    }
    
    /* Collect results */
    for (int i = 0; i < n; i++) {
        fsum += farr[i];
        isum += iarr[i];
    }
    sink += (int)fsum + isum;
}

/* Function 3: Pointer chasing with conditional inner logic */
void func3_pointer_chase(int *data, int n) {
    volatile int result = 0;
    int *ptr = data;
    
    /* Outer loop to encourage modulo scheduling */
    for (int outer = 0; outer < 10; outer++) {
        /* Reset pointer */
        ptr = data + (outer % n);
        
        /* Inner loop with pointer chasing */
        for (int i = 0; i < n/2; i++) {
            /* Conditional to create complex control flow */
            if (ptr < data + n - 1) {
                /* Load-use dependency chain */
                int val = *ptr;
                val = val * 7 + 13;
                *(ptr + 1) = val;
                ptr++;
                
                /* Additional operations to increase ILP opportunity */
                int temp = val ^ (val << 2);
                temp = temp >> 1;
                result += temp;
            }
        }
        
        /* Memory barrier */
        asm volatile("" : : "r"(ptr) : "memory");
    }
    
    sink += result;
}

/* Function 4: Multiple independent statements with final dependency */
void func4_multiple_deps(double *darr, int *iarr, int n) {
    volatile double dacc = 0.0;
    volatile int iacc = 0;
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        darr[i] = (double)i * 0.25;
        iarr[i] = i * i;
    }
    
    /* Loop with multiple independent chains converging */
    for (int i = 1; i < n; i++) {
        /* Chain A: Integer operations */
        int a = iarr[i-1] * 3;
        a = a + (a % 17);
        
        /* Chain B: Floating-point operations */
        double b = darr[i-1] * 1.75;
        b = b + 2.5;
        
        /* Chain C: Memory access pattern */
        int c = iarr[i] ^ iarr[i-1];
        c = c << 1;
        
        /* Converging dependency */
        double d = b * (double)a;
        d = d + (double)c;
        
        /* Final store with dependency on all chains */
        darr[i] = d;
        iarr[i] = (int)d + a + c;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
    }
    
    /* Use results */
    for (int i = 0; i < n; i++) {
        dacc += darr[i];
        iacc += iarr[i];
    }
    sink += (int)dacc + iacc;
}

/* Main driver */
int main() {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    float *arr2 = (float*)malloc(SIZE * sizeof(float));
    int *arr3 = (int*)malloc(SIZE * sizeof(int));
    double *arr4 = (double*)malloc(SIZE * sizeof(double));
    int *arr5 = (int*)malloc(SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr3[i] = i * 3;
        arr5[i] = i * 5;
    }
    
    /* Call functions with different loop patterns */
    func1_carried_dep(arr1, SIZE, 7);
    func2_reduction_mixed(arr2, arr3, SIZE);
    func3_pointer_chase(arr5, SIZE);
    func4_multiple_deps(arr4, arr5, SIZE);
    
    /* Additional calls with different parameters */
    for (int repeat = 0; repeat < 3; repeat++) {
        func1_carried_dep(arr1, SIZE/2, repeat + 2);
        func3_pointer_chase(arr3, SIZE/4);
    }
    
    /* Final use of results to prevent elimination */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += arr1[i] + arr3[i] + arr5[i] + (int)arr2[i] + (int)arr4[i];
    }
    sink += final_sum;
    
    /* Print minimal output */
    printf("Result checksum: %d\n", sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return 0;
}
