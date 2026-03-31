/* test_ddg.c - Program to trigger GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x ^ 0x55;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_source;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i-1] + init_val;      /* Read a[i-1] */
        b[i] = temp * 2;                   /* Write b[i] */
        c[i] = b[i] + get_value(i);        /* Read b[i] just written */
        
        /* 2. ANTI DEPENDENCY (WAR) - Write after read */
        int read_val = d[i];               /* Read d[i] */
        d[i] = read_val * 3 + i;           /* Write d[i] after reading */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Write after write */
        a[i] = temp + read_val;            /* First write to a[i] */
        a[i] = a[i] * 2 - i;               /* Second write to a[i] */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance > 0) */
        /* This creates edge with distance = 1 */
        c[i] = c[i-1] + b[i];              /* Read c[i-1] from previous iteration */
        
        /* 5. Additional memory dependencies with variant indices */
        int idx = i & 0xF;                 /* Prevent simple analysis */
        b[idx] = a[i] + c[idx];            /* Complex memory dependency */
    }
}

/* Another function with different pattern to increase coverage */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, int n) {
    int i;
    volatile float v = 1.5f;
    
    /* Loop with floating-point dependencies */
    for (i = 2; i < n; i++) {
        /* Flow dependency chain */
        float t1 = x[i-1] * v;
        float t2 = t1 + y[i-2];            /* Distance = 2 dependency */
        x[i] = t2 * 0.5f;
        
        /* Anti dependency */
        float old = y[i];
        y[i] = old * x[i] + (float)i;
        
        /* Output dependency */
        float tmp = x[i] * 2.0f;
        tmp = tmp * tmp;                   /* WAW on tmp */
        x[i] = tmp;
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Allocate arrays with volatile to prevent optimization */
    volatile int alloc_size = n;
    int *a = (int*)malloc(alloc_size * sizeof(int));
    int *b = (int*)malloc(alloc_size * sizeof(int));
    int *c = (int*)malloc(alloc_size * sizeof(int));
    int *d = (int*)malloc(alloc_size * sizeof(int));
    float *x = (float*)malloc(alloc_size * sizeof(float));
    float *y = (float*)malloc(alloc_size * sizeof(float));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;
        b[i] = i * 5;
        c[i] = i * 7;
        d[i] = i * 11;
        x[i] = (float)i * 0.1f;
        y[i] = (float)i * 0.3f;
    }
    
    /* Call the loops multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    float fsum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
        fsum += x[i] + y[i];
    }
    
    /* Use results to prevent optimization */
    printf("Checksums: %d, %.2f\n", sum, fsum);
    
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
