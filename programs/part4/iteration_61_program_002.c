/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization of critical variables */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double loop1(double *a, double *b, double c) {
    double sum = 0.0;
    double prod = 1.0;
    
    /* Loop with distance-1 dependency on 'sum' and high-latency operations */
    for (int i = 1; i < N; i++) {
        /* Carried dependency: sum[i] depends on sum[i-1] */
        sum = sum + a[i] * b[i];
        
        /* High-latency floating point operations that compete for resources */
        double temp = sum / 3.141592653589793;
        temp = temp * 2.718281828459045;
        
        /* Another carried dependency with multiplication */
        prod = prod * (temp + 1.0);
        
        /* Additional arithmetic to create more pressure */
        a[i] = a[i-1] * c + temp;
    }
    
    return sum + prod;
}

/* Integer loop with complex dependency chain */
int loop2(int *arr1, int *arr2, int k) {
    int acc1 = arr1[0];
    int acc2 = arr2[0];
    
    /* Multiple interleaved carried dependencies */
    for (int i = 1; i < M; i++) {
        /* Distance-1 dependency on acc1 */
        acc1 = acc1 * k + arr1[i];
        
        /* Distance-1 dependency on acc2 with different operation */
        acc2 = (acc2 + arr2[i]) * acc1;
        
        /* Cross-dependency between the two accumulators */
        arr1[i] = (acc1 + acc2) / (i + 1);
        arr2[i] = (acc1 * acc2) % (i + 5);
        
        /* High-latency integer division */
        int div_result = acc1 / (arr2[i-1] + 1);
        acc1 = acc1 ^ div_result;
    }
    
    return acc1 + acc2;
}

/* Loop with nested dependencies and array recurrence */
float loop3(float *x, float *y, float alpha) {
    float result = x[0];
    
    /* Complex recurrence pattern */
    for (int i = 1; i < N; i++) {
        /* Multiple carried dependencies in one expression */
        x[i] = (x[i-1] * alpha + y[i]) / (x[i-1] + 1.0f);
        
        /* Chain of dependent floating-point operations */
        float t1 = x[i] * x[i];
        float t2 = t1 + sqrtf(fabsf(x[i]));
        float t3 = t2 / (y[i-1] + 0.5f);
        
        /* Recurrence with distance 1 */
        y[i] = y[i-1] * t3 + alpha;
        
        result += x[i] + y[i];
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays with volatile elements to prevent optimization */
    double a[N], b[N];
    int arr1[M], arr2[M];
    float x[N], y[N];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = (double)(i % 37) * 0.1;
        b[i] = (double)(i % 23) * 0.2;
        x[i] = (float)(i % 17) * 0.3f;
        y[i] = (float)(i % 29) * 0.4f;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    
    /* Execute loops with carried dependencies */
    double res1 = loop1(a, b, 1.5);
    int res2 = loop2(arr1, arr2, 3);
    float res3 = loop3(x, y, 2.5f);
    
    /* Use results to prevent dead code elimination */
    sink = (int)res1 + res2 + (int)res3;
    
    /* Print results to ensure side effects */
    printf("Results: %f, %d, %f\n", res1, res2, res3);
    printf("Sink: %d\n", sink);
    
    return 0;
}
