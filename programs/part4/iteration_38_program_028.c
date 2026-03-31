/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debugging output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Prevent compiler from optimizing away loops */
static volatile int force_keep = 0;

/* Function 1: Basic loop with multiple carried dependencies */
int loop_carried_dep(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Pattern: a[i] = a[i-1] * b[i] + c[i] */
    /* Creates distance-1 dependencies through a[i-1] */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * b[i] + c[i];
        sum += a[i];  /* Additional accumulator dependency */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return sum + force_keep;
}

/* Function 2: Multiple interleaved accumulators */
int multiple_accumulators(int n, int *restrict x, int *restrict y, 
                          int *restrict z, int *restrict w) {
    volatile int sum1 = 0, sum2 = 0;
    int i;
    
    /* Two separate carried dependencies */
    for (i = 1; i < n; i++) {
        /* Distance-1 dependency through x[i-1] */
        x[i] = x[i-1] + y[i] * z[i];
        
        /* Another distance-1 dependency through w[i-1] */
        w[i] = w[i-1] - y[i] + z[i];
        
        /* Two accumulators with carried dependencies */
        sum1 = sum1 + x[i];
        sum2 = sum2 + w[i];
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    
    return sum1 + sum2 + force_keep;
}

/* Function 3: Nested loop with inner carried dependency */
int nested_loop_carried(int n, int m, int *restrict mat, int *restrict vec) {
    volatile int total = 0;
    int i, j;
    
    /* Outer loop partially unrolled */
    for (i = 0; i < n; i += 2) {
        int acc1 = 0, acc2 = 0;
        
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            /* Distance-1 dependency through mat */
            mat[i*m + j] = mat[i*m + j-1] * vec[j] + i;
            acc1 += mat[i*m + j];
            
            if (i+1 < n) {
                mat[(i+1)*m + j] = mat[(i+1)*m + j-1] + vec[j] * j;
                acc2 += mat[(i+1)*m + j];
            }
        }
        
        total += acc1 + acc2;
        asm volatile("" : : "r"(total) : "memory");
    }
    
    return total + force_keep;
}

/* Function 4: Complex pattern with unknown trip count (parameter) */
int unknown_trip_count(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    if (n <= 1) return 0;
    
    /* Multiple interleaved distance-1 dependencies */
    for (i = 2; i < n; i++) {
        /* Chain of dependencies: data[i] depends on data[i-1] and data[i-2] */
        int temp = data[i-1] * coeff + data[i-2];
        data[i] = temp + i;
        
        /* Accumulator with carried dependency */
        result = result + data[i] * (i % 8);
        
        /* Additional operation with its own dependency */
        data[i-1] = data[i-1] + (result % 256);
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(result), "r"(data[i]), "r"(data[i-1]) : "memory");
    }
    
    return result + force_keep;
}

/* Function 5: Mixed operations to create complex dependency graph */
int mixed_operations(int n, int *restrict arr1, int *restrict arr2, 
                     int *restrict arr3, int *restrict arr4) {
    volatile int sum = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Multiple distance-1 dependencies */
        arr1[i] = arr1[i-1] * 3 + arr2[i];
        arr2[i] = arr2[i-1] + arr1[i] / 2;
        arr3[i] = arr3[i-1] - arr2[i] * arr1[i];
        arr4[i] = arr4[i-1] ^ (arr3[i] & 0xFF);
        
        /* Complex accumulator with multiple uses */
        sum = (sum + arr1[i]) * 7 - arr2[i] + arr3[i] ^ arr4[i];
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum), "r"(arr1[i]), "r"(arr2[i]), 
                          "r"(arr3[i]), "r"(arr4[i]) : "memory");
    }
    
    return sum + force_keep;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int *b, int *c, int *d, int n) {
    int i;
    for (i = 0; i < n; i++) {
        a[i] = (i * 13) % 97;
        b[i] = (i * 17) % 101;
        c[i] = (i * 19) % 103;
        d[i] = (i * 23) % 107;
    }
}

int main() {
    int *array1, *array2, *array3, *array4;
    int *matrix, *vector;
    int result = 0;
    int i;
    
    /* Allocate and initialize memory */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    array3 = (int*)malloc(SIZE * sizeof(int));
    array4 = (int*)malloc(SIZE * sizeof(int));
    matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    vector = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4 || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns */
    init_arrays(array1, array2, array3, array4, SIZE);
    init_arrays(vector, matrix, array1, array2, SIZE);
    
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (i * 29) % 113;
    }
    
    /* Call all test functions with different parameters
     * to ensure they're all compiled and potentially scheduled */
    
    /* Function 1 - Basic carried dependency */
    result += loop_carried_dep(SIZE, array1, array2, array3);
    
    /* Function 2 - Multiple accumulators */
    result += multiple_accumulators(SIZE, array1, array2, array3, array4);
    
    /* Function 3 - Nested loops */
    result += nested_loop_carried(32, 32, matrix, vector);
    
    /* Function 4 - Unknown trip count (parameter) */
    result += unknown_trip_count(SIZE, array1, 7);
    
    /* Function 5 - Mixed operations */
    result += mixed_operations(SIZE, array2, array3, array4, array1);
    
    /* Additional calls with different parameters to create variation */
    for (i = 0; i < 10; i++) {
        result += loop_carried_dep(SIZE/2 + i, array1, array2, array3);
        result += multiple_accumulators(SIZE/4 + i, array3, array4, array1, array2);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(matrix);
    free(vector);
    
    return 0;
}
