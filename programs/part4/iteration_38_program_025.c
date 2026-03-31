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
        int temp1 = a[i-1] * b[i];      /* distance-1 use */
        int temp2 = temp1 + c[i];
        a[i] = temp2 + sum;             /* accumulator pattern */
        sum = sum + a[i];               /* another distance-1 dependence */
        
        /* Additional operations for instruction-level parallelism */
        b[i] = b[i] ^ (a[i] >> 2);      /* independent operation */
        c[i] = c[i] * 3 + i;            /* another independent operation */
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Outer loop unrolled manually to some extent */
        int base = i * m;
        y[base] = x[base] + i;
        
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            /* Distance-1 dependence: y depends on previous y */
            int idx = base + j;
            y[idx] = y[idx-1] + x[idx] * 2;  /* distance-1 use */
            acc += y[idx];
            
            /* Additional operations */
            x[idx] = x[idx] ^ (y[idx] & 0xFF);
        }
        
        /* Prevent outer loop from being optimized away */
        asm volatile("" : : "r"(acc) : "memory");
    }
    
    return acc;
}

/* Function 3: Multiple interleaved accumulators with unknown trip count */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Initialize first element specially */
    arr1[0] = arr2[0] * 2;
    arr2[0] = arr1[0] + 1;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + arr1[i-1] * arr2[i];      /* distance-1 */
        sum2 = sum2 + arr2[i-1] + i;            /* distance-1 */
        sum3 = sum3 ^ (arr1[i] * 3);            /* independent but complex */
        
        /* Array updates with dependencies */
        arr1[i] = sum1 + arr2[i];
        arr2[i] = sum2 + arr1[i-1];             /* another distance-1 */
        
        /* Cross-iteration dependency chain */
        int temp = arr1[i-1] + arr2[i-1];       /* distance-1 use */
        arr1[i] = arr1[i] + temp;
    }
    
    /* Force all accumulators to be used */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    return sum1 + sum2 + sum3;
}

/* Function 4: Complex loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict a, int *restrict b, int threshold) {
    volatile int count = 0;
    int i;
    
    a[0] = b[0];
    for (i = 1; i < n; i++) {
        /* Carried dependency with conditional */
        int pred = a[i-1] > threshold;          /* distance-1 use */
        a[i] = b[i] + (pred ? a[i-1] : b[i-1]); /* distance-1 in one path */
        
        /* Another carried dependency */
        count = count + (a[i] & 1);             /* accumulator */
        
        /* Complex operation to increase latency */
        b[i] = (b[i] * 7 + a[i-1]) ^ count;     /* distance-1 use */
    }
    
    asm volatile("" : : "r"(count) : "memory");
    return count;
}

/* Function 5: Reduction loop with pointer chasing */
int pointer_chasing_loop(int n, int *restrict data) {
    volatile int result = 0;
    int i;
    
    /* Create chain of dependencies */
    int prev = data[0];
    for (i = 1; i < n; i++) {
        /* Strong distance-1 dependence chain */
        int curr = prev * 3 + data[i];          /* depends on prev from last iteration */
        result += curr;
        prev = curr;                            /* setup for next iteration */
        
        /* Additional independent work */
        data[i] = data[i] ^ (result & 0xFF);
    }
    
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int N = 1024;
    const int M = 256;
    int i;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    int *x = (int *)malloc(N * M * sizeof(int));
    int *y = (int *)malloc(N * M * sizeof(int));
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *data = (int *)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    
    for (i = 0; i < N * M; i++) {
        x[i] = rand() % 100;
        y[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    result += loop_carried_deps(N, a, b, c);
    result += nested_loop_deps(4, M, x, y);  /* Smaller outer loop */
    result += multi_accumulator(N, arr1, arr2);
    result += conditional_loop(N, a, b, 50);
    result += pointer_chasing_loop(N, data);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data);
    
    return 0;
}
