/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be preserved through optimization */
static volatile int sink __attribute__((used));

/* Complex loop with multiple carried dependencies and resource contention */
double complex_loop_with_dependencies(double * restrict a, 
                                      double * restrict b, 
                                      double * restrict c, 
                                      int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with floating-point operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] * 1.01 + b[i];
        
        /* Another distance-1 dependency with high-latency operation */
        x = x / 3.1415926535 * c[i];  /* Division and multiplication compete for FPU */
        
        /* Cross-dependency between different variables */
        sum = sum + a[i] * x;  /* Multiplication latency */
        prod = prod * (sum + 1.0);  /* Another multiplication */
        
        /* Artificial resource bottleneck: multiple FP operations */
        b[i] = (b[i-1] + 2.71828) / x;  /* Division creates backpressure */
    }
    
    return sum + prod + x + a[n-1];
}

/* Integer loop with carried dependencies and integer multiplication */
int integer_loop_with_dependencies(int * restrict arr1, 
                                   int * restrict arr2, 
                                   int n) {
    int acc1 = arr1[0];
    int acc2 = arr2[0];
    int temp = 1;
    
    /* Loop with multiple carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency chain */
        acc1 = acc1 * 7 + arr1[i];  /* Integer multiplication */
        
        /* Another distance-1 dependency */
        acc2 = (acc2 + arr2[i]) * 3;  /* More integer operations */
        
        /* Cross-iteration dependency with different distance */
        temp = temp * acc1 + acc2;  /* Complex dependency web */
        
        /* Create anti-dependencies by reusing variables */
        arr1[i-1] = temp;  /* Store creates memory dependencies */
    }
    
    return acc1 + acc2 + temp;
}

/* Mixed-type loop to stress the scheduler */
float mixed_operations(float * restrict fa, 
                       double * restrict db, 
                       int * restrict ic, 
                       int n) {
    float fsum = fa[0];
    double dprod = db[0];
    
    for (int i = 1; i < n; i++) {
        /* Type conversions add complexity */
        fsum = fsum * 1.1f + (float)db[i];
        
        /* High-latency double precision operations */
        dprod = dprod / 2.718281828459045 * fa[i];
        
        /* Integer operation in the mix */
        ic[i] = ic[i-1] * 2 + (int)fsum;
        
        /* More resource contention */
        fa[i] = sqrtf(fabsf(fsum));  /* FP sqrt operation */
        db[i] = dprod * 0.99;
    }
    
    return fsum * (float)dprod;
}

int main(void) {
    /* Declare and initialize arrays with volatile elements 
       to prevent dead code elimination */
    double a[N], b[N], c[N];
    int arr1[M], arr2[M];
    float fa[N];
    double db[N];
    int ic[N];
    
    /* Initialize with non-trivial patterns */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.5;
        b[i] = sin(i * 0.1);
        c[i] = cos(i * 0.05);
        fa[i] = i * 0.25f;
        db[i] = i * 0.125;
        ic[i] = i * 2;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = complex_loop_with_dependencies(a, b, c, N);
    int result2 = integer_loop_with_dependencies(arr1, arr2, M);
    float result3 = mixed_operations(fa, db, ic, N);
    
    /* Compute final result to ensure loops aren't eliminated */
    double final_result = result1 + result2 + result3;
    
    /* Use volatile sink to prevent dead code elimination */
    sink = (int)final_result;
    
    /* Print result to create observable side effect */
    printf("Final result: %f\n", final_result);
    
    /* Additional computation to keep variables alive */
    printf("Checksum: a[%d]=%f, arr1[%d]=%d\n", 
           N-1, a[N-1], M-1, arr1[M-1]);
    
    return 0;
}
