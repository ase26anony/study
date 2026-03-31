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
        /* Multiple carried dependencies creating distance1_uses */
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 dependence on a[i-1] */
        sum = sum + a[i] * 3;          /* Accumulator pattern */
        
        /* Additional operations to create instruction-level parallelism */
        b[i] = b[i-1] + i;             /* Another distance-1 dependence */
        c[i] = c[i] ^ (sum & 0xFF);    /* Use sum with bitwise operation */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Function 2: Nested loops with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Unroll outer loop partially */
        for (j = 1; j < m; j++) {
            /* Inner loop with carried dependency */
            x[j] = x[j-1] + y[j] * i;
            acc += x[j] >> 2;
            
            /* Create multiple use-def chains */
            y[j] = (y[j-1] + x[j]) & 0x7F;
            
            /* Force dependency preservation */
            asm volatile("" : "+r"(acc) : : "memory");
        }
    }
    
    return acc;
}

/* Function 3: Multiple interleaved accumulators with complex addressing */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0;
    int i;
    
    for (i = 2; i < n; i++) {
        /* Two separate carried dependencies */
        sum1 = sum1 + arr1[i-1] * arr2[i];    /* Uses arr1[i-1] */
        sum2 = sum2 + arr1[i-2] * sum1;       /* Uses arr1[i-2] and sum1 */
        
        /* Update arrays with carried dependencies */
        arr1[i] = (arr1[i-1] + arr1[i-2]) ^ sum2;
        arr2[i] = arr2[i-1] * 2 - sum1;
        
        /* Complex expression to create scheduling challenges */
        arr1[i] = (arr1[i] << 3) | (arr2[i] & 0x7);
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    
    return sum1 + sum2;
}

/* Function 4: Loop with unknown trip count (parameter) */
int variable_loop(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    /* Loop count not known at compile time */
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        int temp = data[i-1] * coeff;
        result = result + temp;
        data[i] = (data[i] + result) & 0xFFF;
        
        /* Additional arithmetic to create ILP opportunities */
        coeff = (coeff * 13 + 7) & 0xFF;
        result = result ^ (temp << 4);
        
        /* Memory clobber */
        asm volatile("" : : "r"(result) : "memory");
    }
    
    return result;
}

/* Function 5: Complex loop with if-conversion potential */
int conditional_loop(int n, int *restrict a, int *restrict b, int thresh) {
    volatile int count = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency through count */
        count = count + (a[i-1] > thresh ? 1 : 0);
        
        /* Multiple array updates with dependencies */
        a[i] = a[i-1] + b[i] * count;
        b[i] = b[i-1] - (count & 1);
        
        /* Complex conditional update */
        if (a[i] > b[i]) {
            a[i] = a[i] >> 1;
        } else {
            a[i] = a[i] << 1;
        }
        
        /* Force dependency chain */
        asm volatile("" : "+r"(count) : : "memory");
    }
    
    return count;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int SIZE = 1024;
    int *a, *b, *c, *x, *y, *arr1, *arr2, *data;
    int i, result = 0;
    
    /* Seed for reproducible results */
    srand(42);
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(SIZE * sizeof(int));
    b = (int*)malloc(SIZE * sizeof(int));
    c = (int*)malloc(SIZE * sizeof(int));
    x = (int*)malloc(SIZE * sizeof(int));
    y = (int*)malloc(SIZE * sizeof(int));
    arr1 = (int*)malloc(SIZE * sizeof(int));
    arr2 = (int*)malloc(SIZE * sizeof(int));
    data = (int*)malloc(SIZE * sizeof(int));
    
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    
    /* Call all test functions with different parameters */
    result += loop_carried_deps(SIZE, a, b, c);
    result += nested_loop_carried(SIZE/16, 64, x, y);
    result += multi_accumulator(SIZE, arr1, arr2);
    result += variable_loop(SIZE, data, 17);
    result += conditional_loop(SIZE, a, b, 50);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data);
    
    return 0;
}
