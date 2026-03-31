/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double complex_recurrence(double *a, double *b, double *c, int n) {
    double x = 1.0;
    double y = 0.5;
    double z = 2.0;
    
    /* Loop 1: Multiple carried dependencies with floating-point operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: x[i] depends on x[i-1] */
        x = x / 3.14159 * b[i] + a[i-1];
        
        /* Distance-1 dependency: y[i] depends on y[i-1] and x[i] */
        y = y * 1.23456 + x / 2.71828;
        
        /* Distance-1 dependency: z[i] depends on z[i-1] and y[i] */
        z = z / 1.41421 + y * c[i];
        
        /* Cross-dependency between different variables */
        a[i] = x + y - z;
    }
    
    return x + y + z;
}

/* Integer loop with carried dependency and resource contention */
int integer_recurrence(int *arr1, int *arr2, int n) {
    int sum = arr1[0];
    int prod = 1;
    int acc = 0;
    
    /* Loop 2: Integer operations with carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum + arr1[i] * arr2[i];
        
        /* Distance-1 dependency: prod[i] depends on prod[i-1] and sum[i] */
        prod = prod * (sum % 17 + 1);
        
        /* Nested dependency chain */
        acc = acc + (prod >> 2) - (sum & 0xFF);
        
        /* Store result to create memory dependency */
        arr1[i] = acc;
    }
    
    return sum + prod + acc;
}

/* Loop with mixed operations and array recurrence */
float mixed_operations(float *data, int *indices, int n) {
    float result = data[0];
    float temp = 0.0f;
    
    /* Loop 3: Array-based recurrence with mixed operations */
    for (int i = 1; i < n; i++) {
        /* Array-based distance-1 dependency */
        float prev = data[i-1];
        
        /* High-latency floating-point operations */
        float div_result = prev / 7.0f;
        float mul_result = div_result * 3.0f;
        
        /* Trigonometric function for additional latency */
        float trig = sinf(mul_result * 0.1f);
        
        /* Combined result with carried dependency */
        result = result * 0.99f + trig * data[i];
        
        /* Store with dependency */
        data[i] = result + indices[i];
        
        /* Additional computation to increase register pressure */
        temp = temp + cosf(result) * 0.5f;
    }
    
    return result + temp;
}

int main(void) {
    /* Declare and initialize arrays */
    double a[N], b[N], c[N];
    int arr1[M], arr2[M];
    float fdata[N];
    int indices[N];
    
    /* Initialize with non-trivial patterns */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = sin(i * 0.05);
        c[i] = cos(i * 0.03);
        fdata[i] = i * 0.25f;
        indices[i] = i % 13;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    
    /* Execute loops to create RTL patterns */
    double result1 = complex_recurrence(a, b, c, N);
    int result2 = integer_recurrence(arr1, arr2, M);
    float result3 = mixed_operations(fdata, indices, N);
    
    /* Force results to be used to prevent dead code elimination */
    sink = (int)result1 + result2 + (int)result3;
    
    /* Print results to ensure side effects */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    printf("Sink: %d\n", sink);
    
    return 0;
}
