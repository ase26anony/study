#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimizations that might eliminate dependencies */
static __attribute__((noinline, noipa)) 
void test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

static __attribute__((noinline, noipa))
void test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within the same loop iteration */
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int temp = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: Conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
        
        /* Use temp to prevent dead code elimination */
        arr[i] += (temp & 1);
    }
}

static __attribute__((noinline, noipa))
void test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3 + i;
    }
}

static __attribute__((noinline, noipa))
void test_complex_deps(int *x, int *y, int *z, int n) {
    /* Complex pattern with multiple dependency types */
    for (int i = 1; i < n; i++) {
        /* RAW on x with distance 1 */
        int t1 = x[i-1];
        
        /* RAW on y with distance 1 */
        int t2 = y[i-1];
        
        /* WAR: Write to x[i] after reading x[i-1] */
        x[i] = t1 + t2;
        
        /* WAW: Two writes to y[i] */
        y[i] = x[i] * 2;
        
        /* Conditional second write creating WAW */
        if (i % 4 == 0) {
            y[i] = y[i] + z[i];
        }
        
        /* Anti-dependency (WAR) between z and x */
        z[i] = x[i] - 5;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant size to prevent loop unrolling */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    /* Dynamic allocation prevents constant propagation */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *arr3 = (int*)malloc(size * sizeof(int));
    int *arr4 = (int*)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values to create data dependencies */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(arr1, arr2, size);
    test_mixed_deps(arr3, size);
    test_distance_2(arr4, size);
    
    /* Complex test with multiple arrays */
    test_complex_deps(arr1, arr2, arr3, size);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i];
        /* Add some computation to create more dependencies */
        arr1[i] = (arr1[i] * 13) ^ checksum;
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
