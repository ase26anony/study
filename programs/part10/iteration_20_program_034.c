/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency loops */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* int_arr, float* float_arr, int size) {
    double sum = 0.0;
    double temp = 0.0;
    int i;
    
    /* Initialize arrays with some values */
    for (i = 0; i < size; i++) {
        arr[i] = (double)i * 1.5;
        int_arr[i] = i * 2;
        float_arr[i] = (float)i * 0.5f;
    }
    
    /* 
     * Main loop with multiple carried dependencies:
     * This should create various DDG edges when compiled with optimization
     */
    
    /* 1. True Data Dependency (RAW) with floating point */
    for (i = 1; i < size; i++) {
        arr[i] = arr[i-1] * 1.1 + (double)int_arr[i];  /* RAW on arr */
    }
    
    /* 2. Anti-dependency (WAR) with mixed types */
    for (i = 0; i < size - 1; i++) {
        temp = arr[i];                    /* Read arr[i] */
        arr[i] = (double)float_arr[i] * temp;  /* Write arr[i] - WAR */
        float_arr[i] = (float)(temp * 0.5);     /* Different data type */
    }
    
    /* 3. Output Dependency (WAW) */
    for (i = 0; i < size; i++) {
        arr[i] = (double)int_arr[i] * 2.0;      /* First write */
        /* Some computation in between */
        int_arr[i] = int_arr[i] + i;
        arr[i] = arr[i] * 3.0;                  /* Second write - WAW */
    }
    
    /* 4. Complex loop with pointer aliasing (memory dependencies) */
    double* ptr1 = arr;
    double* ptr2 = &arr[size/2];  /* Potential aliasing */
    
    for (i = 0; i < size/2; i++) {
        /* Memory dependencies through potentially aliased pointers */
        *ptr1 = *ptr1 + *ptr2 * 0.5;
        ptr1++;
        ptr2--;
        
        /* Integer dependency chain */
        int_arr[i] = int_arr[i] + int_arr[i+1] * 2;
    }
    
    /* 5. Nested loop with carried dependencies */
    for (i = 0; i < size; i++) {
        double acc = arr[i];
        for (int j = 0; j < 4; j++) {
            /* Inner loop with dependency */
            acc = acc * 1.05 + (double)j;
            float_arr[j] = (float)acc;  /* Different data type */
        }
        arr[i] = acc;
        
        /* Control dependency */
        if (int_arr[i] > 100) {
            arr[i] = arr[i] * 2.0;
        }
    }
    
    /* Compute final sum to prevent elimination */
    for (i = 0; i < size; i++) {
        sum += arr[i] + (double)int_arr[i] + (double)float_arr[i % 4];
    }
    
    return sum;
}

/* Another function with different patterns */
__attribute__((noinline, noclone))
int integer_dependency_chain(int* data, int size) {
    int result = 0;
    
    /* Long dependency chain */
    for (int i = 1; i < size; i++) {
        data[i] = data[i-1] * 3 + data[i] * 2 - 7;
    }
    
    /* Multiple accumulating variables with dependencies */
    int a = data[0], b = data[1], c = data[2];
    for (int i = 3; i < size; i++) {
        a = b + c;
        b = c * 2;
        c = data[i] - a;
        data[i] = a + b + c;
    }
    
    for (int i = 0; i < size; i++) {
        result += data[i];
    }
    
    return result;
}

int main(void) {
    const int SIZE = 256;
    
    /* Allocate arrays with different alignments */
    double* double_arr = (double*)aligned_alloc(64, SIZE * sizeof(double));
    int* int_arr = (int*)aligned_alloc(64, SIZE * sizeof(int));
    float* float_arr = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!double_arr || !int_arr || !float_arr) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Volatile to prevent over-optimization */
    volatile double result1;
    volatile int result2;
    
    /* Call functions with dependency patterns */
    result1 = compute_loop(double_arr, int_arr, float_arr, SIZE);
    result2 = integer_dependency_chain(int_arr, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %f\n", (double)result1);
    printf("Result 2: %d\n", result2);
    
    /* Additional volatile store */
    volatile int check = (result1 > 0 && result2 > 0) ? 1 : 0;
    
    free(double_arr);
    free(int_arr);
    free(float_arr);
    
    return check;
}
