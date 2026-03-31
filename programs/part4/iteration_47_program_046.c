#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimizations that might eliminate dependencies */
#define NOOPT __attribute__((noinline, noipa))

/* Test 1: Simple RAW dependency with distance 1 */
static void NOOPT test_raw_dep(int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];  /* RAW: a[i-1] read, then a[i] written */
    }
}

/* Test 2: Mixed dependencies (RAW, WAR, WAW) */
static void NOOPT test_mixed_deps(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        /* RAW dependency: read arr[i] */
        int temp = arr[i];
        
        /* WAR dependency: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW dependency: conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
    }
}

/* Test 3: Distance-2 dependency */
static void NOOPT test_distance_2(int *a, int n) {
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;  /* RAW with distance 2 */
    }
}

/* Test 4: Complex dependencies with multiple arrays */
static void NOOPT test_complex_deps(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n; i++) {
        /* Multiple RAW dependencies */
        int t1 = a[i-1];
        int t2 = b[i];
        
        /* WAR: read b[i], then write to it */
        b[i] = t1 + t2;
        
        /* WAW: two writes to c[i] */
        c[i] = t1 * 2;
        if (i % 4 == 0) {
            c[i] = c[i] + t2;  /* Second write */
        }
        
        /* RAW with variable distance */
        a[i] = c[i] + (i > 3 ? a[i-3] : 0);
    }
}

/* Test 5: Prevent induction variable optimization */
static void NOOPT test_induction_var(int *a, int *b, int n, int step) {
    for (int i = 0; i < n; i += step) {
        /* Use non-constant step to prevent optimization */
        if (i > 0) {
            a[i] = a[i-step] + b[i];  /* RAW with variable distance */
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant for the optimizer */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *arr3 = (int*)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values to prevent optimization */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(arr1, arr2, size);
    test_mixed_deps(arr3, size);
    test_distance_2(arr1, size);
    test_complex_deps(arr1, arr2, arr3, size);
    test_induction_var(arr2, arr3, size, 1 + (rand() % 3));
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
