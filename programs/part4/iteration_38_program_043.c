/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] + c[0];
    for (i = 1; i < n; i++) {
        /* Multiple operations to create complex schedule */
        int temp1 = a[i-1] * 3;      /* Distance-1 use of a[i-1] */
        int temp2 = b[i] + 7;
        a[i] = temp1 + temp2 + c[i];
        
        /* Accumulator with carried dependency */
        sum = sum + a[i];            /* Another distance-1 dependence */
        
        /* Additional operation to increase ILP */
        c[i] = c[i-1] * 2 - b[i];    /* Yet another distance-1 dependence */
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Function 2: Nested loop with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        int inner_sum = 0;
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            /* Pattern: y[j] depends on y[j-1] */
            y[j] = y[j-1] + x[i] * (j % 8);
            inner_sum += y[j];
            
            /* Additional operation with stride-1 access */
            x[i] = x[i] + (y[j] & 0xFF);
        }
        total += inner_sum;
        
        /* Prevent outer loop unrolling from eliminating modulo scheduling */
        if (i % 4 == 0) {
            asm volatile("" : : "r"(inner_sum) : "memory");
        }
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int acc1 = 0, acc2 = 0, acc3 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        acc1 = acc1 + arr1[i] * 2;
        acc2 = acc2 - arr2[i-1];      /* Explicit distance-1 use of arr2[i-1] */
        acc3 = acc3 ^ (arr1[i-1] + arr2[i]);  /* Mixed distance-0 and distance-1 */
        
        /* Update arrays with carried dependencies */
        arr1[i] = arr1[i-1] + acc1 % 256;
        arr2[i] = arr2[i-1] * 3 - acc2;
        
        /* Complex expression to increase register pressure */
        int temp = (acc1 * acc2) / (acc3 + 1);
        arr1[i] += temp & 0xFF;
    }
    
    /* Force all accumulators to be live */
    asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3) : "memory");
    return acc1 + acc2 + acc3;
}

/* Function 4: Loop with unknown trip count (prevents unrolling) */
int unknown_trip_count(int *restrict data, int start, int end) {
    volatile int result = 0;
    int i;
    
    /* Loop count not known at compile time */
    for (i = start + 1; i < end; i++) {
        /* Strong distance-1 dependencies */
        data[i] = data[i-1] * 2 + data[i];
        result += data[i] * i;
        
        /* Additional carried dependency chain */
        int chain = data[i-1] & 0xF;
        data[i] ^= chain;
        result -= chain * 3;
    }
    
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Function 5: Complex loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict a, int *restrict b, int threshold) {
    volatile int count = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency through array */
        a[i] = a[i-1] + b[i];
        
        /* Conditional that creates control dependencies */
        if (a[i] > threshold) {
            count += a[i] * 2;
            b[i] = b[i-1] + 1;  /* Another distance-1 dependence */
        } else {
            count -= a[i];
            b[i] = b[i-1] - 1;  /* Distance-1 dependence in else path */
        }
        
        /* Additional operation to increase schedule complexity */
        a[i] = (a[i] * 3) % 1024;
    }
    
    asm volatile("" : : "r"(count) : "memory");
    return count;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int SIZE = 1024;
    int *array1, *array2, *array3, *array4, *array5;
    int i, result = 0;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    array3 = (int*)malloc(SIZE * sizeof(int));
    array4 = (int*)malloc(SIZE * sizeof(int));
    array5 = (int*)malloc(SIZE * sizeof(int));
    
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        array4[i] = rand() % 100;
        array5[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(SIZE, array1, array2, array3);
    result += nested_loop_carried(64, 16, array4, array5);
    result += multi_accumulators(SIZE, array1, array2);
    result += unknown_trip_count(array3, 0, SIZE);
    result += conditional_loop(SIZE, array4, array5, 5000);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return result != 0 ? 0 : 1;
}
