/* test_modulo_sched.c
 * Designed to trigger DDG edge creation in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms test_modulo_sched.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation and loop unrolling */
extern volatile int GLOBAL_N;

/* Function containing the core loop - marked noinline to prevent inlining */
__attribute__((noinline))
int compute_loop(int *restrict a, int *restrict b, 
                 int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies for DDG construction */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then b[i-1] might be read in next iteration
           if we had: ... = b[i-1] somewhere */
        /* Output dependence: a[i] is written twice in same iteration (simulated) */
        int temp = a[i] - d[i];
        
        /* Create anti-dependence by using b[i-1] in next iteration's computation */
        /* This creates a read-after-write (RAW) dependency through b array */
        b[i] = temp + (i > 1 ? b[i-1] : 0);
        
        /* Additional operation to create more edges in DDG */
        c[i] = (a[i] << 2) | (b[i] & 0xF);
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1];
}

/* Another loop with different dependency pattern */
__attribute__((noinline))
int compute_loop2(int *restrict x, int *restrict y, 
                  int *restrict z, int n) {
    int i;
    
    /* Different dependency pattern to exercise various edge types */
    for (i = 2; i < n; ++i) {
        /* Multiple true dependencies */
        x[i] = x[i-1] + x[i-2];
        
        /* Cross-iteration dependency through y */
        y[i] = y[i-1] * 3 - x[i];
        
        /* Dependency chain within iteration */
        z[i] = (x[i] & 0xFF) + (y[i] >> 4);
        
        /* Create longer dependency chain */
        x[i] = z[i] * 7 + i;
    }
    
    return x[n-1] ^ y[n-1] ^ z[n-1];
}

int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int N = 1024;
    int i, result1, result2;
    
    /* Allocate arrays with restrict qualifiers to help alias analysis */
    int *restrict a = (int*)malloc(N * sizeof(int));
    int *restrict b = (int*)malloc(N * sizeof(int));
    int *restrict c = (int*)malloc(N * sizeof(int));
    int *restrict d = (int*)malloc(N * sizeof(int));
    
    int *restrict x = (int*)malloc(N * sizeof(int));
    int *restrict y = (int*)malloc(N * sizeof(int));
    int *restrict z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values
       This prevents constant propagation */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
        
        x[i] = (i * 101) % 977;
        y[i] = (i * 131) % 971;
        z[i] = (i * 151) % 967;
    }
    
    /* Call the loop functions - these should trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Use results to prevent dead code elimination */
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Final result: %d\n", result1 + result2);
    
    /* Free memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
