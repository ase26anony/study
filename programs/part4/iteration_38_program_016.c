/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    /* This should create distance1_uses = true */
    for (i = 1; i < n; i++) {
        /* Multiple operations to create non-trivial schedule */
        a[i] = a[i-1] * b[i] + c[i];      /* Distance-1 carried dependence */
        sum += a[i] * 3;                   /* Accumulator pattern */
        b[i] = b[i-1] + sum / 2;           /* Another distance-1 dependence */
        
        /* Memory clobber to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loops(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Unroll hint */
        #pragma GCC unroll 2
        for (j = 1; j < m; j++) {
            /* Complex carried dependencies */
            x[j] = x[j-1] * y[j] + i;      /* Distance-1 in inner loop */
            y[j] = y[j-1] + x[j] * 2;      /* Another distance-1 */
            acc += x[j] + y[j];
            
            /* Force dependency preservation */
            asm volatile("" : "+r"(acc) : : "memory");
        }
    }
    return acc;
}

/* Function 3: Multiple interleaved accumulators */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + arr1[i] * 7;         /* Accumulator 1 */
        sum2 = sum2 + arr2[i] * 11;        /* Accumulator 2 */
        sum3 = sum3 + sum1 * sum2;         /* Accumulator with complex dep */
        
        arr1[i] = arr1[i-1] + sum1;        /* Distance-1 array dep */
        arr2[i] = arr2[i-1] + sum2;        /* Another distance-1 */
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (parameter) */
int unknown_trip_count(int start, int end, int *restrict data) {
    volatile int result = 0;
    int i;
    
    /* Loop count not known at compile time */
    for (i = start + 1; i < end; i++) {
        /* Pattern that creates modulo scheduling opportunities */
        int temp = data[i-1] * 3 + data[i] * 2;  /* Distance-1 use */
        data[i] = temp + result;
        result = result ^ temp;                   /* Non-linear accumulator */
        
        /* Mix of operations for ILP */
        data[i] += (data[i] << 2) | (data[i] >> 3);
        
        asm volatile("" : "+r"(result) : : "memory");
    }
    return result;
}

/* Function 5: Complex loop with if-conversion potential */
int complex_conditional(int n, int *restrict a, int *restrict b, int threshold) {
    volatile int count = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency through count */
        int diff = a[i] - a[i-1];                 /* Distance-1 */
        
        /* Conditional that might be if-converted */
        if (diff > threshold) {
            b[i] = b[i-1] + diff * 2;             /* Another distance-1 */
            count += diff;
        } else {
            b[i] = b[i-1] - diff;                 /* Alternative path */
            count -= diff;
        }
        
        a[i] = a[i] ^ count;                      /* Non-linear update */
        
        asm volatile("" : : "r"(count) : "memory");
    }
    return count;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int n) {
    int i;
    for (i = 0; i < n; i++) {
        a[i] = (i * 13 + 7) & 0xFF;  /* Simple pseudo-random */
    }
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int SIZE = 1024;
    int *array1, *array2, *array3, *array4, *array5;
    int result = 0;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    array3 = (int*)malloc(SIZE * sizeof(int));
    array4 = (int*)malloc(SIZE * sizeof(int));
    array5 = (int*)malloc(SIZE * sizeof(int));
    
    init_arrays(array1, SIZE);
    init_arrays(array2, SIZE);
    init_arrays(array3, SIZE);
    init_arrays(array4, SIZE);
    init_arrays(array5, SIZE);
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(SIZE, array1, array2, array3);
    result += nested_loops(10, SIZE/10, array4, array5);
    result += multi_accumulators(SIZE, array1, array2);
    result += unknown_trip_count(100, SIZE-100, array3);
    result += complex_conditional(SIZE, array4, array5, 50);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return result != 0 ? 0 : 1;
}
