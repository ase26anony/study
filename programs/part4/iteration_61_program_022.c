/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define ITERS 128

/* Force variables to be preserved through optimization */
static volatile int sink __attribute__((used));

/* Complex loop with multiple carried dependencies */
double complex_recurrence(double *arr1, double *arr2, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with high-latency operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum + arr1[i] * arr2[i];
        
        /* Another distance-1 dependency with floating division/multiplication */
        x = x / 3.14159 * arr1[i] + sin(arr2[i]);
        
        /* Cross dependency between sum and prod */
        prod = prod * (sum + 1.0) / (x + 0.5);
        
        /* Additional arithmetic to create resource pressure */
        arr1[i] = arr1[i-1] * 0.99 + arr2[i] * 1.01;
    }
    
    return sum + prod + x;
}

/* Integer loop with carried dependency and mixed operations */
int integer_recurrence(int *a, int *b, int n) {
    int result = 0;
    int acc = 1;
    
    /* Loop with distance-1 dependency and integer multiplication */
    for (int i = 1; i < n; i++) {
        /* Carried dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + i;
        
        /* Another carried dependency chain */
        acc = acc * 2 + a[i];
        
        /* Mix with bit operations for variety */
        result ^= (acc >> 1) & 0xFF;
    }
    
    return result + acc;
}

/* Loop with nested dependencies and array accesses */
float nested_dependencies(float *data, int n) {
    float temp1 = 1.0f;
    float temp2 = 2.0f;
    float temp3 = 3.0f;
    
    for (int i = 1; i < n; i++) {
        /* Chain of dependencies across iterations */
        temp1 = temp1 * 1.1f + data[i];
        temp2 = temp2 / 1.2f + temp1 * 0.5f;
        temp3 = temp3 * 0.9f + temp2 * 0.8f;
        
        /* Store result with carried dependency */
        data[i] = data[i-1] + temp3;
    }
    
    return temp1 + temp2 + temp3;
}

int main(void) {
    /* Declare and initialize arrays with volatile elements 
       to prevent dead code elimination */
    volatile double arr1[N];
    volatile double arr2[N];
    volatile int int_arr1[N];
    volatile int int_arr2[N];
    volatile float float_arr[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i % 7) * 1.5;
        arr2[i] = (i % 5) * 2.3;
        int_arr1[i] = i * 3;
        int_arr2[i] = i * 2 + 1;
        float_arr[i] = i * 0.7f;
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    
    /* Execute loops multiple times to ensure they're not optimized away */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Call each loop function */
        result1 += complex_recurrence((double*)&arr1, (double*)&arr2, N);
        result2 += integer_recurrence((int*)&int_arr1, (int*)&int_arr2, N);
        result3 += nested_dependencies((float*)&float_arr, N);
        
        /* Modify inputs slightly each iteration */
        arr1[iter % N] += 0.1;
        int_arr1[iter % N] += 1;
        float_arr[iter % N] += 0.05f;
    }
    
    /* Force all results to be used to prevent optimization */
    sink = (int)result1 + result2 + (int)result3;
    
    /* Print results to create observable side effect */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    
    return 0;
}
