#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
static void __attribute__((noinline, noipa)) 
test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];  /* RAW: a[i-1] read, then a[i] written next iteration */
    }
}

static void __attribute__((noinline, noipa))
test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within same loop iteration */
    for (int i = 0; i < n; i++) {
        /* RAW dependency */
        int temp = arr[i];           /* Read arr[i] */
        
        /* WAR dependency - overwrite the location we just read */
        arr[i] = temp * 2 + i;       /* Write to arr[i] after reading it */
        
        /* WAW dependency - potential second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;   /* Second write to arr[i] in same iteration */
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
    /* Complex pattern with multiple interleaved dependencies */
    for (int i = 1; i < n; i++) {
        /* Chain of RAW dependencies */
        int t1 = x[i-1] + y[i];
        int t2 = t1 * z[i];
        
        /* WAR: read x[i] then modify it */
        int old_x = x[i];
        x[i] = t2 + old_x;
        
        /* WAW: conditional second write to y[i] */
        y[i] = t1;
        if (i % 4 == 0) {
            y[i] = y[i] * 2;  /* Second write to y[i] */
        }
        
        /* Another distance-2 dependency */
        if (i >= 2) {
            z[i] = z[i-2] + 1;
        }
    }
}

/* Volatile and global variables to prevent optimization */
volatile int global_seed = 42;
int global_array[10];

static void __attribute__((noinline, noipa))
test_volatile_and_global(int *arr, int n) {
    /* Mix with volatile/global to create memory barriers */
    for (int i = 1; i < n; i++) {
        /* Read from volatile global */
        int seed = global_seed;
        
        /* RAW with global array */
        global_array[i % 10] = arr[i-1] + seed;
        
        /* WAR: read then write arr[i] */
        int tmp = arr[i];
        arr[i] = global_array[(i-1) % 10] + tmp;
        
        /* Potential WAW through global */
        if (arr[i] > 100) {
            global_array[i % 10] = arr[i] / 2;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation to prevent constant propagation */
    int *array1 = (int*)malloc(size * sizeof(int));
    int *array2 = (int*)malloc(size * sizeof(int));
    int *array3 = (int*)malloc(size * sizeof(int));
    int *array4 = (int*)malloc(size * sizeof(int));
    int *array5 = (int*)malloc(size * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4 || !array5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        array4[i] = rand() % 100;
        array5[i] = rand() % 100;
    }
    
    /* Call test functions to create DDG edges */
    test_raw_dep(array1, array2, size);
    test_mixed_deps(array3, size);
    test_distance_2(array4, size);
    test_complex_deps(array1, array2, array5, size);
    test_volatile_and_global(array3, size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i] + array5[i];
        checksum += global_array[i % 10];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return 0;
}
