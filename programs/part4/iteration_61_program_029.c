/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be kept and prevent optimizations */
static volatile int sink;
static volatile double dsink;

/* Complex loop with multiple carried dependencies and resource contention */
void test_modulo_scheduling(void) {
    /* Loop 1: Integer carried dependency with multiplication bottleneck */
    int array1[N], array2[N];
    int sum = 1;  /* Start with non-zero to create dependency chain */
    
    /* Initialize arrays with non-trivial values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 3) % 17 + 1;
        array2[i] = (i * 7) % 13 + 1;
    }
    
    /* Main test loop 1: Multiple carried dependencies
     * sum depends on previous iteration (distance-1)
     * Multiplications compete for integer multiplier units */
    for (int i = 1; i < N; i++) {
        /* Carried dependency: sum from iteration i-1 used in iteration i */
        sum = sum * array1[i] + array2[i];
        
        /* Additional operation with carried dependency on sum */
        array1[i] = (array1[i-1] + sum) % 31;
        
        /* Another multiplication to pressure resources */
        array2[i] = array2[i-1] * 3;
    }
    
    sink = sum;  /* Prevent dead code elimination */
    
    /* Loop 2: Floating-point with high-latency operations */
    double fp_array[M];
    double fp_sum = 3.14159;
    double fp_prod = 2.71828;
    
    /* Initialize with values that create dependencies */
    for (int i = 0; i < M; i++) {
        fp_array[i] = sin(i * 0.1) + 2.0;
    }
    
    /* Complex floating-point loop with multiple carried dependencies
     * Division and multiplication compete for FP units */
    for (int i = 1; i < M; i++) {
        /* High-latency division operation */
        fp_sum = fp_sum / 3.14159 + fp_array[i];
        
        /* Carried dependency chain with multiplication */
        fp_prod = fp_prod * (fp_sum + 1.0);
        
        /* Another carried dependency */
        fp_array[i] = fp_array[i-1] * 0.99 + fp_sum;
        
        /* Additional FP operation to increase II */
        fp_sum = fp_sum * cos(i * 0.01);
    }
    
    dsink = fp_sum + fp_prod;
    
    /* Loop 3: Mixed integer/float with complex dependency pattern */
    int mixed_array[N];
    double mixed_fp = 1.0;
    int mixed_int = 42;
    
    for (int i = 1; i < N; i++) {
        /* Interleaved dependencies across iterations */
        mixed_int = mixed_int * 2 + mixed_array[i-1];
        mixed_fp = mixed_fp * 1.5 + sin(mixed_int * 0.01);
        mixed_array[i] = (int)(mixed_fp * 100) % 256;
        
        /* Additional operations to create more edges in DDG */
        if (i > 2) {
            mixed_array[i] += mixed_array[i-2];  /* Distance-2 dependency */
        }
    }
    
    sink = mixed_int;
    dsink = mixed_fp;
}

/* Loop with explicit distance-1 dependencies for scheduler analysis */
void test_distance1_deps(void) {
    int a[N], b[N], c[N];
    int result = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
        c[i] = i * 5;
    }
    
    /* Clear classic distance-1 carried dependency
     * a[i] depends on a[i-1] from previous iteration */
    a[0] = 1;
    for (int i = 1; i < N; i++) {
        /* Explicit distance-1 dependency */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Another carried dependency chain */
        b[i] = b[i-1] + a[i] * 2;
        
        /* Multiple uses to create more edges in dependency graph */
        result += a[i] * b[i];
    }
    
    sink = result;
    
    /* Nested loop with resource contention */
    double matrix[16][16];
    double vec[16];
    double res[16] = {0};
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        vec[i] = i * 0.1;
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i + j) * 0.05;
        }
    }
    
    /* Matrix-vector multiply with carried dependencies */
    for (int iter = 0; iter < 8; iter++) {
        for (int i = 0; i < 16; i++) {
            double sum = 0;
            for (int j = 0; j < 16; j++) {
                /* FP multiply-add chain */
                sum += matrix[i][j] * vec[j];
            }
            /* Carried dependency through vec */
            vec[i] = vec[i] * 0.9 + sum * 0.1;
            res[i] += sum;
        }
    }
    
    /* Use results */
    double total = 0;
    for (int i = 0; i < 16; i++) {
        total += res[i];
    }
    dsink = total;
}

int main(void) {
    printf("Testing modulo scheduling coverage...\n");
    
    /* Run both test functions */
    test_modulo_scheduling();
    test_distance1_deps();
    
    /* Print results to ensure side effects */
    printf("Integer sink: %d\n", sink);
    printf("Double sink: %f\n", dsink);
    
    /* Additional computation to keep optimizer interested */
    volatile int check = sink + (int)dsink;
    printf("Final check: %d\n", check);
    
    return 0;
}
