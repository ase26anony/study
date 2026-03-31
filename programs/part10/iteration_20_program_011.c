/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size, int* counter) {
    volatile double result = 0.0;
    double temp1, temp2, temp3;
    int i, j;
    
    /* Multiple carried dependencies in nested loops */
    for (i = 1; i < size; i++) {
        /* True Data Dependency (RAW) with floating point */
        arr[i] = arr[i-1] * 1.5 + (double)i;
        
        /* Anti-dependency (WAR) - read then write same location */
        temp1 = arr[i];           /* Read arr[i] */
        arr[i] = temp1 + 0.5;     /* Write arr[i] - WAR with previous read */
        
        /* Output Dependency (WAW) - multiple writes to same location */
        arr[i] = arr[i] * 2.0;    /* First write */
        arr[i] = arr[i] / 1.5;    /* Second write - WAW with previous */
        
        /* Memory dependency with pointer aliasing */
        double* ptr = &arr[indices[i] % size];
        *ptr = *ptr + arr[i];     /* Potential memory dependency */
        
        /* Mixed data types creating different latency operations */
        temp2 = (double)((int)arr[i] + i);  /* Integer cast and float */
        temp3 = temp1 * temp2;              /* FP multiply (higher latency) */
        
        /* Control dependency affecting scheduling */
        if (arr[i] > 100.0) {
            *counter += 1;         /* Memory store with control dependency */
        }
        
        /* Complex expression with multiple dependencies */
        result = result + arr[i] * 0.1 + temp3;
    }
    
    /* Additional loop with different patterns */
    for (j = size - 1; j > 0; j--) {
        /* Reverse carried dependency */
        arr[j-1] = arr[j] * 0.9 + (double)j;
        
        /* Integer operations with different latency */
        indices[j] = indices[j-1] + indices[j] * 3;
        
        /* Pointer chasing creating memory dependencies */
        double* alias_ptr = arr + (indices[j] % size);
        *alias_ptr = *alias_ptr * 1.1;
    }
    
    return result;
}

/* Another function with different dependency patterns */
__attribute__((noinline, noclone))
int integer_dep_loop(int* data, float* fdata, int size) {
    int sum = 0;
    int temp_int;
    float temp_float;
    
    for (int i = 1; i < size; i++) {
        /* Integer RAW dependency */
        data[i] = data[i-1] + i * 2;
        
        /* Float-integer mixed dependency */
        temp_float = (float)data[i] * 1.5f;
        fdata[i] = fdata[i-1] + temp_float;
        
        /* WAR dependency with array */
        temp_int = data[i];        /* Read */
        data[i] = temp_int * 3;    /* Write - WAR */
        
        /* Complex expression with multiple uses */
        sum += data[i] + (int)fdata[i];
        
        /* WAW dependency through pointer */
        int* ptr = &data[i % size];
        *ptr = *ptr + 1;           /* WAW potential */
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 256;
    double* arr = (double*)malloc(SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    int counter = 0;
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (double)(i + 1) * 0.5;
        indices[i] = i;
        int_data[i] = i * 2;
        float_data[i] = (float)i * 0.25f;
    }
    
    /* Force compiler to consider all dependencies */
    volatile double result1 = compute_loop(arr, indices, SIZE, &counter);
    volatile int result2 = integer_dep_loop(int_data, float_data, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %f, Result2: %d, Counter: %d\n", 
           result1, result2, counter);
    
    /* Additional computation to increase optimization opportunities */
    double final_check = 0.0;
    for (int i = 0; i < SIZE; i++) {
        final_check += arr[i] + int_data[i] + float_data[i];
    }
    printf("Final check: %f\n", final_check);
    
    free(arr);
    free(indices);
    free(int_data);
    free(float_data);
    
    return 0;
}
