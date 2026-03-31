/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_sink;

/* Target function with complex data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < n - 1; i++) {
        /* 1. FLOW dependency (RAW): a[i] depends on b[i] */
        int temp = b[i] + get_value();
        
        /* 2. ANTI dependency (WAR): c[i] read before write */
        volatile_sink = c[i];  /* Read c[i] */
        
        /* 3. OUTPUT dependency (WAW): a[i] written twice */
        a[i] = temp * 2;
        
        /* 4. Loop-carried FLOW dependency: distance = 1 */
        d[i] = d[i-1] + a[i];
        
        /* 5. ANTI dependency: a[i] read before next iteration's write */
        c[i] = a[i] + volatile_sink;  /* Write c[i] - creates WAR with line 31 */
        
        /* 6. Another OUTPUT dependency on a[i] */
        a[i] = c[i] / 3;  /* Second write to a[i] - creates WAW with line 34 */
        
        /* 7. Loop-carried ANTI dependency */
        b[i+1] = b[i] - 1;  /* Read b[i], write b[i+1] - WAR across iterations */
    }
}

/* Alternate version with memory dependencies through arrays */
void __attribute__((noinline))
process_loop2(int *arr, int n) {
    int i;
    
    /* Loop with array-based dependencies */
    for (i = 2; i < n; i++) {
        /* Flow dependency chain */
        int x = arr[i-2] + arr[i-1];
        
        /* Anti dependency */
        int y = arr[i];
        arr[i] = x + y;
        
        /* Output dependency */
        arr[i-1] = y * 2;
        arr[i-1] = arr[i-1] + 1;  /* Second write - WAW */
        
        /* Loop-carried flow with distance 2 */
        arr[i] = arr[i] + arr[i-2];
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line or environment for variable loop bound */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    } else {
        /* Use volatile to prevent compile-time determination */
        volatile int vn = 1000;
        n = vn;
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *arr = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i;
        arr[i] = i * 5;
    }
    
    /* Call the loops multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(arr, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i] + arr[i];
    }
    
    printf("Checksum: %d\n", sum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr);
    
    return 0;
}
