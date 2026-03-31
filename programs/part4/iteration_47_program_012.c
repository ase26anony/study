#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
static void __attribute__((noinline, noipa)) 
test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i - 1] + b[i];
    }
}

static void __attribute__((noinline, noipa))
test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within the same loop */
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int t = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = t * 2 + i;
        
        /* WAW: Conditional second write to arr[i] */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to same location */
        }
        
        /* Use t to prevent dead code elimination */
        arr[i] += (t & 1);
    }
}

static void __attribute__((noinline, noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i - 2] * 3 + i;
    }
}

static void __attribute__((noinline, noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    /* Complex dependencies with multiple arrays */
    for (int i = 1; i < n; i++) {
        /* RAW between x and y */
        y[i] = x[i - 1] + 5;
        
        /* RAW between y and z */
        z[i] = y[i] * 2;
        
        /* WAR: Read x[i] then write to it */
        int tmp = x[i];
        x[i] = z[i] + tmp;
        
        /* WAW on x through conditional */
        if (i % 4 == 0) {
            x[i] = x[i] - 10;  /* Second write to x[i] */
        }
    }
}

/* Function to create anti-dependencies */
static void __attribute__((noinline, noipa))
test_anti_deps(int *p, int *q, int n) {
    for (int i = 0; i < n - 1; i++) {
        /* Anti-dependency (WAR): write after read */
        int val = p[i];      /* Read */
        p[i] = q[i] + 1;     /* Write to same location */
        q[i] = val;          /* Use the read value */
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation to prevent constant propagation */
    int *a = (int *)malloc(size * sizeof(int));
    int *b = (int *)malloc(size * sizeof(int));
    int *c = (int *)malloc(size * sizeof(int));
    int *d = (int *)malloc(size * sizeof(int));
    int *e = (int *)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !d || !e) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        e[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(a, b, size);
    test_mixed_deps(c, size);
    test_distance_2(d, size);
    test_complex_deps(a, b, e, size);
    test_anti_deps(c, d, size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + e[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    return 0;
}
