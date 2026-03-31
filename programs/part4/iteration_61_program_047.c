/* test_modulo_sched.c
 * 
 * This program creates loops with specific characteristics to trigger
 * GCC's modulo scheduler debug output for dependency edges.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Prevent optimization of critical variables */
static volatile int sink;

/* Function with multiple loops exhibiting different dependency patterns */
void compute_loops(void) {
    /* Loop 1: Integer carried dependency with arithmetic operations */
    {
        int array1[N];
        int result1 = 1;
        
        /* Initialize array with non-zero values */
        for (int i = 0; i < N; i++) {
            array1[i] = (i % 7) + 1;
        }
        
        /* Core loop with distance-1 carried dependency
         * result1 from iteration i is used in iteration i+1
         * This creates the distance1_uses condition */
        for (int i = 1; i < N; i++) {
            /* Multiple operations to create resource pressure */
            int temp = array1[i] * 3;      /* Integer multiplication */
            result1 = result1 + temp;      /* Carried dependency */
            result1 = result1 / 2;         /* Integer division - high latency */
            array1[i] = result1;
        }
        
        sink = result1;  /* Prevent dead code elimination */
    }
    
    /* Loop 2: Floating-point operations with complex dependency chain */
    {
        double array2[M];
        double x = 1.0;
        double y = 2.0;
        
        /* Initialize with varying values */
        for (int i = 0; i < M; i++) {
            array2[i] = 1.0 + (i % 5) * 0.1;
        }
        
        /* Loop with multiple carried dependencies and FP operations
         * Creates non-zero latencies for the scheduler */
        for (int i = 1; i < M; i++) {
            /* Complex dependency chain with high-latency operations */
            double a = x / 3.14159;        /* FP division - high latency */
            double b = y * array2[i];      /* FP multiplication */
            double c = a * b;              /* FP multiplication */
            
            /* Cross-iteration dependencies */
            x = c + array2[i-1];           /* Distance-1 dependency */
            y = x * 0.99;                  /* Another dependency */
            
            array2[i] = x + y;
        }
        
        /* Use results to prevent optimization */
        sink = (int)(x + y);
    }
    
    /* Loop 3: Mixed operations with nested dependencies */
    {
        int array3[N];
        int sum = 0;
        int prod = 1;
        
        for (int i = 0; i < N; i++) {
            array3[i] = i + 1;
        }
        
        /* Loop with interdependent carried dependencies
         * Forces higher II calculation */
        for (int i = 0; i < N; i++) {
            /* Multiple operations that compete for resources */
            int val1 = array3[i] * 7;      /* Integer multiply */
            int val2 = val1 / 3;           /* Integer divide */
            
            /* Two separate carried dependencies */
            sum = sum + val2;              /* Carried: sum[i] depends on sum[i-1] */
            prod = prod * (sum + 1);       /* Carried: prod[i] depends on prod[i-1] AND sum[i] */
            
            array3[i] = sum + prod;
        }
        
        sink = sum + prod;
    }
}

/* Loop 4: Array recurrence with multiple dependency distances */
void array_recurrence(void) {
    volatile int arr[N+2];
    int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < N+2; i++) {
        arr[i] = i % 10;
    }
    
    /* Complex recurrence pattern
     * arr[i] depends on arr[i-1] AND arr[i-2]
     * Creates multiple dependency edges */
    for (int i = 2; i < N; i++) {
        /* Multiple high-latency operations in dependency chain */
        int temp1 = arr[i-1] * arr[i-2];   /* Distance-1 and distance-2 dependencies */
        int temp2 = temp1 / 5;             /* Integer division */
        int temp3 = temp2 + arr[i];        /* Current iteration value */
        arr[i] = temp3 * 2;                /* Update with multiplication */
        
        result += arr[i];
    }
    
    sink = result;
}

/* Loop 5: Conditional carried dependencies */
void conditional_dependencies(void) {
    int data[N];
    int acc = 1;
    
    for (int i = 0; i < N; i++) {
        data[i] = (i * 3) % 17;
    }
    
    /* Loop with conditional execution paths
     * Still maintains carried dependencies */
    for (int i = 1; i < N; i++) {
        if (data[i] > 8) {
            acc = acc * data[i-1];     /* Carried dependency in one path */
        } else {
            acc = acc + data[i-1];     /* Carried dependency in other path */
        }
        
        /* Additional operations to increase II */
        data[i] = acc / 2;             /* Integer division */
        acc = data[i] * 3;             /* Multiplication */
    }
    
    sink = acc;
}

int main(void) {
    printf("Starting modulo scheduler test...\n");
    
    /* Execute all loops to ensure coverage */
    compute_loops();
    array_recurrence();
    conditional_dependencies();
    
    /* Final computation using sink to prevent optimization */
    int final_result = sink;
    printf("Final result: %d\n", final_result);
    
    return 0;
}
