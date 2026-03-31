/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Prevent compiler from optimizing away loops */
static void escape(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Function 1: Basic loop with multiple carried dependencies */
void loop_carried_deps(int *a, int *b, int *c, int n) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int acc1 = 0, acc2 = 0;
    
    /* Multiple distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Two separate accumulators with carried dependencies */
        acc1 = acc1 + b[i] * 3;      /* Distance-1 use of acc1 */
        acc2 = acc2 * 2 + c[i];      /* Distance-1 use of acc2 */
        
        /* Cross-iteration array access */
        c[i] = b[i-1] + acc1;        /* Uses b from previous iteration */
    }
    
    /* Force compiler to consider values */
    escape(&sum);
    escape(&acc1);
    escape(&acc2);
}

/* Function 2: Nested loops with inner loop carried dependency */
void nested_loop_deps(int *x, int *y, int *z, int n, int m) {
    volatile int outer_acc = 0;
    
    for (int j = 0; j < m; j++) {
        int inner_acc = 0;
        
        /* Inner loop with multiple carried dependencies */
        for (int i = 1; i < n; i++) {
            /* Complex distance-1 pattern */
            x[i] = x[i-1] + y[i] * z[i];
            inner_acc = inner_acc + x[i] * 7;
            y[i] = y[i-1] * 3 - inner_acc;
        }
        
        outer_acc += inner_acc;
        
        /* Small unrolled section to create scheduling complexity */
        for (int i = 0; i < 4; i++) {
            z[i + j] = z[i + j] * 2 + outer_acc;
        }
    }
    
    escape(&outer_acc);
}

/* Function 3: Multiple interleaved carried dependencies */
void interleaved_deps(int *arr1, int *arr2, int *arr3, int n) {
    int dep1 = arr1[0];
    int dep2 = arr2[0];
    int dep3 = arr3[0];
    
    /* Three separate carried dependency chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: Simple accumulator */
        dep1 = dep1 * 5 + arr1[i];
        
        /* Chain 2: More complex with array access */
        dep2 = (dep2 + arr2[i-1]) * 3;  /* Explicit distance-1 use */
        
        /* Chain 3: Mixed operations */
        dep3 = (dep3 - arr3[i]) * 2 + dep1;
        
        /* Cross-dependencies between chains */
        arr1[i] = dep2 + dep3;
        arr2[i] = dep1 - dep2;
        arr3[i] = dep3 * dep1;
    }
    
    /* Use results to prevent elimination */
    arr1[n-1] = dep1 + dep2 + dep3;
}

/* Function 4: Loop with unknown trip count (parameter) */
void variable_loop(int *data, int *coeffs, int start, int end) {
    volatile int result = 0;
    int carry = data[start];
    
    /* Loop with parameterized bounds - prevents unrolling */
    for (int i = start + 1; i < end; i++) {
        /* Multiple operations with carried dependency */
        int temp = carry * coeffs[i];
        result = result + temp;
        carry = data[i] - result;
        data[i] = temp + carry * 2;
        
        /* Additional distance-1 use */
        coeffs[i] = coeffs[i-1] + carry;
    }
    
    escape(&result);
    data[end-1] = result;
}

/* Function 5: Complex loop with if-conversion potential */
void conditional_loop(int *a, int *b, int *c, int n) {
    int sum_even = 0;
    int sum_odd = 0;
    
    for (int i = 1; i < n; i++) {
        /* Carried dependency with conditional */
        if (i % 2 == 0) {
            sum_even = sum_even + a[i] * b[i-1];  /* Distance-1 use */
            a[i] = sum_even - c[i];
        } else {
            sum_odd = sum_odd * 2 + b[i];
            a[i] = sum_odd + c[i-1];  /* Distance-1 use */
        }
        
        /* Unconditional carried dependency */
        c[i] = c[i-1] + a[i];
    }
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    int *array3 = (int*)malloc(SIZE * sizeof(int));
    int *array4 = (int*)malloc(SIZE * sizeof(int));
    int *array5 = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 - 2;
        array3[i] = i * 7 + 3;
        array4[i] = i * 11 - 5;
        array5[i] = i * 13 + 7;
    }
    
    /* Call all test functions with different parameters */
    loop_carried_deps(array1, array2, array3, SIZE);
    nested_loop_deps(array2, array3, array4, SIZE/2, 10);
    interleaved_deps(array3, array4, array5, SIZE);
    variable_loop(array4, array5, 10, SIZE-10);
    conditional_loop(array5, array1, array2, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i] + array5[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return 0;
}
