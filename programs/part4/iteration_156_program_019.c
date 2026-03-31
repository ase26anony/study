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
        /* Mixed integer operations with dependency chain */
        int temp = arr[i-1] * scalar;
        temp = temp + (temp >> 3);        /* Bitwise operation */
        arr[i] = temp + arr[i] * 7;       /* Another multiplication */
        
        /* Memory barrier to preserve operations */
        asm volatile("" : : "r"(arr[i]) : "memory");
    }
    
    /* Use result to prevent elimination */
    for (int i = 0; i < n; i++) {
        local_sink += arr[i];
    }
    sink += local_sink;
}

/* Function 2: Reduction loop with floating point operations */
void func2_reduction_mixed(float *farr, int *iarr, int n) {
    volatile float f_sink = 0.0f;
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        farr[i] = (float)i * 1.5f;
        iarr[i] = i * 2;
    }
    
    /* Reduction with mixed float/int operations */
    float acc = farr[0];
    for (int i = 1; i < n; i++) {
        /* Floating point operation with dependency */
        acc = acc * 1.1f + farr[i];
        
        /* Integer operation in same loop */
        iarr[i] = iarr[i-1] + iarr[i] * 3;
        
        /* Dependency between float and int */
        farr[i] = acc + (float)iarr[i];
    }
    
    f_sink = acc;
    sink += (int)f_sink;
}

/* Function 3: Pointer chasing pattern with conditional */
void func3_pointer_chase(int **ptr_arr, int n) {
    volatile int chase_sink = 0;
    int *current = ptr_arr[0];
    
    /* Pointer chasing with conditional inner logic */
    for (int i = 1; i < n; i++) {
        if (current != NULL) {
            /* Load from pointer creates dependency */
            int val = *current;
            
            /* Some computation */
            val = val * 13 + 7;
            val = val ^ (val << 3);  /* Bitwise operation */
            
            /* Store and update pointer */
            *current = val;
            current = ptr_arr[i % n];
            
            /* Compiler barrier */
            asm volatile("" : : "r"(val) : "memory");
        }
    }
    
    chase_sink = (current != NULL) ? *current : 0;
    sink += chase_sink;
}

/* Function 4: Nested loops with inner carried dependency */
void func4_nested_loops(int *arr, int n, int outer_iters) {
    volatile int nest_sink = 0;
    
    for (int iter = 0; iter < outer_iters; iter++) {
        /* Inner loop with carried dependency */
        int base = iter * 17;
        for (int i = 1; i < n; i++) {
            /* Complex dependency chain */
            int a = arr[i-1] * 3;
            int b = a + (a << 2);      /* Shift operation */
            int c = b ^ (b >> 1);      /* More bitwise ops */
            arr[i] = c + base + (arr[i] & 0xFF);
        }
        
        /* Use result to prevent elimination */
        nest_sink += arr[n-1];
    }
    
    sink += nest_sink;
}

/* Function 5: Multiple independent statements with final dependent store */
void func5_multi_indep(float *farr, int *iarr, int n) {
    volatile float multi_sink = 0.0f;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        float f1 = farr[i] * 2.5f;
        float f2 = farr[i-1] * 1.8f;
        int i1 = iarr[i] * 11;
        int i2 = iarr[i-1] * 7;
        
        /* Some mixing */
        f1 = f1 + (float)i1 * 0.1f;
        f2 = f2 + (float)i2 * 0.2f;
        
        /* Final dependent store with multiple inputs */
        farr[i] = f1 * f2 + (float)(i1 ^ i2);
        
        /* Memory barrier */
        asm volatile("" : : "r"(farr[i]), "r"(iarr[i]) : "memory");
    }
    
    for (int i = 0; i < n; i++) {
        multi_sink += farr[i];
    }
    sink += (int)multi_sink;
}

/* Main function that exercises all patterns */
int main() {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    float *farr = (float*)malloc(SIZE * sizeof(float));
    int *iarr = (int*)malloc(SIZE * sizeof(int));
    int **ptr_arr = (int**)malloc(SIZE * sizeof(int*));
    
    if (!arr1 || !farr || !iarr || !ptr_arr) {
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        farr[i] = (float)(i + 1) * 0.7f;
        iarr[i] = i * 3 + 1;
        ptr_arr[i] = &iarr[i % SIZE];
    }
    
    /* Call all functions to ensure they're compiled and executed */
    func1_carried_dep(arr1, SIZE, 5);
    func2_reduction_mixed(farr, iarr, SIZE);
    func3_pointer_chase(ptr_arr, SIZE);
    func4_nested_loops(arr1, SIZE, 10);
    func5_multi_indep(farr, iarr, SIZE);
    
    /* Additional loop in main to increase chances */
    volatile int main_sink = 0;
    for (int i = 1; i < ITERS; i++) {
        /* Another carried dependency pattern */
        arr1[i] = arr1[i-1] * 3 + arr1[i] * 2;
        /* Mix with floating point */
        farr[i] = farr[i-1] * 1.3f + (float)arr1[i];
        
        /* Prevent optimization */
        asm volatile("" : : "r"(arr1[i]), "r"(farr[i]) : "memory");
    }
    
    /* Final sink calculation */
    for (int i = 0; i < SIZE; i++) {
        main_sink += arr1[i] + (int)farr[i] + iarr[i];
    }
    sink += main_sink;
    
    /* Print something to ensure execution */
    printf("Result: %d\n", sink);
    
    /* Cleanup */
    free(arr1);
    free(farr);
    free(iarr);
    free(ptr_arr);
    
    return 0;
}
