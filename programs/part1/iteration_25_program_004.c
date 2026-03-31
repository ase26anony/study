/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_sink;

/* Target function with complex loop carrying all dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with opaque values */
    int init_val = get_value();
    a[0] = init_val;
    b[0] = init_val + 1;
    
    /* Main loop with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i-1] + b[i];      /* Read a[i-1] */
        a[i] = temp * 2;               /* Write a[i] - flow from line above */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_before = b[i];        /* Read b[i] */
        b[i] = c[i] + get_value();     /* Write b[i] - anti on line above */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on 'c' */
        c[i] = read_before * 3;        /* Write c[i] - first write */
        c[i] = c[i] + a[i];            /* Write c[i] again - output dependency */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance > 0) */
        d[i] = d[i-1] + temp;          /* Flow from iteration i-1 to i */
        
        /* 5. Complex memory dependency with variant index */
        int idx = (i * 7) % n;
        if (idx > 0) {
            /* Creates anti dependency through memory */
            int val = a[idx];
            a[(idx + 1) % n] = val + b[i];
        }
        
        /* Prevent dead code elimination */
        volatile_sink = a[i] + b[i] + c[i] + d[i];
    }
    
    /* Cross-iteration anti dependency */
    for (i = 0; i < n - 1; i++) {
        /* Read after write from previous iteration's write */
        int x = a[i+1];                /* Read what was written in next iteration */
        a[i] = x + get_value();        /* Write creates anti with future read */
    }
}

/* Another function with different pattern to ensure edge variety */
void __attribute__((noinline))
process_loop2(float *restrict fa, float *restrict fb, int n) {
    int i;
    
    /* Initialize */
    fa[0] = 1.0f;
    fb[0] = 2.0f;
    
    for (i = 1; i < n; i++) {
        /* Multiple flow dependencies in sequence */
        float t1 = fa[i-1] * 0.5f;
        float t2 = t1 + fb[i-1];
        float t3 = t2 * t1;
        
        /* Output dependency */
        fa[i] = t3;
        fa[i] = fa[i] + 1.0f;  /* Second write to same location */
        
        /* Anti dependency through different arrays */
        float old_fb = fb[i];
        fb[i] = t3 * old_fb;
        
        /* Loop-carried with distance 2 */
        if (i >= 2) {
            fa[i] += fa[i-2];
        }
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line or volatile to prevent constant folding */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
        if (n > 10000) n = 10000;  /* Bound for safety */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    float *fa = (float*)malloc(n * sizeof(float));
    float *fb = (float*)malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d || !fa || !fb) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
        fa[i] = (float)i;
        fb[i] = (float)(i * 2);
    }
    
    /* Call the loops multiple times to ensure optimization */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(fa, fb, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum_int += a[i] + b[i] + c[i] + d[i];
        sum_float += fa[i] + fb[i];
    }
    
    /* Use results */
    printf("Checksum: int=%lld float=%f\n", 
           (long long)sum_int, (double)sum_float);
    
    /* Free memory */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb);
    
    return 0;
}
