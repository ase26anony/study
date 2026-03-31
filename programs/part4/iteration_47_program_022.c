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
    /* Mixed dependencies within loop body */
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int temp = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: Conditional second write to same location */
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

/* Additional test with volatile to prevent optimization */
static void __attribute__((noinline, noipa))
test_volatile_dep(volatile int *v, int *a, int n) {
    /* Use volatile to create memory barriers */
    for (int i = 1; i < n; i++) {
        *v = a[i-1];      /* Write to volatile */
        a[i] = *v + i;    /* Read from volatile */
    }
}

/* Complex loop with multiple dependencies */
static void __attribute__((noinline, noipa))
test_complex_deps(int *a, int *b, int *c, int n) {
    for (int i = 3; i < n; i++) {
        /* Multiple RAW dependencies */
        int t1 = a[i-1];
        int t2 = b[i-2];
        int t3 = c[i-3];
        
        /* WAR: Write to a[i-1] after reading it */
        a[i-1] = t1 + t2;
        
        /* WAW: Multiple writes to b[i] */
        b[i] = t2 * 2;
        if (t3 > 0) {
            b[i] = t3 + 5;  /* Second write */
        }
        
        /* Loop-carried with distance 3 */
        c[i] = c[i-3] + a[i-1];
    }
}

int main(int argc, char **argv) {
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
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
        arr5[i] = rand() % 100;
    }
    
    volatile int volatile_var = 0;
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(arr1, arr2, size);
    test_mixed_deps(arr3, size);
    test_distance_2(arr4, size);
    test_volatile_dep(&volatile_var, arr5, size);
    test_complex_deps(arr1, arr2, arr3, size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i] + arr5[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return 0;
}
