/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size, int* counter) {
    volatile double sum = 0.0;  /* Prevent optimization of final result */
    double temp1, temp2;
    double* ptr = arr;
    int i, j;
    
    /* Multiple carried dependencies in nested loops */
    for (i = 1; i < size; i++) {
        /* TRUE DEPENDENCY (RAW): Read-after-write */
        arr[i] = arr[i-1] * 1.5 + (double)i;
        
        /* ANTI-DEPENDENCY (WAR): Write-after-read */
        temp1 = arr[i];              /* Read */
        arr[i] = temp1 * 0.75;       /* Write to same location */
        
        /* OUTPUT DEPENDENCY (WAW): Write-after-write */
        arr[i] = arr[i] + 2.0;       /* First write */
        arr[i] = arr[i] * 3.0;       /* Second write to same location */
        
        /* Memory aliasing with pointer arithmetic */
        ptr = &arr[i];
        *ptr = *ptr + *(ptr-1);      /* Potential memory dependency */
        
        /* Mixed data types in dependency chain */
        (*counter) += (int)arr[i];   /* Float to int conversion */
        
        /* Inner loop with carried dependency */
        for (j = 0; j < 4; j++) {
            /* Complex expression with multiple operations */
            temp2 = (double)(*counter) / (j + 1);
            arr[i] = arr[i] + temp2 * 0.1;
            
            /* Pointer-based memory dependency */
            indices[i] = indices[i-1] + j;
        }
        
        /* Floating-point operation with different latency */
        sum += arr[i] / (i + 1.0);   /* FP division has higher latency */
    }
    
    return sum;
}

/* Another function with different dependency patterns */
__attribute__((noinline, noclone))
int integer_loop(int* data, float* fdata, int size) {
    int i;
    volatile int result = 0;
    int temp_int;
    float temp_float;
    
    for (i = 1; i < size; i++) {
        /* Integer true dependency chain */
        data[i] = data[i-1] * 2 + i;
        
        /* Cross-type dependency: int -> float */
        temp_float = (float)data[i];
        fdata[i] = fdata[i-1] + temp_float;
        
        /* Float -> int dependency */
        temp_int = (int)fdata[i];
        data[i] = data[i] + temp_int;
        
        /* Memory dependency through pointer aliasing */
        int* alias = &data[i % 2];  /* Creates ambiguous dependencies */
        *alias = *alias + 1;
        
        result += data[i];
    }
    
    return result;
}

/* Function with loop-invariant code motion opportunities */
__attribute__((noinline, noclone))
double mixed_dependencies(double* a, double* b, int* c, int n) {
    double sum = 0.0;
    double inv = 1.0 / n;  /* Loop-invariant */
    int i;
    
    for (i = 1; i < n; i++) {
        /* Multiple interleaved dependencies */
        double t1 = a[i-1] * b[i];      /* RAW on a[i-1] */
        double t2 = t1 + (double)c[i];  /* RAW on t1 */
        a[i] = t2 * inv;                /* WAW on a[i], RAW on t2 */
        
        /* Anti-dependency with reuse */
        double old_b = b[i];            /* Read b[i] */
        b[i] = old_b * 0.99;            /* Write b[i] - WAR */
        
        /* Output dependency */
        c[i] = i * 2;                   /* First write */
        c[i] = c[i] + 1;                /* Second write - WAW */
        
        sum += a[i] + b[i] + c[i];
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 256;
    double arr[SIZE];
    float farr[SIZE];
    int indices[SIZE];
    int data[SIZE];
    int counter = 0;
    int i;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        arr[i] = (double)(i % 10);
        farr[i] = (float)(i % 5);
        indices[i] = i;
        data[i] = i * 2;
    }
    
    /* Call functions with complex dependency patterns */
    volatile double result1 = compute_loop(arr, indices, SIZE, &counter);
    volatile int result2 = integer_loop(data, farr, SIZE);
    volatile double result3 = mixed_dependencies(arr, arr + SIZE/2, indices, SIZE/2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    printf("Counter: %d\n", counter);
    
    return 0;
}
