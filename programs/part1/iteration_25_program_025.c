/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static int volatile_read(void) {
    volatile int v = 42;
    return v;
}

/* Target function with complex data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with opaque values */
    int init_a = get_value();
    int init_b = volatile_read();
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) between statements */
        int temp = a[i-1] + b[i];      /* Read a[i-1] */
        a[i] = temp * 2;               /* Write a[i] - flow from a[i-1] read */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_b = b[i];             /* Read b[i] */
        b[i] = c[i] + init_a;          /* Write b[i] - anti from b[i] read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on array 'c' */
        c[i] = read_b * 3;             /* Write c[i] - first write */
        c[i] = c[i] + d[i];            /* Write c[i] - second write (WAW) */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance > 0) */
        d[i] = d[i-1] + init_b;        /* Flow from d[i-1] with distance=1 */
        
        /* 5. MEMORY DEPENDENCY with variant index */
        int idx = i % 10;
        a[idx] = b[idx] + c[idx];      /* May create memory dependencies */
        
        /* 6. Additional flow dependency chain */
        temp = a[i] + 1;               /* Read a[i] from earlier write */
        b[i] = temp - 2;               /* Write b[i] - flow from a[i] */
    }
}

/* Another loop with different patterns */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, 
              float *restrict z, int n) {
    int i;
    volatile float v = 3.14f;
    
    for (i = 2; i < n - 1; i++) {
        /* Complex dependency web */
        float t1 = x[i-2] + y[i-1];    /* Flow from x[i-2], y[i-1] */
        float t2 = z[i] * v;           /* Volatile prevents optimization */
        
        /* Cross-iteration anti dependency */
        float old_y = y[i];            /* Read y[i] */
        y[i] = t1 + t2;                /* Write y[i] - anti dependency */
        
        /* Output dependency with computation */
        x[i] = old_y * 2.0f;           /* Write x[i] */
        x[i] = x[i] + z[i+1];          /* Re-write x[i] - output dependency */
        
        /* Loop-carried flow with distance 2 */
        z[i] = z[i-2] * 1.5f;          /* Distance = 2 flow dependency */
    }
}

int main(int argc, char *argv[]) {
    int n = 1000;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays with volatile to prevent alignment assumptions */
    volatile int alloc_size = n * sizeof(int);
    int *a = (int*)malloc(alloc_size);
    int *b = (int*)malloc(alloc_size);
    int *c = (int*)malloc(alloc_size);
    int *d = (int*)malloc(alloc_size);
    
    float *x = (float*)malloc(n * sizeof(float));
    float *y = (float*)malloc(n * sizeof(float));
    float *z = (float*)malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i % 37;
        b[i] = (i * 3) % 41;
        c[i] = (i + 5) % 43;
        d[i] = i;
        x[i] = i * 0.1f;
        y[i] = i * 0.2f;
        z[i] = i * 0.3f;
    }
    
    /* Call loops multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, z, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
        sum += (long long)(x[i] + y[i] + z[i]);
    }
    
    printf("Result checksum: %lld\n", sum);
    
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
