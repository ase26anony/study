#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
__attribute__((noinline, noipa))
static void test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

__attribute__((noinline, noipa))
static void test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within a single loop */
    for (int i = 0; i < n; i++) {
        /* RAW dependency: read arr[i] */
        int temp = arr[i];
        
        /* WAR dependency: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW dependency: conditional second write to arr[i] */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to same location */
        }
    }
}

__attribute__((noinline, noipa))
static void test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

__attribute__((noinline, noipa))
static void test_complex_deps(int *x, int *y, int *z, int n) {
    /* Complex pattern with multiple dependency types and distances */
    for (int i = 3; i < n; i++) {
        /* Multiple RAW dependencies with different distances */
        int t1 = x[i-1];  /* distance 1 */
        int t2 = y[i-2];  /* distance 2 */
        int t3 = z[i-3];  /* distance 3 */
        
        /* WAR: read then write to same array */
        x[i] = t1 + t2;
        
        /* Another WAR with y */
        y[i] = t3 * 2;
        
        /* WAW: two writes to z[i] */
        z[i] = i * 5;
        if (i % 4 == 0) {
            z[i] = z[i] + t1;  /* Second write */
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
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
    
    /* Initialize with non-constant values */
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
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
