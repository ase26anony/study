/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduler register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

volatile int global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int x = 1, y = 2, z = 3;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Three independent recurrence chains */
        x = x * 13 + a[i];          /* Chain 1: distance 1 */
        y = y * 17 + b[i] - x;      /* Chain 2: depends on chain 1 */
        z = z * 19 + c[i] + y;      /* Chain 3: depends on chain 2 */
        
        /* Additional operations to increase register pressure */
        acc1 += x & 0xFF;
        acc2 += y | 0x55;
        acc3 += z ^ 0xAA;
        
        /* Array recurrence with distance 2 */
        if (i >= 2) {
            d[i] = d[i-2] * 3 + d[i-1] * 2;
        }
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(float *fa, float *fb, float *fc, int n) {
    float sum1 = 0.1f, sum2 = 0.2f, sum3 = 0.3f;
    float prod1 = 1.0f, prod2 = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01f + fa[i];            /* Distance 1 */
        sum2 = sum2 * 1.02f + fb[i] * sum1;     /* Distance 1 with cross-dep */
        sum3 = sum3 * 1.03f + fc[i] + sum2;
        
        /* Independent FP operations */
        prod1 *= (fa[i] * 0.5f + 1.0f);
        prod2 *= (fb[i] * 0.3f + fc[i] * 0.7f);
        
        /* Integer operations mixed in */
        int idx = (int)(fa[i] * 100) % 256;
        global_sum += idx;
    }
    
    global_sum += (int)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer chasing with strided access */
void test_pointer_chasing(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + stride * 2;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple pointer chains with dependencies */
        sum1 = sum1 * 11 + *ptr1;
        sum2 = sum2 * 13 + *ptr2 + sum1;
        sum3 = sum3 * 17 + *ptr3 + sum2;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computation to prevent optimization */
        sum1 ^= (sum2 << 3);
        sum2 |= (sum3 >> 2);
        sum3 &= (sum1 | 0xFFFF);
    }
    
    global_sum += sum1 + sum2 + sum3;
}

/* Test 4: Nested loops with inner loop being modulo-scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    for (int j = 0; j < m; j++) {
        int acc = j;
        int prev1 = a[0], prev2 = b[0];
        
        /* This inner loop should trigger modulo scheduling */
        for (int i = 1; i < n; i++) {
            /* Multiple interleaved recurrence chains */
            int temp1 = prev1 * 3 + a[i];
            int temp2 = prev2 * 5 + b[i] + temp1;
            int temp3 = c[i] * 7 + temp2;
            
            /* Cross-iteration dependencies */
            prev1 = temp1 ^ temp3;
            prev2 = temp2 | temp1;
            
            acc += temp1 + temp2 + temp3;
            
            /* More operations for register pressure */
            c[i] = (c[i-1] + temp1) & 0xFF;
            a[i] = (a[i-1] * 2 + temp2) % 256;
        }
        
        global_sum += acc;
    }
}

/* Test 5: Mixed types and operations for complex scheduling */
void test_mixed_types(short *sa, int *ia, float *fa, double *da, int n) {
    int isum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    int prev_int = ia[0];
    float prev_float = fa[0];
    double prev_double = da[0];
    
    for (int i = 1; i < n; i++) {
        /* Type 1: Integer chain with distance 1 */
        int cur_int = prev_int * 2 + ia[i] + sa[i];
        prev_int = cur_int;
        
        /* Type 2: Float chain with distance 1 */
        float cur_float = prev_float * 1.5f + fa[i];
        prev_float = cur_float;
        
        /* Type 3: Double chain with distance 1 */
        double cur_double = prev_double * 1.25 + da[i];
        prev_double = cur_double;
        
        /* Cross-type dependencies */
        isum += (int)(cur_float * 10) + cur_int;
        fsum += cur_float + (float)cur_double;
        dsum += cur_double + cur_int;
        
        /* Additional operations */
        sa[i] = (short)((cur_int + (int)cur_float) & 0xFFFF);
    }
    
    global_sum += isum + (int)fsum + (int)dsum;
}

/* Test 6: Loop with compile-time unrolling hint */
#pragma GCC unroll 4
void test_unrolled_loop(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple operations that should be unrolled */
        x = x * 3 + a[i];
        y = y * 5 + b[i] + x;
        z = z * 7 + c[i] + y;
        
        /* Additional parallel chains */
        int t1 = x ^ y;
        int t2 = y | z;
        int t3 = z & x;
        
        sum += t1 + t2 + t3 + x + y + z;
        
        /* Array recurrences */
        if (i >= 1) {
            a[i] = a[i-1] + b[i];
            b[i] = b[i-1] + c[i];
        }
    }
    
    global_sum += sum;
}

/* Test 7: PowerPC specific patterns (using double for FP register pressure) */
#ifdef __powerpc__
void test_powerpc_specific(double *da, double *db, double *dc, int n) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double-precision chains */
        sum1 = sum1 * 1.01 + da[i];
        sum2 = sum2 * 1.02 + db[i] * sum1;
        sum3 = sum3 * 1.03 + dc[i] + sum2;
        
        /* Independent chains */
        prod1 *= da[i] * 0.5 + 1.0;
        prod2 *= db[i] * 0.3 + dc[i] * 0.7;
        
        /* Mix in integer ops */
        global_sum += (int)(sum1 * 100);
    }
    
    global_sum += (int)(sum1 + sum2 + sum3 + prod1 + prod2);
}
#endif

/* Test 8: Variable distance dependencies */
void test_variable_distance(int *a, int *b, int *c, int n, int dist) {
    int history[8] = {0};  /* Small history buffer */
    
    for (int i = 0; i < n; i++) {
        /* Dependency with variable distance (1-3) */
        int d1 = (i >= 1) ? a[i-1] : 0;
        int d2 = (i >= 2) ? a[i-2] : 0;
        int d3 = (i >= dist) ? a[i-dist] : 0;
        
        /* Multiple computations using different distances */
        int val1 = d1 * 3 + b[i];
        int val2 = d2 * 5 + c[i];
        int val3 = d3 * 7 + b[i] * c[i];
        
        /* Update history */
        history[i % 8] = val1 + val2 + val3;
        
        /* Complex recurrence */
        a[i] = history[(i+1) % 8] + history[(i+3) % 8] + history[(i+7) % 8];
        
        global_sum += val1 + val2 + val3;
    }
}

int main() {
    /* Allocate and initialize test data */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    short *sa = malloc(SIZE * sizeof(short));
    float *fa = malloc(SIZE * sizeof(float));
    float *fb = malloc(SIZE * sizeof(float));
    float *fc = malloc(SIZE * sizeof(float));
    double *da = malloc(SIZE * sizeof(double));
    double *db = malloc(SIZE * sizeof(double));
    double *dc = malloc(SIZE * sizeof(double));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
        c[i] = rand() % 256;
        d[i] = rand() % 256;
        sa[i] = rand() % 256;
        fa[i] = (float)rand() / RAND_MAX;
        fb[i] = (float)rand() / RAND_MAX;
        fc[i] = (float)rand() / RAND_MAX;
        da[i] = (double)rand() / RAND_MAX;
        db[i] = (double)rand() / RAND_MAX;
        dc[i] = (double)rand() / RAND_MAX;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Cycle through different test patterns */
        test_multi_recurrence_int(a, b, c, d, SIZE);
        test_float_accumulate(fa, fb, fc, SIZE);
        test_pointer_chasing(a, 4, SIZE/4);
        test_nested_loops(a, b, c, SIZE, 4);
        test_mixed_types(sa, a, fa, da, SIZE);
        test_unrolled_loop(a, b, c, SIZE);
        test_variable_distance(a, b, c, SIZE, 3);
        
        #ifdef __powerpc__
        test_powerpc_specific(da, db, dc, SIZE);
        #endif
        
        /* Modify data slightly each iteration */
        if (iter % 100 == 0) {
            a[rand() % SIZE] = rand() % 256;
            b[rand() % SIZE] = rand() % 256;
        }
    }
    
    printf("Final global sum: %d\n", global_sum);
    printf("Tests completed.\n");
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(sa); free(fa); free(fb); free(fc);
    free(da); free(db); free(dc);
    
    return 0;
}
