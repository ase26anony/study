/* test_ddg.c - Program to trigger GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
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
        /* 1. FLOW DEPENDENCY (RAW) with distance 0 (within iteration) */
        int temp = a[i-1] + get_value();  /* Read a[i-1] */
        b[i] = temp * 2;                  /* Write b[i] */
        c[i] = b[i] + init_val;           /* Read b[i] just written - flow dep */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_before = d[i];           /* Read d[i] */
        a[i] = read_before + i;           /* Write a[i] - anti dep on next stmt */
        d[i] = get_value() + c[i];        /* Write d[i] - anti: write after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        int tmp = get_value();
        c[i] = tmp + 1;                   /* Write c[i] - output dep on line 28 */
        c[i] = c[i] * 3;                  /* Write c[i] again - output dep */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance > 0) */
        a[i] = a[i-1] + b[i-1];           /* Flow dep from iteration i-1 to i */
    }
}

/* Another variant with different patterns */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, 
              float *restrict z, int n) {
    int i;
    float acc = 0.0f;
    
    /* Loop with reduction and multiple dependencies */
    for (i = 0; i < n; i++) {
        /* Complex flow dependencies */
        float t1 = x[i] * y[i];
        float t2 = t1 + z[i];      /* Flow dep on t1 */
        z[i] = t2 * 0.5f;          /* Flow dep on t2, output dep on z */
        
        /* Anti dependency pattern */
        float old_z = z[i];        /* Read z[i] */
        x[i] = old_z + i;          /* Write x[i] */
        z[i] = x[i] * y[i];        /* Write z[i] - anti dep on line 55 */
        
        /* Loop-carried flow dependency */
        acc = acc + t1;            /* Reduction: flow dep across iterations */
        y[i] = acc;                /* Flow dep on acc */
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate arrays with volatile to prevent optimization */
    volatile int size_volatile = n;
    int array_size = size_volatile;
    
    int *a = (int*)malloc(array_size * sizeof(int));
    int *b = (int*)malloc(array_size * sizeof(int));
    int *c = (int*)malloc(array_size * sizeof(int));
    int *d = (int*)malloc(array_size * sizeof(int));
    
    float *x = (float*)malloc(array_size * sizeof(float));
    float *y = (float*)malloc(array_size * sizeof(float));
    float *z = (float*)malloc(array_size * sizeof(float));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < array_size; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
        c[i] = (i * 5) % 100;
        d[i] = (i * 7) % 100;
        x[i] = (float)(i % 100) * 0.1f;
        y[i] = (float)((i * 2) % 100) * 0.1f;
        z[i] = (float)((i * 3) % 100) * 0.1f;
    }
    
    /* Call the loops multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, d, array_size);
        process_loop2(x, y, z, array_size);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum_int = 0;
    float sum_float = 0.0f;
    for (int i = 0; i < array_size; i++) {
        sum_int += a[i] + b[i] + c[i] + d[i];
        sum_float += x[i] + y[i] + z[i];
    }
    
    /* Use results (prevents optimization) */
    printf("Checksums: int=%d, float=%.2f\n", sum_int, sum_float);
    
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
