/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Prevent compiler from optimizing away loops */
static void escape(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Function 1: Single accumulator with distance-1 dependence */
int loop_accumulator(int *data, int n, int coeff) {
    volatile int sum = 0;  /* volatile to preserve dependence */
    int i;
    
    /* Core loop with carried dependency */
    for (i = 1; i < n; i++) {
        /* Multiple operations to create ILP opportunities */
        int temp = data[i] * coeff;
        sum = sum + temp;           /* Distance-1 dependence on sum */
        data[i] = sum + data[i-1];  /* Another distance-1 dependence */
        sum = sum ^ data[i];        /* More operations on sum */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Multiple interleaved accumulators */
int loop_multiple_accumulators(int *a, int *b, int n) {
    volatile int sum1 = 0, sum2 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Two independent carried dependencies */
        sum1 = sum1 + a[i] * b[i];
        sum2 = sum2 + a[i-1] + b[i-1];  /* Distance-1 on array accesses */
        
        /* Cross-iteration dependencies */
        a[i] = sum1 + a[i-1];  /* Distance-1 on a[] */
        b[i] = sum2 + b[i-1];  /* Distance-1 on b[] */
        
        /* Complex operation mixing both accumulators */
        sum1 = sum1 ^ sum2;
        sum2 = sum2 * 3 + a[i];
        
        escape(&sum1);
        escape(&sum2);
    }
    return sum1 + sum2;
}

/* Function 3: Nested loops with inner loop carried dependency */
int loop_nested(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        for (j = 1; j < cols; j++) {
            /* Inner loop has distance-1 dependence */
            int idx = i * cols + j;
            int prev_idx = i * cols + (j - 1);
            row_sum = row_sum + matrix[idx] * matrix[prev_idx];
            matrix[idx] = row_sum + matrix[prev_idx];
            
            /* Additional operations to increase schedule complexity */
            row_sum = row_sum ^ (matrix[idx] >> 2);
            asm volatile("" : : "r"(row_sum) : "memory");
        }
        total += row_sum;
        
        /* Unrolled outer loop iteration */
        if (i % 2 == 0) {
            total = total * 7 - 3;
        }
    }
    return total;
}

/* Function 4: Loop with unknown trip count (prevents unrolling) */
int loop_variable_bound(int *arr, int start, int end, int init) {
    volatile int acc = init;
    int i;
    
    /* Trip count not known at compile time */
    for (i = start + 1; i < end; i++) {
        /* Strong distance-1 dependencies */
        int diff = arr[i] - arr[i-1];
        acc = acc * 31 + diff;
        arr[i] = acc + arr[i-1] * 17;
        
        /* Multiple uses of acc to create pressure */
        acc = acc ^ (arr[i] << 3);
        acc = acc + i;
        
        escape(&acc);
    }
    return acc;
}

/* Function 5: Complex recurrence with multiple dependencies */
int loop_complex_recurrence(int *x, int *y, int *z, int n) {
    volatile int a = x[0], b = y[0];
    int i;
    
    for (i = 1; i < n; i++) {
        /* Multiple interleaved distance-1 dependencies */
        int t1 = a + x[i-1];
        int t2 = b * y[i-1];
        a = t1 * 3 + z[i];
        b = t2 - a;
        x[i] = a + b;
        y[i] = a - b;
        z[i] = x[i-1] + y[i-1];
        
        /* Additional operations to create more edges */
        a = a ^ (b >> 1);
        b = b + (i & 0xFF);
        
        asm volatile("" : : "r"(a), "r"(b) : "memory");
    }
    return a + b;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int *data1, *data2, *data3, *data4;
    int *matrix, *x, *y, *z;
    int i, result = 0;
    
    /* Seed for reproducible results */
    srand(42);
    
    /* Allocate and initialize test arrays */
    data1 = (int*)malloc(SIZE * sizeof(int));
    data2 = (int*)malloc(SIZE * sizeof(int));
    data3 = (int*)malloc(SIZE * sizeof(int));
    data4 = (int*)malloc(SIZE * sizeof(int));
    matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    x = (int*)malloc(SIZE * sizeof(int));
    y = (int*)malloc(SIZE * sizeof(int));
    z = (int*)malloc(SIZE * sizeof(int));
    
    for (i = 0; i < SIZE; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
        data4[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        z[i] = rand() % 100;
    }
    
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = rand() % 100;
    }
    
    /* Call all test functions multiple times to ensure compilation */
    printf("Starting modulo scheduling tests...\n");
    
    for (i = 0; i < 10; i++) {
        result ^= loop_accumulator(data1, SIZE, 3);
        result ^= loop_multiple_accumulators(data2, data3, SIZE);
        result ^= loop_nested(matrix, 32, 32);
        result ^= loop_variable_bound(data4, 0, SIZE, result);
        result ^= loop_complex_recurrence(x, y, z, SIZE);
    }
    
    printf("Final result: %d\n", result);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(matrix);
    free(x);
    free(y);
    free(z);
    
    return 0;
}
