/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
static volatile int g_sink = 0;
static volatile double d_sink = 0.0;

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(void) {
    /* Array with volatile elements to prevent optimization */
    volatile double arr1[N];
    volatile double arr2[N];
    volatile double arr3[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.5;
        arr3[i] = i * 0.5;
    }
    
    /* Loop 1: Simple carried dependency with integer */
    volatile int sum = 0;
    volatile int prev = 1;
    
    /* Distance-1 dependency: prev from iteration i used in iteration i+1 */
    for (int i = 0; i < M; i++) {
        /* Carried dependency: prev_i depends on prev_{i-1} */
        prev = prev * 3 + i;  /* Integer multiplication creates latency */
        sum += prev;          /* Another operation using prev */
        
        /* Additional operation to create resource pressure */
        arr1[i] = arr1[i] / 3.14159 * 2.71828;  /* FP division and multiplication */
    }
    
    /* Loop 2: Complex FP carried dependency chain */
    volatile double x = 1.0;
    volatile double y = 2.0;
    volatile double z = 3.0;
    
    /* Multiple interdependent carried dependencies */
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: x_i depends on x_{i-1} */
        x = x / 1.234567 + arr2[i] * 0.987654;  /* FP division and multiplication */
        
        /* Another carried dependency with distance 1 */
        y = y * 1.111111 - arr1[i-1];  /* Uses value from previous iteration */
        
        /* Cross-dependency between x and y */
        z = z + x * y / 5.4321;  /* Complex FP operation chain */
        
        /* Array with carried dependency */
        arr3[i] = arr3[i-1] * 0.888888 + arr3[i] * 1.222222;
    }
    
    /* Loop 3: Nested dependencies with high latency operations */
    volatile double acc1 = 0.0;
    volatile double acc2 = 1.0;
    volatile double acc3 = 0.5;
    
    for (int i = 0; i < M; i++) {
        /* Multiple high-latency operations in dependency chain */
        acc1 = acc1 + sin(arr1[i] * 3.14159 / 180.0);  /* FP trig function */
        acc2 = acc2 * cos(acc1 * 0.0174533);           /* Dependent on acc1 */
        acc3 = acc3 / (1.0 + fabs(acc2));              /* FP division dependent on acc2 */
        
        /* Integer carried dependency mixed with FP */
        sum = sum + (int)(acc3 * 1000);
    }
    
    /* Store results to volatile sinks to prevent dead code elimination */
    g_sink = sum;
    d_sink = x + y + z + acc1 + acc2 + acc3;
}

/* Another loop with different pattern to increase scheduling complexity */
void loop_with_resource_conflict(void) {
    volatile double a[N], b[N], c[N];
    volatile double result = 0.0;
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.234;
        b[i] = i * 2.345;
        c[i] = i * 3.456;
    }
    
    /* Loop with multiple carried dependencies creating resource conflicts */
    volatile double t1 = 1.0, t2 = 2.0, t3 = 3.0;
    
    for (int i = 1; i < N-1; i++) {
        /* Chain of dependent FP operations competing for FP units */
        t1 = t1 * a[i] / b[i-1];      /* Uses b from previous iteration */
        t2 = t2 + t1 * c[i];          /* Dependent on t1 */
        t3 = t3 - t2 / a[i+1];        /* Dependent on t2, uses future a */
        
        /* Additional independent but high-latency operations */
        a[i] = sqrt(fabs(t3)) * 2.0;  /* FP sqrt */
        b[i] = log(fabs(t1) + 1.0);   /* FP log */
        
        result += t1 + t2 + t3;
    }
    
    d_sink += result;
}

/* Main function to ensure all loops are executed */
int main(void) {
    double total = 0.0;
    
    /* Execute loops multiple times to increase scheduling opportunities */
    for (int iter = 0; iter < 3; iter++) {
        loop_with_carried_deps();
        loop_with_resource_conflict();
        
        /* Use results in computation */
        total += g_sink * 0.001 + d_sink;
        
        /* Print intermediate result to create observable side effect */
        printf("Iteration %d: partial result = %f\n", iter, total);
    }
    
    /* Final print to ensure no dead code elimination */
    printf("Final result: %f\n", total);
    
    return (int)(total * 1000) % 100;
}
