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
        /* Multiple operations with carried dependencies */
        int temp = a[i-1] * 3;      /* Distance-1 use of a[i-1] */
        a[i] = temp + b[i] * c[i];  /* Def of a[i] */
        sum += a[i];                /* Accumulator with carried dependency */
        
        /* Additional operation with another carried dependency */
        b[i] = b[i-1] + i;          /* Another distance-1 dependence */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop having carried dependencies */
int nested_loops(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = 0;
        /* Inner loop with multiple carried dependencies */
        for (j = 1; j < m; j++) {
            /* Pattern: y[j] depends on y[j-1] AND x[j-1] */
            int val1 = y[j-1] * 2;      /* Distance-1 use */
            int val2 = x[j-1] + 5;      /* Another distance-1 use */
            y[j] = val1 + val2 + j;     /* Def with multiple dependencies */
            acc += y[j] * 3;            /* Accumulator */
            
            /* Interleaved dependency */
            x[j] = x[j-1] + y[j] - 7;
        }
        total += acc;
        /* Prevent outer loop optimization */
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 3: Multiple independent accumulators with complex dependencies */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Initialize first elements */
    arr1[0] = 1;
    arr2[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependency chains */
        int chain1 = arr1[i-1] * 7 + i;      /* Distance-1 use */
        int chain2 = arr2[i-1] / 3 + i * 2;  /* Distance-1 use */
        
        arr1[i] = chain1 + chain2;
        arr2[i] = arr1[i-1] + arr2[i-1];     /* Uses both previous values */
        
        /* Three accumulators with different dependencies */
        sum1 += arr1[i];                     /* Depends on arr1[i] */
        sum2 += arr2[i] * sum1;              /* Depends on sum1 (carried) */
        sum3 = sum3 * 2 + arr1[i-1];         /* Explicit distance-1 use */
        
        /* Complex expression to create more ILP opportunities */
        arr1[i] = (arr1[i] + arr2[i]) * (sum1 - sum2) / (sum3 + 1);
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    return sum1 + sum2 * 2 + sum3 * 3;
}

/* Function 4: Loop with unknown trip count (parameter) and mixed operations */
int unknown_trip_count(int start, int end, int *restrict data) {
    volatile int result = 0;
    int i;
    
    /* Initialize first element */
    data[start] = start * 2;
    
    for (i = start + 1; i < end; i++) {
        /* Multiple distance-1 dependencies */
        int prev_val = data[i-1];                 /* Direct distance-1 use */
        int computed = prev_val * 3 + i * 5;      /* Arithmetic with distance-1 */
        
        data[i] = computed + data[i-1] / 2;       /* Another distance-1 use */
        
        /* Accumulator with carried dependency */
        result = result + data[i] - data[i-1];    /* Uses both current and previous */
        
        /* Additional operations to create scheduling complexity */
        data[i] = data[i] ^ (data[i-1] & 0xFF);   /* Bitwise with distance-1 */
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(result) : "memory");
    }
    return result;
}

/* Function 5: Loop with if-conversion opportunities and carried deps */
int conditional_loop(int n, int *restrict src, int *restrict dst) {
    volatile int count = 0;
    int i;
    
    dst[0] = src[0];
    for (i = 1; i < n; i++) {
        /* Condition that creates control dependencies */
        int pred = src[i] > src[i-1];  /* Distance-1 comparison */
        
        /* Operations with carried dependencies in both paths */
        if (pred) {
            dst[i] = dst[i-1] + src[i] * 2;      /* True path with distance-1 */
            count += dst[i] * 3;
        } else {
            dst[i] = dst[i-1] - src[i];          /* False path with distance-1 */
            count -= dst[i];
        }
        
        /* Additional unconditional carried dependency */
        src[i] = src[i-1] + i % 7;
        
        asm volatile("" : : "r"(count) : "memory");
    }
    return count;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    const int SIZE = 1000;
    int *array1, *array2, *array3, *array4, *array5;
    int result = 0;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    array3 = (int*)malloc(SIZE * sizeof(int));
    array4 = (int*)malloc(SIZE * sizeof(int));
    array5 = (int*)malloc(SIZE * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        array4[i] = rand() % 100;
        array5[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(SIZE, array1, array2, array3);
    result += nested_loops(10, SIZE/10, array4, array5);
    result += multi_accumulators(SIZE, array1, array2);
    result += unknown_trip_count(0, SIZE, array3);
    result += conditional_loop(SIZE, array4, array5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return 0;
}
