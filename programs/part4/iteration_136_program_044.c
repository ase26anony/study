/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdint.h>
#include <stdlib.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop to trigger modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int vol_var1 = seed;
    volatile float vol_var2 = seed * 0.5f;
    volatile double vol_var3 = seed * 0.25;
    
    /* Cross-iteration dependency with recurrence */
    int acc_int = vol_var1;
    float acc_float = vol_var2;
    double acc_double = vol_var3;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;
        
        /* Mixed latency operations */
        switch (i & 3) {
            case 0:
                /* Integer operations (low latency) */
                acc_int += (a[i] << 2) | 1;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (acc_int));
                break;
                
            case 1:
                /* Floating point (higher latency) */
                acc_float = acc_float * b[i] + 1.5f;
                /* Memory operation */
                vol_var2 = acc_float;
                break;
                
            case 2:
                /* Mixed operations */
                acc_double = acc_double * c[i] + 2.5;
                /* Another asm to force register use */
                asm volatile ("" : "+r" (i) : "r" (acc_double));
                break;
                
            case 3:
                /* Complex integer with memory */
                acc_int = (acc_int ^ a[i]) * 3;
                /* Volatile store/load creates memory dependence */
                vol_var1 = acc_int;
                acc_int = vol_var1 + 1;
                break;
        }
        
        /* Additional control flow within loop */
        if (i & 1) {
            /* Nested loop to create pressure */
            int temp = 0;
            for (int j = 0; j < 3; j++) {
                temp += a[(i + j) % n];
            }
            acc_int += temp;
        } else {
            /* Different path */
            acc_float += b[i % n] * 0.75f;
        }
        
        /* Pointer chasing to create memory dependencies */
        static int chain[100];
        static int chain_idx = 0;
        chain[chain_idx] = acc_int;
        chain_idx = (chain_idx + 1) % 100;
        acc_int ^= chain[(chain_idx + 50) % 100];
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Another function with irreducible control flow using computed goto */
__attribute__((optimize("no-unroll-loops")))
void irreducible_flow(int *arr, int n) {
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    int state = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        goto *labels[state];
        
        L0:
            sum += arr[i] * 2;
            state = (state + 1) % 5;
            continue;
            
        L1:
            sum -= arr[i] / 3;
            state = (state + 2) % 5;
            continue;
            
        L2:
            sum ^= arr[i] << 1;
            state = (state + 3) % 5;
            continue;
            
        L3:
            sum |= arr[i];
            state = (state + 4) % 5;
            continue;
            
        L4:
            sum &= arr[i] | 0xFF;
            state = (state + 5) % 5;
            continue;
    }
    
    global_sink += sum;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    /* Use command line or random for runtime values */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate arrays with dynamic size */
    int *int_arr = (int*)malloc(n * sizeof(int));
    float *float_arr = (float*)malloc(n * sizeof(float));
    double *double_arr = (double*)malloc(n * sizeof(double));
    
    if (!int_arr || !float_arr || !double_arr) {
        return 1;
    }
    
    /* Initialize with pattern (not compile-time constant) */
    for (int i = 0; i < n; i++) {
        int_arr[i] = (i * 37 + 123) % 7919;
        float_arr[i] = (float)((i * 51 + 456) % 7919) * 0.001f;
        double_arr[i] = (double)((i * 73 + 789) % 7919) * 0.0001;
    }
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(int_arr, float_arr, double_arr, n, iter * 1000);
        irreducible_flow(int_arr, n);
    }
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return global_sink != 0 ? 0 : 1;
}
