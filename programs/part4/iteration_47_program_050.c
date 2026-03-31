#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimizations that might eliminate dependencies */
static void __attribute__((noinline, noipa)) 
test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

static void __attribute__((noinline, noipa))
test_mixed_deps(int *arr, int n) {
    /* Contains RAW, WAR, and WAW dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW: read arr[i] */
        int temp = arr[i];
        
        /* WAR: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 1;  /* Another write to arr[i] */
        }
        
        /* Use temp to prevent dead code elimination */
        arr[i] += (temp & 1);
    }
}

static void __attribute__((noinline, noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3 + i;
    }
}

/* Additional test with volatile to ensure memory dependencies */
static void __attribute__((noinline, noipa))
test_volatile_deps(volatile int *a, int *b, int n) {
    /* Volatile ensures compiler can't reorder or eliminate accesses */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

/* Test with pointer aliasing to confuse the optimizer */
static void __attribute__((noinline, noipa))
test_aliasing_deps(int *p, int *q, int n) {
    /* p and q might alias, creating additional dependencies */
    for (int i = 1; i < n; i++) {
        p[i] = q[i-1] + p[i-1];
        q[i] = p[i] * 2;
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant for the optimizer */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure minimum size */
    
    /* Allocate dynamic memory to prevent constant propagation */
    int *array1 = (int*)malloc(size * sizeof(int));
    int *array2 = (int*)malloc(size * sizeof(int));
    int *array3 = (int*)malloc(size * sizeof(int));
    volatile int *varray = (volatile int*)malloc(size * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !varray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        varray[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(array1, array2, size);
    test_mixed_deps(array3, size);
    test_distance_2(array1, size);
    test_volatile_deps(varray, array2, size);
    test_aliasing_deps(array1, array2, size);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i] + array3[i] + varray[i];
        checksum &= 0xFFFF;  /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free((int*)varray);
    
    return 0;
}
