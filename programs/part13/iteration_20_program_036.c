/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test_modulo_sched.c -o test_modulo_sched
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multiple_recurrence_chains(int *a, int *b, int *c, int *d, int n) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1 = 5, y2 = 6, y3 = 7, y4 = 8;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent recurrence chains */
        x1 = x1 * 13 + a[i];
        x2 = x2 * 17 + b[i];
        x3 = x3 * 19 + c[i];
        x4 = x4 * 23 + d[i];
        
        /* Cross-chain operations to increase register pressure */
        y1 = y1 + x1 * 3 - x2;
        y2 = y2 + x2 * 5 - x3;
        y3 = y3 + x3 * 7 - x4;
        y4 = y4 + x4 * 11 - x1;
        
        /* More operations to create register pressure */
        a[i] = x1 + y1;
        b[i] = x2 + y2;
        c[i] = x3 + y3;
        d[i] = x4 + y4;
    }
    
    global_sum += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulation(float *fa, float *fb, float *fc, int n) {
    float sum1 = 0.1f, sum2 = 0.2f, sum3 = 0.3f;
    float prod1 = 1.0f, prod2 = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP recurrence chains */
        sum1 = sum1 * 1.01f + fa[i];
        sum2 = sum2 * 1.02f + fb[i];
        sum3 = sum3 * 1.03f + fc[i];
        
        /* Cross dependencies */
        prod1 = prod1 * (sum1 + 0.5f);
        prod2 = prod2 * (sum2 - 0.3f);
        
        /* More operations to increase register usage */
        fa[i] = sum1 * prod1;
        fb[i] = sum2 * prod2;
        fc[i] = sum3 + prod1 - prod2;
    }
    
    global_sum += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *arr, int n, int stride) {
    int *ptr1 = arr;
    int *ptr2 = arr + stride;
    int *ptr3 = arr + 2 * stride;
    int *end = arr + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr3 < end) {
        /* Pointer chasing with dependencies */
        acc1 = acc1 * 3 + *ptr1;
        acc2 = acc2 * 5 + *ptr2;
        acc3 = acc3 * 7 + *ptr3;
        
        /* Cross updates */
        *ptr1 = acc1 + acc2;
        *ptr2 = acc2 + acc3;
        *ptr3 = acc3 + acc1;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with shift and bitwise ops */
void test_mixed_operations(int *a, int *b, int *c, int n) {
    unsigned int x = 1, y = 2, z = 3;
    unsigned int mask1 = 0xAAAAAAAA, mask2 = 0x55555555;
    
    for (int i = 0; i < n; i++) {
        /* Various operations to use different functional units */
        x = (x << 3) | (x >> 29);  /* Rotate left */
        x = x + a[i] * 7;
        x = x ^ mask1;
        
        y = (y << 5) | (y >> 27);  /* Different rotate */
        y = y + b[i] * 11;
        y = y & mask2;
        
        z = (z << 7) | (z >> 25);  /* Another rotate */
        z = z + c[i] * 13;
        z = z | (x & y);
        
        /* Store results with dependencies */
        a[i] = x + z;
        b[i] = y + x;
        c[i] = z + y;
    }
    
    global_sum += x + y + z;
}

/* Test 5: Nested loops with inner loop being the hot spot */
void test_nested_loops(int *mat, int rows, int cols) {
    for (int i = 1; i < rows - 1; i++) {
        int *prev = mat + (i - 1) * cols;
        int *curr = mat + i * cols;
        int *next = mat + (i + 1) * cols;
        
        /* Innermost loop with multiple dependencies */
        for (int j = 1; j < cols - 1; j++) {
            /* Stencil computation with true dependencies */
            int up = prev[j];
            int left = curr[j - 1];
            int center = curr[j];
            int right = curr[j + 1];
            int down = next[j];
            
            /* Multiple computation chains */
            int sum1 = up + left + right + down;
            int sum2 = center * 2 - sum1;
            int sum3 = (up * left) + (right * down);
            
            /* Cross dependencies */
            curr[j] = sum1 + sum2 - sum3;
            prev[j] = sum1 * 2;
            next[j] = sum3 / 4;
        }
    }
    
    /* Accumulate some values to prevent elimination */
    for (int i = 0; i < rows * cols; i += 64) {
        global_sum += mat[i];
    }
}

/* Test 6: Double precision floating point with high register pressure */
void test_double_precision(double *da, double *db, double *dc, int n) {
    double acc1 = 0.5, acc2 = 1.0, acc3 = 1.5;
    double fact1 = 1.0001, fact2 = 1.0002, fact3 = 1.0003;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double precision recurrence chains */
        acc1 = acc1 * fact1 + da[i];
        acc2 = acc2 * fact2 + db[i];
        acc3 = acc3 * fact3 + dc[i];
        
        /* Cross-multiplications to increase register pressure */
        double t1 = acc1 * acc2;
        double t2 = acc2 * acc3;
        double t3 = acc3 * acc1;
        
        /* More operations */
        da[i] = t1 + acc1;
        db[i] = t2 + acc2;
        dc[i] = t3 + acc3;
        
        /* Update factors slightly */
        fact1 *= 0.99999;
        fact2 *= 0.99998;
        fact3 *= 0.99997;
    }
    
    global_sum += (long long)(acc1 + acc2 + acc3);
}

/* Main test driver */
int main() {
    /* Initialize data arrays */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    
    float *fa = malloc(SIZE * sizeof(float));
    float *fb = malloc(SIZE * sizeof(float));
    float *fc = malloc(SIZE * sizeof(float));
    
    double *da = malloc(SIZE * sizeof(double));
    double *db = malloc(SIZE * sizeof(double));
    double *dc = malloc(SIZE * sizeof(double));
    
    int *matrix = malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        
        fa[i] = (float)rand() / RAND_MAX;
        fb[i] = (float)rand() / RAND_MAX;
        fc[i] = (float)rand() / RAND_MAX;
        
        da[i] = (double)rand() / RAND_MAX;
        db[i] = (double)rand() / RAND_MAX;
        dc[i] = (double)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = rand() % 256;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_multiple_recurrence_chains(a, b, c, d, SIZE);
        test_float_accumulation(fa, fb, fc, SIZE);
        test_pointer_chasing(a, SIZE, 4);
        test_mixed_operations(a, b, c, SIZE);
        test_nested_loops(matrix, 32, 32);
        test_double_precision(da, db, dc, SIZE);
        
        /* Modify data slightly each iteration */
        if (iter % 100 == 0) {
            for (int i = 0; i < SIZE; i++) {
                a[i] += 1;
                b[i] += 2;
                c[i] += 3;
            }
        }
    }
    
    printf("Final global sum: %lld\n", global_sum);
    printf("Tests completed.\n");
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb); free(fc);
    free(da); free(db); free(dc);
    free(matrix);
    
    return 0;
}
