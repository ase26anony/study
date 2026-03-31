/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    for (i = 1; i < n; i++) {
        /* Multiple operations to create non-trivial schedule */
        int temp = a[i-1] * b[i];      /* Carried dependence on a[i-1] */
        temp += c[i] * 3;              /* Independent operation */
        a[i] = temp + sum;             /* Uses sum from previous iteration */
        sum = sum + a[i] / 2;          /* Accumulator with carried dependence */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loop with inner loop carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Unrolled outer loop operations */
        acc1 = acc1 + x[i] * 2;
        
        for (j = 1; j < m; j++) {
            /* Inner loop with carried dependency */
            y[j] = y[j-1] + x[i] * y[j];  /* Distance-1 on y[j-1] */
            acc2 = acc2 + y[j];           /* Another accumulator */
            
            /* Additional operations for parallelism */
            y[j] += (acc1 & 0xFF) * 7;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved carried dependencies */
int multi_dependencies(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Complex loop with multiple dependency chains */
    for (i = 2; i < n; i++) {
        /* Chain 1: arr1[i] depends on arr1[i-2] and arr1[i-1] */
        int val1 = arr1[i-2] * 3 + arr1[i-1] * 2;
        
        /* Chain 2: arr2[i] depends on arr2[i-1] */
        int val2 = arr2[i-1] + arr1[i] * 5;
        
        /* Chain 3: Multiple accumulators with carried dependencies */
        sum1 = sum1 + val1;
        sum2 = sum2 + val2;
        sum3 = sum3 + (sum1 & 0xF) * (sum2 & 0xF);
        
        /* Store results creating more dependencies */
        arr1[i] = val1 + (sum3 % 256);
        arr2[i] = val2 - (sum1 % 128);
        
        /* Force dependency preservation */
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
        /* Pattern with distance-1 dependencies */
        int diff = data[i] - data[i-1];          /* Distance-1 dependence */
        result = result + diff * diff;           /* Accumulator */
        data[i] = data[i-1] + (result & 0x3F);   /* Another distance-1 */
        
        /* Mix in some independent operations */
        data[i] ^= (i * 0x9E3779B9);             /* Non-dependent operation */
        
        asm volatile("" : : "r"(result) : "memory");
    }
    return result;
}

/* Function 5: Complex loop with if-conversion opportunities */
int conditional_deps(int n, int *restrict a, int *restrict b, int threshold) {
    volatile int count = 0, total = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency through total */
        int base = a[i-1] + total;
        
        /* Conditional that might be if-converted */
        int increment = (base > threshold) ? b[i] * 2 : b[i];
        
        /* Multiple updates with dependencies */
        total = total + increment;
        a[i] = base + (total & 0xFF);
        
        /* Another carried dependency */
        count = count + (a[i] > 0);
        
        asm volatile("" : : "r"(total), "r"(count) : "memory");
    }
    return total - count;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int n) {
    int i;
    for (i = 0; i < n; i++) {
        a[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
}

int main() {
    const int N = 1024;
    const int M = 512;
    int *array1, *array2, *array3, *array4, *array5;
    int result = 0;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    array4 = (int*)malloc(N * sizeof(int));
    array5 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4 || !array5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, N);
    init_arrays(array2, N);
    init_arrays(array3, N);
    init_arrays(array4, N);
    init_arrays(array5, N);
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(N, array1, array2, array3);
    result += nested_loop_deps(N/16, M, array4, array5);
    result += multi_dependencies(N, array1, array2);
    result += unknown_trip_count(100, N-100, array3);
    result += conditional_deps(N, array4, array5, 10000);
    
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
