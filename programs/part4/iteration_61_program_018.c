/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Force variables to be kept and not optimized away */
static volatile int g_sink = 0;
static volatile double d_sink = 0.0;

/* Function with multiple loops exhibiting different dependency patterns */
void compute_loops(void) {
    /* Loop 1: Integer carried dependency with arithmetic operations */
    int array1[N];
    int array2[N];
    int result1 = 1;
    
    /* Initialize arrays with non-trivial values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 3 + 7) % 19;
        array2[i] = (i * 5 + 11) % 23;
    }
    
    /* Loop with distance-1 dependency and mixed operations */
    /* a[i] depends on a[i-1] creating carried dependency */
    for (int i = 1; i < N; i++) {
        /* Multiple operations with different latencies */
        int temp = array1[i-1] * 3;      /* Multiplication - higher latency */
        array1[i] = temp + array2[i];    /* Addition - lower latency */
        
        /* Another carried dependency chain */
        result1 = result1 * 2 + array1[i]; /* Recurrence with multiplication */
    }
    
    g_sink = result1;  /* Prevent dead code elimination */
    
    /* Loop 2: Floating-point operations with complex dependencies */
    double fp_array[M];
    double fp_result = 1.0;
    double fp_accum = 0.5;
    
    /* Initialize with values that create non-trivial dependencies */
    for (int i = 0; i < M; i++) {
        fp_array[i] = 1.0 + (i % 7) * 0.1;
    }
    
    /* Loop with floating-point carried dependencies and high-latency ops */
    for (int i = 1; i < M; i++) {
        /* Division - high latency operation */
        double div_result = fp_accum / 3.14159;
        
        /* Multiplication - another high latency operation */
        double mul_result = div_result * fp_array[i];
        
        /* Carried dependency: fp_accum from previous iteration */
        fp_accum = mul_result + fp_array[i-1] * 0.5;
        
        /* Another independent carried dependency chain */
        fp_result = fp_result * (1.0 + fp_accum * 0.01);
        
        /* Additional arithmetic to create more scheduling pressure */
        fp_array[i] = fp_array[i] + fp_accum * 0.3;
    }
    
    d_sink = fp_result + fp_accum;  /* Prevent dead code elimination */
    
    /* Loop 3: Nested dependencies and multiple recurrence patterns */
    int matrix[16][16];
    int sum = 0;
    int prod = 1;
    
    /* Initialize matrix */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 17 + j * 13) % 29;
        }
    }
    
    /* Loop with multiple interleaved carried dependencies */
    for (int i = 1; i < 16; i++) {
        for (int j = 1; j < 16; j++) {
            /* Horizontal dependency (distance 1 in j dimension) */
            int left_val = matrix[i][j-1] * 2;
            
            /* Vertical dependency (distance 1 in i dimension) */
            int top_val = matrix[i-1][j] / 3;
            
            /* Diagonal dependency (distance 1 in both dimensions) */
            int diag_val = matrix[i-1][j-1] + 5;
            
            /* Combine with multiplication (higher latency) */
            matrix[i][j] = (left_val + top_val) * diag_val;
            
            /* Two independent carried dependency chains */
            sum = sum + matrix[i][j];
            prod = prod * (matrix[i][j] % 7 + 1);
        }
    }
    
    g_sink = sum + prod;  /* Prevent dead code elimination */
}

/* Loop 4: Mixed integer/floating point with artificial resource contention */
void mixed_operations(void) {
    double d_array[64];
    int i_array[64];
    double d_acc = 1.0;
    int i_acc = 0;
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        d_array[i] = (i % 11) * 0.2 + 0.1;
        i_array[i] = (i * 7) % 13;
    }
    
    /* Loop with operations competing for functional units */
    for (int i = 1; i < 64; i++) {
        /* Floating-point division (high latency, specific unit) */
        double div_op = d_acc / 2.71828;
        
        /* Integer multiplication (different unit, but still latency) */
        int mul_op = i_acc * 3;
        
        /* Floating-point multiplication */
        double fmul_op = div_op * d_array[i];
        
        /* Integer addition */
        int add_op = mul_op + i_array[i-1];
        
        /* Carried dependencies update */
        d_acc = fmul_op + d_array[i-1];
        i_acc = add_op % 17;
        
        /* Cross-type operation to create more complexity */
        d_array[i] = d_array[i] + (i_acc * 0.01);
    }
    
    d_sink = d_acc;
    g_sink = i_acc;
}

int main(void) {
    printf("Starting modulo scheduling test...\n");
    
    compute_loops();
    mixed_operations();
    
    /* Use results to ensure they're not optimized away */
    printf("Results: int=%d, double=%.6f\n", g_sink, d_sink);
    
    return 0;
}
