#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimizations that might eliminate dependencies */
#define NOOPT __attribute__((noinline, noipa))

/* Test 1: Simple RAW (Read-After-Write) dependency with distance 1 */
static void NOOPT test_raw_dep(int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];  // RAW: a[i-1] read, then a[i] written
    }
}

/* Test 2: Mixed dependencies (RAW, WAR, WAW) in same loop */
static void NOOPT test_mixed_deps(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        /* RAW dependency */
        int temp = arr[i];           // Read arr[i]
        
        /* WAR dependency - overwrite the location we just read */
        arr[i] = temp * 2 + i;       // Write to arr[i] after reading it
        
        /* WAW dependency - conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;   // Second write to arr[i]
        }
    }
}

/* Test 3: Loop-carried dependency with distance 2 */
static void NOOPT test_distance_2(int *a, int n) {
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;  // Distance 2: uses value from 2 iterations back
    }
}

/* Test 4: Complex dependencies with multiple arrays */
static void NOOPT test_complex_deps(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n; i++) {
        /* Multiple RAW dependencies */
        int x = a[i-1] + b[i];      // RAW on a[i-1]
        int y = b[i-1] * c[i];      // RAW on b[i-1]
        
        /* WAR on b[i] */
        b[i] = x + y;
        
        /* WAW on a[i] with conditional */
        a[i] = x * 2;
        if (y > 0) {
            a[i] = a[i] + y;        // Second write to a[i]
        }
        
        /* Cross-iteration dependency with varying distance */
        if (i % 4 == 0 && i >= 3) {
            c[i] = c[i-3] + a[i];   // Distance 3
        }
    }
}

/* Test 5: Prevent induction variable optimization */
static void NOOPT test_induction_var(int *a, int *b, int start, int end, int step) {
    for (int i = start; i < end; i += step) {
        /* Non-unit stride to prevent optimization */
        a[i] = a[i - step] + b[i];  // RAW with variable distance
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant for the optimizer */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values to prevent constant folding */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(a, b, size);
    test_mixed_deps(c, size);
    test_distance_2(a, size);
    test_complex_deps(a, b, c, size);
    
    /* Variable stride to prevent optimization */
    int stride = (argc > 2) ? atoi(argv[2]) : 2;
    if (stride < 1) stride = 2;
    test_induction_var(b, c, stride, size, stride);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
