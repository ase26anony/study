/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
volatile int force_use = 0;
volatile double force_use_d = 0.0;

/* Complex loop with multiple carried dependencies and resource contention */
void complex_loop_with_dependencies(double * restrict a, 
                                    double * restrict b, 
                                    double * restrict c,
                                    int n) {
    /* Loop 1: Simple carried dependency with integer */
    int acc_int = 1;
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: acc_int from iteration i-1 used in iteration i */
        acc_int = acc_int * 3 + i;  /* Integer multiplication - competes for ALU */
        a[i] = a[i-1] * 2.5 + b[i]; /* FP carried dependency + memory access */
    }
    force_use = acc_int;  /* Prevent dead code elimination */
    
    /* Loop 2: Multiple interdependent carried dependencies with FP operations */
    double x = 1.0;
    double y = 2.0;
    for (int i = 1; i < n; i++) {
        /* Complex dependency chain with multiple distance-1 dependencies */
        double t1 = x / 3.14159;     /* FP division - high latency operation */
        double t2 = y * 1.618;       /* FP multiplication */
        x = t1 * t2 + c[i];          /* Mix of operations */
        y = x * 0.5 + y;             /* Recurrence: y depends on previous y */
        
        /* Additional carried dependency through array */
        b[i] = b[i-1] * x + a[i] / y; /* FP division and multiplication */
    }
    force_use_d = x + y;
    
    /* Loop 3: Nested dependencies creating longer critical path */
    double sum = 0.0;
    double prod = 1.0;
    for (int i = 0; i < n; i++) {
        /* Interdependent carried dependencies */
        sum = sum + a[i] * b[i];      /* FP multiply-add */
        prod = prod * (sum + 1.0);    /* Uses sum from same iteration */
        c[i] = sqrt(prod) / 2.0;      /* FP sqrt and division - high latency */
    }
    force_use_d += sum * prod;
}

/* Loop with artificial resource bottlenecks */
void resource_intensive_loop(float * restrict in, 
                             float * restrict out, 
                             int n) {
    /* Initialize with volatile to prevent optimization */
    volatile float temp = 1.0f;
    
    for (int i = 1; i < n; i++) {
        /* Create resource contention: multiple FP divides in dependency chain */
        float div1 = in[i] / 3.0f;           /* FP division unit contention */
        float div2 = temp / 7.0f;            /* Another division, depends on carried value */
        float mul1 = div1 * div2;            /* FP multiplication */
        float div3 = mul1 / 11.0f;           /* Yet another division */
        
        /* Carried dependency through temp */
        temp = div3 * 0.99f + in[i-1];
        
        /* Store result with another operation */
        out[i] = temp * sqrtf(fabsf(in[i])); /* sqrt and fabs compete for FP units */
    }
    
    /* Use result to prevent elimination */
    force_use_d += out[n-1];
}

/* Main function with observable side effects */
int main(void) {
    /* Declare arrays with enough elements to avoid trivial unrolling */
    double a[N], b[N], c[N];
    float in[M], out[M];
    
    /* Initialize arrays in a way that prevents compile-time computation */
    for (int i = 0; i < N; i++) {
        a[i] = (i * 1.1) + 0.5;
        b[i] = (i * 0.7) - 0.3;
        c[i] = sin(i * 0.1);  /* Non-trivial initialization */
    }
    
    for (int i = 0; i < M; i++) {
        in[i] = cos(i * 0.05) * 10.0f;
    }
    
    /* Execute loops that should trigger modulo scheduler */
    complex_loop_with_dependencies(a, b, c, N);
    resource_intensive_loop(in, out, M);
    
    /* Compute final result with observable side effect */
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i];
    }
    for (int i = 0; i < M; i++) {
        final_result += out[i];
    }
    
    /* Print result to ensure loops aren't eliminated */
    printf("Final result: %f\n", final_result);
    
    return 0;
}
