/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] + c[0];
    for (i = 1; i < n; i++) {
        /* Multiple operations to create instruction-level parallelism */
        int temp1 = a[i-1] * 3;      /* Carried dependency */
        int temp2 = b[i] + c[i];
        a[i] = temp1 + temp2;
        sum += a[i];                 /* Another carried dependency */
        
        /* Additional operations to create more edges */
        b[i] = b[i-1] + i;           /* Another distance-1 dependence */
        c[i] = c[i-1] * 2 - i;       /* Yet another distance-1 dependence */
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
        int acc = 0;
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            /* Pattern: y[j] depends on y[j-1] */
            y[j] = y[j-1] + x[i] * j;
            acc += y[j];
            
            /* Additional arithmetic to create schedule pressure */
            x[i] = (x[i] * 13 + 7) % 1024;
        }
        total += acc;
        
        /* Prevent outer loop unrolling completely */
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators with complex dependencies */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Initialize first elements to create proper dependencies */
    arr1[0] = 1;
    arr2[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + arr1[i-1] * arr2[i-1];  /* Uses previous iteration values */
        arr1[i] = arr1[i-1] + i * 7;          /* Distance-1 on arr1 */
        arr2[i] = arr2[i-1] * 3 - i;          /* Distance-1 on arr2 */
        sum2 = sum2 + arr1[i] * 3;
        sum3 = sum3 + arr2[i] * 5;
        
        /* Cross-dependencies between arrays */
        arr1[i] += arr2[i-1] % 16;
        arr2[i] += arr1[i-1] % 8;
    }
    
    /* Combine all sums with memory barrier */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (prevents complete unrolling) */
int variable_trip_count(int start, int end, int *restrict data) {
    volatile int result = 0;
    int i;
    
    /* Initialize first element */
    data[start] = start * 2;
    
    /* Loop with variable bounds - compiler can't unroll completely */
    for (i = start + 1; i < end; i++) {
        /* Strong carried dependency chain */
        data[i] = data[i-1] * 2 + data[i-1] / 3 + i;
        
        /* Additional operations to create scheduling pressure */
        int temp = data[i] * data[i-1];
        result += temp % 10007;
        
        /* Memory operation to create more edges */
        data[i-1] = (data[i-1] + temp) & 0xFF;
    }
    
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Function 5: Complex loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict a, int *restrict b, int threshold) {
    volatile int count = 0;
    int i;
    
    a[0] = 1;
    b[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency */
        a[i] = a[i-1] + b[i-1];
        
        /* Conditional that creates control dependencies */
        if (a[i] > threshold) {
            b[i] = b[i-1] * 2;
            count += a[i];
        } else {
            b[i] = b[i-1] / 2;
            count -= a[i];
        }
        
        /* Additional arithmetic */
        a[i] = (a[i] * 11) % 1000;
        b[i] = (b[i] * 13) % 1000;
    }
    
    asm volatile("" : : "r"(count) : "memory");
    return count;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    const int SIZE = 1024;
    int *a, *b, *c, *x, *y, *arr1, *arr2, *data;
    int result = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(SIZE * sizeof(int));
    b = (int*)malloc(SIZE * sizeof(int));
    c = (int*)malloc(SIZE * sizeof(int));
    x = (int*)malloc(SIZE * sizeof(int));
    y = (int*)malloc(SIZE * sizeof(int));
    arr1 = (int*)malloc(SIZE * sizeof(int));
    arr2 = (int*)malloc(SIZE * sizeof(int));
    data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(SIZE, a, b, c);
    result += nested_loop_carried(SIZE/16, 16, x, y);
    result += multi_accumulator(SIZE, arr1, arr2);
    result += variable_trip_count(SIZE/4, SIZE, data);
    result += conditional_loop(SIZE, a, b, 500);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data);
    
    return result != 0 ? 0 : 1;
}
