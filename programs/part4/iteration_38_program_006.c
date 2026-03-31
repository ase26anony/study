/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Simple loop with distance-1 dependence and accumulator */
int loop_with_carried_dep(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependence */
    int i;
    
    /* Loop with multiple carried dependencies */
    for (i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Another distance-1 dependence with accumulator */
        sum = sum + a[i] * 3;
        
        /* Additional operation to create more ILP opportunities */
        b[i] = b[i-1] + (c[i] << 2);
    }
    
    /* Memory clobber to prevent optimization */
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Function 2: Nested loops with inner loop having carried dependency */
void nested_loops_with_deps(int n, int m, int *restrict x, int *restrict y) {
    int i, j;
    volatile int acc = 0;
    
    for (i = 0; i < n; i++) {
        /* Unrolled outer loop */
        int temp = x[i];
        
        for (j = 1; j < m; j++) {
            /* Inner loop with distance-1 dependence */
            y[j] = y[j-1] + temp * j;
            
            /* Multiple operations to create scheduling opportunities */
            temp = temp + (y[j] & 0xFF);
            acc = acc ^ y[j];
        }
        
        x[i] = temp;
    }
    
    asm volatile("" : : "r"(acc) : "memory");
}

/* Function 3: Multiple interleaved carried dependencies */
int multiple_interleaved_deps(int n, int *restrict arr1, int *restrict arr2, 
                              int *restrict arr3) {
    volatile int sum1 = 0, sum2 = 0;
    int i;
    
    for (i = 2; i < n; i++) {
        /* Three different distance-1 dependencies */
        arr1[i] = arr1[i-1] + arr1[i-2];  /* Distance 1 and 2 */
        arr2[i] = arr2[i-1] * arr3[i] + 7;
        arr3[i] = arr3[i-1] - (arr1[i] >> 1);
        
        /* Two separate accumulators with carried dependencies */
        sum1 = sum1 + arr1[i];
        sum2 = sum2 ^ arr2[i];
        
        /* Additional computation to increase register pressure */
        arr1[i] = arr1[i] + (arr2[i] * arr3[i]) / 3;
    }
    
    asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    return sum1 + sum2;
}

/* Function 4: Loop with unknown trip count (parameter) and complex deps */
int unknown_trip_count(int start, int end, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    /* Loop count not known at compile time */
    for (i = start + 1; i < end; i++) {
        /* Multiple carried dependencies */
        data[i] = data[i-1] * coeff + data[i] * 2;
        
        /* Accumulator with distance-1 dependence */
        result = result + data[i] * i;
        
        /* Additional operations for scheduling complexity */
        coeff = (coeff * 13 + 1) & 0xFF;
        data[i] = data[i] ^ (data[i-1] << 3);
    }
    
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Function 5: Software pipelining candidate with high ILP */
void high_ilp_loop(int n, int *restrict a, int *restrict b, 
                   int *restrict c, int *restrict d) {
    int i;
    volatile int t1 = 0, t2 = 0, t3 = 0;
    
    for (i = 1; i < n; i++) {
        /* Multiple independent chains with distance-1 dependencies */
        a[i] = a[i-1] + b[i] * 5;
        b[i] = b[i-1] - c[i] * 3;
        c[i] = c[i-1] + d[i] * 2;
        d[i] = d[i-1] ^ a[i];
        
        /* Cross dependencies between chains */
        t1 = t1 + a[i] * b[i];
        t2 = t2 + c[i] * d[i];
        t3 = t3 ^ (a[i] + b[i] + c[i] + d[i]);
    }
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3) : "memory");
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int SIZE = 1000;
    int i, result = 0;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *d = (int*)malloc(SIZE * sizeof(int));
    int *x = (int*)malloc(SIZE * sizeof(int));
    int *y = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_with_carried_dep(SIZE, a, b, c);
    nested_loops_with_deps(10, SIZE/10, x, y);
    result += multiple_interleaved_deps(SIZE, a, b, c);
    result += unknown_trip_count(0, SIZE, d, 17);
    high_ilp_loop(SIZE, a, b, c, d);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(x); free(y);
    
    return result != 0 ? 0 : 1;
}
