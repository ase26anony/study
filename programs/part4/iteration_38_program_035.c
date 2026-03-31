/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Single loop with multiple carried dependencies */
void loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    
    /* Loop with distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Another distance-1 dependence with accumulator pattern */
        acc1 = acc1 + d[i] * 3;
        
        /* Cross-iteration dependency with different distance */
        b[i] = (b[i-1] + a[i]) >> 1;
        
        /* Second accumulator with different operation */
        acc2 = acc2 * 2 + c[i];
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    a[0] = acc1 + acc2;
}

/* Function 2: Nested loops with inner loop carried dependency */
void nested_loop_deps(int n, int m, int *restrict mat, int *restrict vec) {
    volatile int sum = 0;
    
    /* Outer loop partially unrolled */
    for (int i = 0; i < n; i += 2) {
        /* Inner loop with carried dependency */
        for (int j = 1; j < m; j++) {
            /* Distance-1 dependence in inner loop */
            mat[i*m + j] = mat[i*m + j-1] * vec[j] + i;
            
            /* Accumulator with loop-carried dependency */
            sum = sum + mat[i*m + j] * 7;
            
            /* Another array with stride-1 access pattern */
            vec[j] = (vec[j-1] + mat[i*m + j]) & 0xFF;
        }
        
        /* Prevent outer loop optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    /* Store result */
    mat[0] = sum;
}

/* Function 3: Multiple interleaved accumulators with unknown trip count */
int multi_accumulators(int n, int *restrict data, int coeff1, int coeff2) {
    volatile int acc_a = 0;
    volatile int acc_b = 0;
    volatile int acc_c = 1;
    
    /* Loop with parameter-dependent trip count */
    for (int i = 1; i < n; i++) {
        /* Three separate accumulators with carried dependencies */
        acc_a = acc_a + data[i] * coeff1;
        acc_b = acc_b - data[i-1] * coeff2;  /* Distance-1 use */
        acc_c = acc_c * 2 + data[i];
        
        /* Create data dependencies between accumulators */
        data[i] = (acc_a + acc_b) ^ acc_c;
        
        /* Complex operation with multiple dependencies */
        coeff1 = (coeff1 * 13 + i) & 0x7F;
        coeff2 = (coeff2 * 17 - i) & 0x7F;
        
        /* Memory clobber to preserve dependencies */
        asm volatile("" : : "r"(acc_a), "r"(acc_b), "r"(acc_c) : "memory");
    }
    
    return acc_a + acc_b + acc_c;
}

/* Function 4: Loop with if-conversion opportunities */
void conditional_loop(int n, int *restrict x, int *restrict y, int threshold) {
    volatile int count = 0;
    volatile int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence */
        int diff = x[i] - x[i-1];
        
        /* Conditional that may be if-converted */
        if (diff > threshold) {
            y[i] = y[i-1] + diff * 2;  /* Another distance-1 */
            count++;
        } else {
            y[i] = y[i-1] - diff;      /* Same distance-1 pattern */
        }
        
        /* Accumulator with carried dependency */
        sum = sum + y[i];
        
        /* Prevent optimization */
        asm volatile("" : : "r"(count), "r"(sum) : "memory");
    }
    
    x[0] = sum + count;
}

/* Function 5: Complex loop with pointer chasing */
void pointer_chasing_loop(int n, int *restrict arr, int *restrict hist) {
    volatile int *ptr = arr;
    volatile int total = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Pointer-based distance-1 access */
        int curr = *ptr;
        int next = *(ptr + 1);
        
        /* Computation with carried dependency */
        *ptr = curr * 3 + next;
        
        /* Histogram update with dependency */
        hist[curr & 0xF] = hist[curr & 0xF] + 1;
        
        /* Accumulator */
        total = total + *ptr;
        
        /* Move pointer (creates address dependency) */
        ptr++;
        
        /* Memory barrier */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    arr[0] = total;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int N = 1000;
    const int M = 100;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    int *mat = (int*)malloc(N * M * sizeof(int));
    int *vec = (int*)malloc(M * sizeof(int));
    int *hist = (int*)malloc(16 * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    for (int i = 0; i < N * M; i++) {
        mat[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        vec[i] = rand() % 100;
    }
    
    for (int i = 0; i < 16; i++) {
        hist[i] = 0;
    }
    
    /* Call all test functions with different parameters */
    loop_carried_deps(N, a, b, c, d);
    nested_loop_deps(10, M, mat, vec);
    
    int coeff1 = rand() % 100;
    int coeff2 = rand() % 100;
    int result1 = multi_accumulators(N, a, coeff1, coeff2);
    
    int threshold = 50;
    conditional_loop(N, b, c, threshold);
    
    pointer_chasing_loop(N, d, hist);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum = (checksum * 31 + a[i]) & 0xFFFF;
        checksum = (checksum * 31 + b[i]) & 0xFFFF;
        checksum = (checksum * 31 + c[i]) & 0xFFFF;
        checksum = (checksum * 31 + d[i]) & 0xFFFF;
    }
    
    checksum += result1;
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(mat);
    free(vec);
    free(hist);
    
    return 0;
}
