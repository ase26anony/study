/* test_modulo_sched.c
 * 
 * This program creates loops with specific patterns to trigger
 * GCC's modulo scheduler debug output for dependency edges.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization of critical variables */
static volatile int sink;

/* Function with multiple loops exhibiting different dependency patterns */
double compute_loop_patterns(void) {
    /* Pattern 1: Integer carried dependency with arithmetic operations */
    int array1[N];
    int array2[N];
    int result1 = 1;
    
    /* Initialize arrays with non-trivial values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 3) % 17;
        array2[i] = (i * 5) % 23;
    }
    
    /* Loop 1: Simple carried dependency (distance-1) with integer multiply */
    /* This creates: result1 = result1 * array1[i] + array2[i] */
    /* The multiplication creates resource pressure on integer multiply units */
    for (int i = 0; i < N; i++) {
        result1 = result1 * array1[i] + array2[i];
    }
    
    /* Pattern 2: Floating-point carried dependency with high-latency operations */
    double fp_array1[M];
    double fp_array2[M];
    double result2 = 3.14159;
    
    /* Initialize with values that prevent simple optimization */
    for (int i = 0; i < M; i++) {
        fp_array1[i] = sin(i * 0.1) + 2.0;
        fp_array2[i] = cos(i * 0.07) * 1.5;
    }
    
    /* Loop 2: Complex carried dependency chain with FP division and multiplication */
    /* Division is high-latency and creates resource contention */
    for (int i = 0; i < M; i++) {
        /* Cross-iteration dependency: result2 from iteration i used in i+1 */
        result2 = result2 / 2.71828 * fp_array1[i] + fp_array2[i];
        
        /* Additional operation to increase register pressure */
        fp_array1[i] = fp_array1[i] * 0.99;
    }
    
    /* Pattern 3: Nested dependencies with array accesses */
    double arr3[N];
    double sum = 0.0;
    double prod = 1.0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr3[i] = (i % 19) * 0.1 + 0.5;
    }
    
    /* Loop 3: Multiple interdependent carried dependencies */
    /* Creates complex dependency graph for the scheduler */
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency on arr3 */
        arr3[i] = arr3[i-1] * 1.1 + arr3[i] * 0.9;
        
        /* Update sum and prod with cross-dependencies */
        sum = sum + arr3[i];
        prod = prod * (sum + 1.0);
    }
    
    /* Pattern 4: Mixed integer/floating point with conditional */
    int mixed_result = 0;
    double temp = 10.0;
    
    for (int i = 0; i < N; i++) {
        /* High-latency floating point operation */
        temp = temp / 1.23456;
        
        /* Integer operation dependent on FP result */
        mixed_result += (int)(temp * 100) + i;
        
        /* Conditional to prevent simple if-conversion */
        if (mixed_result % 7 == 0) {
            temp = temp * 1.5;
        }
    }
    
    /* Combine all results to prevent dead code elimination */
    double final_result = (double)result1 + result2 + sum + prod + (double)mixed_result + temp;
    
    /* Use volatile sink to ensure computation isn't optimized away */
    sink = (int)final_result;
    
    return final_result;
}

/* Pattern 5: Loop with multiple recurrence chains */
int multi_recurrence(void) {
    int a[N], b[N], c[N];
    int x = 1, y = 2, z = 3;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
        c[i] = i * 5;
    }
    
    /* Loop with three independent carried dependencies */
    /* Each creates a separate recurrence chain */
    for (int i = 1; i < N; i++) {
        /* Three separate distance-1 dependencies */
        x = x * a[i-1] + b[i];
        y = y + c[i] * x;
        z = z * 2 - y;
        
        /* Additional operation mixing all three */
        a[i] = (x + y + z) % 100;
    }
    
    return x + y + z;
}

int main(void) {
    printf("Starting modulo scheduler test patterns...\n");
    
    /* Execute the main computation pattern */
    double result1 = compute_loop_patterns();
    printf("Result from pattern 1-4: %f\n", result1);
    
    /* Execute multi-recurrence pattern */
    int result2 = multi_recurrence();
    printf("Result from multi-recurrence pattern: %d\n", result2);
    
    /* Additional pattern: loop with varying dependency distances */
    {
        int arr[2*N];
        int sum = 0;
        
        for (int i = 0; i < 2*N; i++) {
            arr[i] = i * 3;
        }
        
        /* Loop with potential distance-2 dependencies */
        for (int i = 2; i < 2*N; i++) {
            /* Mix of distance-1 and distance-2 dependencies */
            arr[i] = arr[i-1] + arr[i-2] * 2;
            sum += arr[i];
        }
        
        printf("Result from mixed-distance pattern: %d\n", sum);
        sink = sum;  /* Prevent optimization */
    }
    
    printf("Test patterns completed.\n");
    printf("Check the generated .rtl dump files for modulo scheduler debug output.\n");
    
    return 0;
}
