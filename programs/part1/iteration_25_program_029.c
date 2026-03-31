/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with complex data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile read to prevent dead code elimination */
    int init_val = volatile_source;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n - 1; i++) {
        /* 1. FLOW dependency (RAW): a[i] depends on b[i] */
        int temp = b[i] + init_val;
        
        /* 2. OUTPUT dependency (WAW): a[i] written twice */
        a[i] = temp * 2;
        
        /* 3. ANTI dependency (WAR): c[i] read before write */
        int old_c = c[i];
        
        /* 4. Another FLOW dependency: c[i] depends on a[i] */
        c[i] = a[i] + old_c;
        
        /* 5. Loop-carried FLOW dependency with distance 1 */
        /* d[i] depends on d[i-1] from previous iteration */
        d[i] = d[i-1] + a[i];
        
        /* 6. OUTPUT dependency with loop-carried element */
        a[i] = c[i] - b[i];  // Second write to a[i] - WAW with line above
        
        /* 7. ANTI dependency with loop-carried element */
        /* Read b[i+1] before potentially writing it next iteration */
        int next_b = b[i+1];
        
        /* 8. Complex memory dependency pattern */
        /* Flow dependency through array with variant index */
        b[i] = a[i-1] + next_b + get_value();
    }
}

/* Another loop with different patterns to increase coverage */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, 
              float *restrict z, int m) {
    int j;
    volatile float v = 3.14f;
    
    for (j = 2; j < m; j++) {
        /* Multiple interleaved dependencies */
        float t1 = x[j-1];           /* Flow from previous iteration */
        float t2 = y[j] + v;         /* Anti dependency potential */
        x[j] = t1 * t2;              /* Write x[j] */
        y[j] = x[j] / 2.0f;          /* Flow: y[j] depends on x[j] */
        z[j] = z[j-2] + y[j];        /* Flow with distance 2 */
        
        /* Create output dependency */
        x[j] = z[j] - 1.0f;          /* Second write to x[j] - WAW */
        
        /* Anti dependency through function call */
        float tmp = get_value() * 0.5f;
        y[j] = tmp + x[j];           /* WAR with earlier y[j] read */
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    int m = 500;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    if (argc > 2) {
        m = atoi(argv[2]);
        if (m < 10) m = 500;
    }
    
    /* Allocate arrays with alignment hint */
    int *a = __builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    int *b = __builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    int *c = __builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    int *d = __builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    
    float *x = __builtin_assume_aligned(malloc(m * sizeof(float)), 16);
    float *y = __builtin_assume_aligned(malloc(m * sizeof(float)), 16);
    float *z = __builtin_assume_aligned(malloc(m * sizeof(float)), 16);
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i % 7;
        b[i] = (i * 3) % 11;
        c[i] = (i + 5) % 13;
        d[i] = i;
    }
    
    for (int j = 0; j < m; j++) {
        x[j] = (j % 5) * 1.5f;
        y[j] = (j % 3) * 2.5f;
        z[j] = j * 0.7f;
    }
    
    /* Call the loops multiple times to ensure they're not dead code */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, z, m);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum_int += a[i] + b[i] + c[i] + d[i];
    }
    
    for (int j = 0; j < m; j++) {
        sum_float += x[j] + y[j] + z[j];
    }
    
    printf("Checksums: int=%d float=%.2f\n", sum_int, sum_float);
    
    /* Free memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
