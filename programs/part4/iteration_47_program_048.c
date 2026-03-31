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
        /* RAW: read arr[i] */
        int temp = arr[i];
        
        /* WAR: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
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
    /* Complex pattern with multiple dependencies */
    for (int i = 3; i < n; i++) {
        /* Multiple RAW dependencies */
        int t1 = x[i-1];
        int t2 = y[i-2];
        int t3 = z[i-3];
        
        /* WAR: read then write to same array */
        x[i] = t1 + t2;
        
        /* Another RAW with different distance */
        y[i] = x[i-2] + t3;
        
        /* WAW: two writes to z[i] */
        z[i] = x[i] + y[i];
        if (i % 4 == 0) {
            z[i] = z[i] * 2;  /* Second write */
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *arr3 = (int*)malloc(size * sizeof(int));
    int *arr4 = (int*)malloc(size * sizeof(int));
    int *arr5 = (int*)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
        arr5[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(arr1, arr2, size);
    test_mixed_deps(arr3, size);
    test_distance_2(arr4, size);
    test_complex_deps(arr1, arr2, arr5, size);
    
    /* Prevent dead code elimination by computing checksum */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i] + arr5[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return 0;
}
