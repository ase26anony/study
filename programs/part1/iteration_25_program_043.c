/* test_ddg.c - Program to trigger GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile access functions */
static volatile int vol_var;
static void __attribute__((noinline)) volatile_write(int x) { vol_var = x; }
static int __attribute__((noinline)) volatile_read(void) { return vol_var; }

/* Target function with carefully constructed loop dependencies */
void __attribute__((noinline)) 
process(int *restrict a, int *restrict b, int *restrict c, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_read();
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] (loop-carried, distance=1) */
        a[i] = a[i-1] + b[i] + init_val;
        
        /* ANTI (WAR) dependency: c reads a[i] before it's overwritten */
        int temp = a[i] * 2;          /* Read a[i] */
        a[i] = temp + get_value();    /* Write a[i] - creates WAR with previous read */
        
        /* OUTPUT (WAW) dependency: multiple writes to b[i] */
        b[i] = c[i] + i;              /* First write to b[i] */
        b[i] = b[i] * 3;              /* Second write to b[i] - creates WAW */
        
        /* Another FLOW dependency with different distance */
        c[i] = c[i-2] + a[i];         /* Distance=2 flow dependency */
        
        /* Memory dependency with variant index */
        int idx = i % 10;
        b[idx] = a[i] + c[idx];       /* Creates complex memory dependencies */
        
        /* Volatile operation to prevent reordering */
        volatile_write(i);
    }
}

/* Alternative simpler version that focuses on core dependencies */
void __attribute__((noinline))
process_simple(int *a, int *b, int n) {
    int i;
    
    /* Initialize arrays */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Main computation loop with clear dependencies */
    for (i = 2; i < n - 2; i++) {
        /* Loop-carried FLOW dependency (distance=1) */
        int t1 = a[i-1] + b[i];       /* Read a[i-1] */
        a[i] = t1 * 2;                /* Write a[i] - flow dep from iteration i-1 */
        
        /* Loop-carried FLOW dependency (distance=2) */
        int t2 = b[i-2] + a[i];       /* Read b[i-2] */
        b[i] = t2 / 3;                /* Write b[i] - flow dep from iteration i-2 */
        
        /* ANTI dependency within same iteration */
        int t3 = a[i] + 5;            /* Read a[i] */
        a[i] = b[i] - 2;              /* Write a[i] - anti dep with previous read */
        
        /* OUTPUT dependency */
        b[i] = t3 * 4;                /* Write b[i] - output dep with previous write */
        
        /* Another FLOW to create more edges */
        a[i+1] = a[i] + b[i];         /* Flow to next iteration */
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line or volatile to prevent constant folding */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
    } else {
        /* Volatile prevents compile-time determination */
        volatile int vn = 1000;
        n = vn;
    }
    
    /* Allocate arrays with dynamic size */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
        c[i] = (i * 7) % 100;
    }
    
    /* Call the processing function */
    process(a, b, c, n);
    
    /* Also call simpler version */
    process_simple(a, b, n);
    
    /* Compute checksum to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Checksum: %lld\n", sum);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
