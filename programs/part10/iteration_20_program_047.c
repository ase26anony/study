/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing the complex dependency loop */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size, int* counter) {
    volatile double result = 0.0;  /* Prevent optimization */
    double temp1, temp2, temp3;
    int i, j;
    
    /* Initialize with some values */
    for (i = 0; i < size; i++) {
        arr[i] = (double)i * 1.5;
        indices[i] = i;
    }
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size - 1; i++) {
        /* ========== TRUE DEPENDENCIES (RAW) ========== */
        /* Chain of true dependencies */
        temp1 = arr[i-1] * 2.0;          /* Read arr[i-1] */
        temp2 = temp1 + (double)i;       /* Depends on temp1 */
        temp3 = temp2 / 1.7;             /* Depends on temp2 */
        
        /* ========== ANTI-DEPENDENCIES (WAR) ========== */
        double old_val = arr[i];         /* Read arr[i] before write */
        arr[i] = temp3 + old_val * 0.5;  /* Anti-dep: read then write arr[i] */
        
        /* ========== OUTPUT DEPENDENCIES (WAW) ========== */
        arr[i] = arr[i] * 1.1;           /* Output dep: write arr[i] twice */
        
        /* ========== MEMORY ALIASING DEPENDENCIES ========== */
        /* Pointer aliasing creates ambiguous dependencies */
        double* ptr1 = &arr[i];
        double* ptr2 = &arr[indices[i]]; /* Could alias with ptr1 */
        *ptr1 = *ptr1 + *ptr2 * 0.3;     /* Compiler must assume dependency */
        
        /* ========== MIXED DATA TYPES ========== */
        /* Integer operations mixed with floating point */
        *counter += (int)(arr[i] * 100.0);  /* Integer dependency chain */
        
        /* ========== LOOP-CARRIED DEPENDENCIES ========== */
        /* Cross-iteration dependencies */
        arr[i+1] = arr[i] * 0.9;         /* True dep across iterations */
        
        /* ========== DIFFERENT LATENCY OPERATIONS ========== */
        /* Operations with different execution latencies */
        result += arr[i];                /* Floating add (medium latency) */
        result = result * 1.01;          /* Floating multiply (higher latency) */
        
        /* Memory access with potential cache effects */
        volatile double* vptr = (volatile double*)&arr[i];
        *vptr = *vptr + 0.001;           /* Volatile access */
    }
    
    /* Nested loop for additional complexity */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            /* Complex addressing with dependencies */
            int idx = (i * 11 + j) % size;
            if (idx > 0) {
                /* Memory dependency through pointer */
                double* p = &arr[idx];
                *p = *p + arr[idx-1] * 0.1;  /* True dep + potential anti-dep */
                
                /* Conditional creates control dependencies */
                if (*p > 100.0) {
                    *p = *p / 2.0;
                }
            }
        }
    }
    
    return result;
}

/* Another function with different patterns */
__attribute__((noinline, noclone))
int integer_dep_chain(int* data, int size) {
    int sum = 0;
    int i;
    
    /* Integer dependency chain */
    for (i = 1; i < size; i++) {
        /* Long dependency chain */
        int t1 = data[i-1] + i;
        int t2 = t1 * 2;
        int t3 = t2 - data[i];
        int t4 = t3 / 3;
        data[i] = t4 + data[i-1];  /* True dep + output dep */
        
        /* Anti-dependency */
        int old = data[i];
        data[i] = old * old;
        
        sum += data[i];
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 1024;
    double* array = (double*)malloc(SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int counter = 0;
    double final_result;
    
    if (!array || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Call the complex loop function */
    final_result = compute_loop(array, indices, SIZE, &counter);
    
    /* Call integer dependency function */
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = i;
    }
    int int_sum = integer_dep_chain(int_data, SIZE);
    
    /* Use results to prevent dead code elimination */
    volatile double vol_result = final_result;
    volatile int vol_sum = int_sum;
    volatile int vol_counter = counter;
    
    printf("Results: %f, %d, %d\n", 
           final_result, int_sum, counter);
    
    free(array);
    free(indices);
    free(int_data);
    
    return 0;
}
