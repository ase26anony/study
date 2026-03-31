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
double loop1(double *a, double *b, double c) {
    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    
    /* Loop with distance-1 dependencies and high-latency operations */
    for (int i = 1; i < N; i++) {
        /* Multiple carried dependencies creating complex scheduling constraints */
        x = x / 3.14159 * b[i] + a[i-1];  /* Distance-1 dependency on a[i-1] */
        y = y * 2.71828 + x / 1.41421;    /* High-latency FP division */
        z = z + y * x;                     /* FP multiplication dependency */
        a[i] = x + y + z;                  /* Store with carried dependency */
    }
    
    return x + y + z;
}

/* Integer loop with resource contention */
int loop2(int *arr1, int *arr2) {
    int sum = arr1[0];
    int prod = 1;
    
    /* Loop with integer operations competing for ALU/multiplier units */
    for (int i = 1; i < M; i++) {
        /* Multiple interdependent carried dependencies */
        sum = sum + arr1[i] * arr2[i];     /* Integer multiplication */
        prod = prod * (sum + 1);           /* Distance-1 dependency on sum */
        
        /* Additional operations to increase II */
        arr1[i] = (arr1[i] << 3) | (arr2[i] >> 2);  /* Shift operations */
        arr2[i] = arr1[i-1] ^ arr2[i-1];            /* Distance-1 dependency */
    }
    
    return sum + prod;
}

/* Mixed-type loop with complex dependency chain */
float loop3(float *farr, double *darr) {
    float f1 = farr[0];
    double d1 = darr[0];
    
    for (int i = 1; i < N/2; i++) {
        /* Type conversions and mixed operations */
        f1 = f1 * 1.5f + (float)d1;
        d1 = d1 / 2.0 + (double)farr[i-1];  /* Distance-1 dependency */
        
        /* Trigonometric operations (high latency) */
        farr[i] = f1 + sinf((float)i * 0.1f);
        darr[i] = d1 * cos((double)i * 0.05);
    }
    
    return f1 + (float)d1;
}

int main() {
    /* Declare and initialize arrays with volatile elements to prevent optimization */
    double a[N], b[N];
    int arr1[M], arr2[M];
    float farr[N/2];
    double darr[N/2];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (double)(i % 37) * 0.1;
        b[i] = (double)(i % 23) * 0.2;
        if (i < M) {
            arr1[i] = i * 3;
            arr2[i] = i * 5;
        }
        if (i < N/2) {
            farr[i] = (float)i * 0.3f;
            darr[i] = (double)i * 0.4;
        }
    }
    
    /* Execute loops with different dependency patterns */
    double result1 = loop1(a, b, 2.5);
    int result2 = loop2(arr1, arr2);
    float result3 = loop3(farr, darr);
    
    /* Compute final result to ensure loops aren't dead code */
    double final_result = result1 + result2 + result3;
    
    /* Use volatile sink to prevent dead code elimination */
    sink = (int)final_result;
    
    /* Print result to create observable side effect */
    printf("Final result: %f\n", final_result);
    
    return 0;
}
