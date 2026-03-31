/* test_modulo_sched.c - Program to trigger GCC's modulo scheduler edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define ITERS 1000

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(volatile int* arr, int n, int scalar) {
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = arr[i-1] * scalar + arr[i];
        arr[i] = (temp >> 3) & 0xFF;  /* Bitwise ops */
        
        /* Fake dependency with asm to prevent optimization */
        asm volatile("" : "+r"(arr[i]) : : "memory");
    }
}

/* Function 2: Reduction loop with floating point and integer mix */
float func2_reduction(float* farr, int* iarr, int n) {
    float sum = 0.0f;
    float prod = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Floating point operations with higher latency */
        float ftemp = farr[i] * 1.5f + 0.25f;
        
        /* Integer operations mixed in */
        int itemp = iarr[i] * 2 + 1;
        
        /* Cross-type dependency */
        sum += ftemp + (float)itemp;
        prod *= ftemp;
        
        /* Memory barrier to preserve operations */
        asm volatile("" : "+r"(sum), "+r"(prod) : : "memory");
    }
    
    return sum + prod;
}

/* Function 3: Pointer chasing pattern with conditional inner logic */
void func3_pointer_chase(int** ptr_arr, int n) {
    int* current = ptr_arr[0];
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with dependency */
        int* next = (int*)((long)current + *current);
        
        /* Conditional operation inside loop */
        if (next != NULL && (i % 7) == 0) {
            *next = *current * 3 + i;
        }
        
        current = ptr_arr[i % n];
        
        /* Compiler barrier */
        asm volatile("" : "+r"(current) : : "memory");
    }
}

/* Function 4: Multiple independent statements with final dependent store */
void func4_multi_ops(double* darr, int* iarr, int n) {
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        double d1 = darr[i-1] * 2.5;
        double d2 = darr[i] / 1.7;
        int i1 = iarr[i-1] ^ 0xAA55;
        int i2 = iarr[i] << 2;
        
        /* Dependent combination */
        double result = d1 + d2 + (double)(i1 + i2);
        
        /* Store with dependency on all above */
        darr[i] = result;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(result) : : "memory");
    }
}

/* Function 5: Nested loop with outer loop controlling inner modulo-scheduled loop */
void func5_nested_loops(int* arr, int outer_iters, int inner_size) {
    volatile int cond = 1;
    
    for (int o = 0; o < outer_iters; o++) {
        /* Outer loop condition that's not trivially predictable */
        if (cond || (o % 3) == 0) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < inner_size; i++) {
                /* Complex dependency chain */
                int val = arr[i-1] * arr[i] + o;
                arr[i] = (val & 0xFF) | ((val >> 8) & 0xFF00);
                
                /* Mix of operations */
                arr[i] += (i % 5) == 0 ? val * 2 : val / 2;
            }
        }
        
        /* Toggle condition */
        cond = !cond;
        asm volatile("" : "+r"(cond) : : "memory");
    }
}

/* Main function that exercises all patterns */
int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float farr[SIZE];
    int iarr[SIZE];
    double darr[SIZE];
    int* ptr_arr[SIZE];
    int arr2[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        farr[i] = (float)i * 0.5f;
        iarr[i] = i * 3;
        darr[i] = (double)i * 0.25;
        ptr_arr[i] = &iarr[i % SIZE];
        arr2[i] = i * 2;
    }
    
    /* Execute each function multiple times */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Call function 1 - carried dependency */
        func1_carried_dep(arr1, SIZE, iter + 1);
        
        /* Call function 2 - reduction with FP */
        float fresult = func2_reduction(farr, iarr, SIZE);
        global_sink += (int)fresult;
        
        /* Call function 3 - pointer chasing */
        func3_pointer_chase(ptr_arr, SIZE);
        
        /* Call function 4 - multiple ops */
        func4_multi_ops(darr, iarr, SIZE);
        
        /* Call function 5 - nested loops */
        func5_nested_loops(arr2, 10, SIZE);
        
        /* Modify inputs slightly each iteration */
        if (iter % 2 == 0) {
            arr1[0] += 1;
            farr[0] += 0.1f;
        }
    }
    
    /* Use results to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + (int)farr[i] + iarr[i] + (int)darr[i] + arr2[i];
    }
    
    printf("Checksum: %d (global_sink: %d)\n", checksum, global_sink);
    
    return 0;
}
