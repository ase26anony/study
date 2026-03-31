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
void loop_with_carried_deps(int *result) {
    volatile double x = 3.14159;
    volatile double y = 2.71828;
    double array[N];
    double temp[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array[i] = i * 0.5;
        temp[i] = i * 0.25;
    }
    
    /* Loop 1: Simple carried dependency with integer */
    int sum = 0;
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: sum from previous iteration used */
        sum = sum + (int)(array[i] * 2.0);
        /* Another carried dependency on the same variable */
        sum = sum * 1.01 + i;
        array[i] = array[i-1] * 1.5 + sum * 0.1;  /* Distance-1 array dependency */
    }
    
    /* Loop 2: Complex FP operations with resource contention */
    double acc = 1.0;
    for (int i = 1; i < M; i++) {
        /* High-latency FP operations that compete for resources */
        double t1 = acc / 3.141592653589793;  /* Division - high latency */
        double t2 = t1 * temp[i];             /* Multiplication */
        double t3 = sin(t2) * cos(acc);       /* Transcendental functions */
        
        /* Multiple carried dependencies creating complex schedule */
        acc = t3 + array[i] * 0.5;
        temp[i] = temp[i-1] * acc + 0.1;      /* Another distance-1 dependency */
        
        /* Additional operations to increase register pressure */
        array[i] = array[i] * 0.99 + temp[i] * 0.01;
    }
    
    /* Loop 3: Nested dependencies and recurrence */
    double prod = 1.0;
    double prev = 0.5;
    for (int i = 0; i < N; i++) {
        /* Complex dependency chain */
        double val = prev * array[i];
        prod = prod * (val + 1.0);            /* Product accumulation */
        prev = sin(val) * 0.5 + cos(prod) * 0.5;
        
        /* Integer operations mixed with FP */
        sum += (int)(prod * 100);
    }
    
    /* Use results to prevent dead code elimination */
    *result = sum + (int)(acc * 100) + (int)(prod * 1000);
    sink = *result;  /* Volatile write ensures loop isn't optimized away */
}

/* Another loop with different pattern */
void loop_with_memory_deps(double *output) {
    volatile double data[N];
    volatile double coeff[N];
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        data[i] = i * 0.1;
        coeff[i] = 1.0 + i * 0.01;
    }
    
    /* Loop with multiple interleaved carried dependencies */
    double sum1 = 0.0, sum2 = 0.0;
    for (int i = 2; i < N; i++) {
        /* Multiple distance-1 dependencies */
        double t1 = data[i-1] * coeff[i];     /* Uses data[i-1] */
        double t2 = data[i-2] / coeff[i-1];   /* Uses data[i-2] - distance 2 */
        
        /* Cross-dependency between accumulators */
        sum1 = sum1 + t1 * t2;
        sum2 = sum2 * 0.99 + sum1 * 0.01;     /* Depends on sum1 */
        
        /* Update with carried dependency */
        data[i] = data[i-1] * 0.9 + sum2 * 0.1;
    }
    
    /* Final computation with result */
    *output = sum1 + sum2;
    sink = (int)(*output * 1000);
}

int main() {
    int result_int;
    double result_double;
    
    printf("Starting modulo scheduling test loops...\n");
    
    /* Execute loops that should trigger modulo scheduler */
    loop_with_carried_deps(&result_int);
    loop_with_memory_deps(&result_double);
    
    /* Use results to ensure loops execute */
    printf("Integer result: %d\n", result_int);
    printf("Double result: %f\n", result_double);
    
    /* Additional computation to keep everything alive */
    volatile double final = result_double * result_int;
    printf("Final: %f\n", final);
    
    return 0;
}
