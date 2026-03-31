#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and IPA to preserve dependencies */
static void __attribute__((noinline, noipa)) 
test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];  /* RAW: a[i-1] read, then a[i] written next iteration */
    }
}

static void __attribute__((noinline, noipa))
test_mixed_deps(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        /* RAW dependency */
        int temp = arr[i];          /* Read arr[i] */
        
        /* WAR dependency - overwrite the location we just read */
        arr[i] = temp * 2 + i;      /* Write to arr[i] after reading it */
        
        /* WAW dependency - conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] in same iteration */
        }
    }
}

static void __attribute__((noinline, noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;  /* Distance 2: use value from i-2 */
    }
}

static void __attribute__((noinline, noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    /* Multiple interleaved dependencies */
    for (int i = 1; i < n; i++) {
        /* Chain of RAW dependencies */
        int t1 = x[i-1] + y[i];
        int t2 = t1 * 2;
        x[i] = t2 + z[i];
        
        /* Anti-dependency (WAR) with previous iteration */
        y[i-1] = x[i] - 5;
        
        /* Output dependency (WAW) - conditional */
        if (i % 4 == 0) {
            z[i] = x[i] * 2;
        } else {
            z[i] = x[i] / 2;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant for optimizer */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values to prevent optimization */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(a, b, size);
    test_mixed_deps(c, size);
    test_distance_2(d, size);
    test_complex_deps(a, b, c, size);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
        checksum &= 0xFFFF;  /* Prevent overflow issues */
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
