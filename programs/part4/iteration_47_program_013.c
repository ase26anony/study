#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
static void __attribute__((noinline, noipa)) 
test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

static void __attribute__((noinline, noipa))
test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within same loop iteration */
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int temp = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: Potential second write to arr[i] */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* WAW dependency */
        }
    }
}

static void __attribute__((noinline, noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

static void __attribute__((noinline, noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    /* Complex dependencies with multiple arrays */
    for (int i = 1; i < n; i++) {
        /* RAW on x with distance 1 */
        x[i] = x[i-1] + y[i];
        
        /* RAW on y with distance 1 */
        y[i] = y[i-1] * 2;
        
        /* WAR: Read z[i] then write to it */
        int tmp = z[i];
        z[i] = tmp + x[i] + y[i];
        
        /* Potential WAW on z[i] */
        if (i % 4 == 0) {
            z[i] = z[i] - 50;
        }
    }
}

/* Use volatile to prevent certain optimizations */
static volatile int global_seed = 42;

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *e = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !e) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with input-dependent values to prevent optimization */
    srand(time(NULL) + global_seed);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        e[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(a, b, n);
    test_mixed_deps(c, n);
    test_distance_2(d, n);
    test_complex_deps(a, e, b, n);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + e[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e);
    
    return 0;
}
