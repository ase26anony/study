/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Core loop with distance-1 dependencies */
    for (i = 1; i < n; i++) {
        /* Multiple carried dependencies creating distance1_uses scenarios */
        a[i] = a[i-1] * b[i] + c[i];      /* Distance-1: a[i-1] used in iteration i */
        sum = sum + a[i] * 3;             /* Accumulator pattern */
        b[i] = b[i-1] + sum / 2;          /* Another distance-1 dependency */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Function 2: Nested loops with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    int i, j;
    volatile int acc = 0;
    
    for (i = 0; i < n; i++) {
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            x[i*m + j] = x[i*m + j-1] + y[i*m + j] * 2;  /* Distance-1 */
            acc += x[i*m + j];
            
            /* Create additional ILP */
            y[i*m + j] = y[i*m + j-1] * 3 - acc;
        }
        
        /* Prevent outer loop optimization */
        asm volatile("" : : "r"(acc) : "memory");
    }
    
    return acc;
}

/* Function 3: Multiple interleaved accumulators with complex dependencies */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + arr1[i] * arr1[i-1];      /* Distance-1: arr1[i-1] */
        sum2 = sum2 + arr2[i] * sum1;           /* Depends on sum1 */
        arr1[i] = arr1[i-1] + sum2;             /* Another distance-1 */
        sum3 = sum3 + arr1[i] * arr2[i];
        
        /* Complex expression with multiple operations */
        arr2[i] = (arr2[i-1] * 2 + sum3) / (sum1 > 0 ? sum1 : 1);
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (parameter) and mixed operations */
int unknown_trip_count(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    /* Loop count not known at compile time */
    for (i = 1; i < n; i++) {
        /* Mixed operations creating various dependence edges */
        int temp = data[i-1] * coeff;           /* Distance-1 dependency */
        data[i] = temp + data[i] * 2;
        result += data[i] * (i % 8);
        
        /* Additional arithmetic to increase ILP */
        coeff = (coeff * 13 + 7) % 256;         /* Simple PRNG-like update */
        
        /* Force dependence preservation */
        asm volatile("" : : "r"(temp), "r"(result) : "memory");
    }
    
    return result;
}

/* Function 5: Unrolled outer loop with complex inner pattern */
int unrolled_outer_loop(int n, int *restrict buf1, int *restrict buf2) {
    volatile int total = 0;
    int i;
    
    /* Manual unrolling to create more scheduling opportunities */
    for (i = 2; i < n; i += 2) {
        /* First iteration of unrolled chunk */
        buf1[i] = buf1[i-1] + buf2[i] * 3;      /* Distance-1 */
        total += buf1[i] * 5;
        buf2[i] = buf2[i-1] - total / 4;        /* Another distance-1 */
        
        /* Second iteration of unrolled chunk */
        buf1[i+1] = buf1[i] + buf2[i+1] * 7;    /* Distance-1 within chunk */
        total += buf1[i+1] * 11;
        buf2[i+1] = buf2[i] - total / 8;        /* Distance-1 within chunk */
        
        /* Memory barrier for the entire chunk */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    return total;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    const int SIZE = 1024;
    int *array1, *array2, *array3, *array4, *array5;
    int i, result = 0;
    
    /* Seed RNG for reproducible array values */
    srand(42);
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    array3 = (int*)malloc(SIZE * sizeof(int));
    array4 = (int*)malloc(SIZE * sizeof(int));
    array5 = (int*)malloc(SIZE * sizeof(int));
    
    for (i = 0; i < SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        array4[i] = rand() % 100;
        array5[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(SIZE, array1, array2, array3);
    result += nested_loop_carried(16, 64, array4, array5);
    result += multi_accumulator(SIZE, array1, array2);
    result += unknown_trip_count(SIZE, array3, 17);
    result += unrolled_outer_loop(SIZE, array4, array5);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return 0;
}
