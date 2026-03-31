/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int acc1 = a[0];
    int acc2 = b[0] + b[1];
    int acc3 = c[0];
    int acc4 = d[0];
    
    /* Multiple independent dependency chains with different distances */
    for (int i = 1; i < n; i++) {
        /* Chain 1: distance 1 recurrence */
        acc1 = acc1 * 3 + a[i] * 7;
        a[i] = acc1;
        
        /* Chain 2: distance 1 recurrence with different ops */
        acc2 = (acc2 << 2) - b[i] * 5;
        b[i] = acc2;
        
        /* Chain 3: distance 2 recurrence (uses i-2) */
        if (i >= 2) {
            acc3 = acc3 + c[i-2] * 11;
            c[i] = acc3;
        }
        
        /* Chain 4: simple accumulation with mixing */
        acc4 = acc4 ^ (d[i] * 13);
        d[i] = acc4;
        
        /* Additional operations to increase register pressure */
        int temp1 = a[i] & b[i];
        int temp2 = c[i] | d[i];
        int temp3 = temp1 * temp2;
        global_acc += temp3;
    }
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_recurrence(float *fa, float *fb, double *da, double *db, int n) {
    float f_acc1 = fa[0];
    float f_acc2 = fb[0];
    double d_acc1 = da[0];
    double d_acc2 = db[0];
    
    for (int i = 1; i < n; i++) {
        /* Float chain 1: linear recurrence */
        f_acc1 = f_acc1 * 1.5f + fa[i] * 2.3f;
        fa[i] = f_acc1;
        
        /* Float chain 2: different recurrence pattern */
        f_acc2 = f_acc2 / 1.7f - fb[i] * 0.8f;
        fb[i] = f_acc2;
        
        /* Double chain 1: accumulation with mixing */
        d_acc1 = d_acc1 * 2.0 + da[i] * 3.0;
        da[i] = d_acc1;
        
        /* Double chain 2: more complex recurrence */
        d_acc2 = d_acc2 + db[i-1] * db[i] * 0.5;
        db[i] = d_acc2;
        
        /* Cross-type operations to increase pressure */
        global_acc += (long long)(f_acc1 * d_acc1) + (long long)(f_acc2 * d_acc2);
    }
}

/* Test 3: Pointer-chasing with strided access patterns */
void test_pointer_chasing(int *arr, int n, int stride) {
    int *ptr1 = arr;
    int *ptr2 = arr + stride;
    int *ptr3 = arr + 2*stride;
    int *end = arr + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    while (ptr3 < end) {
        /* Three independent pointer-chasing chains */
        sum1 = sum1 * 3 + *ptr1;
        sum2 = sum2 * 5 + *ptr2;
        sum3 = sum3 * 7 + *ptr3;
        
        *ptr1 = sum1;
        *ptr2 = sum2;
        *ptr3 = sum3;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Additional computation to use results */
        int mix = (sum1 & sum2) | (sum3 << 1);
        global_acc += mix;
    }
}

/* Test 4: Mixed integer operations with manual unrolling */
void test_mixed_unrolled(short *sarr, int *iarr, long long *larr, int n) {
    short s_acc1 = sarr[0], s_acc2 = sarr[1];
    int i_acc1 = iarr[0], i_acc2 = iarr[1];
    long long l_acc1 = larr[0], l_acc2 = larr[1];
    
    /* Manual unrolling to increase operations per iteration */
    for (int i = 2; i < n - 2; i += 2) {
        /* Process two elements at once */
        
        /* Short array operations */
        s_acc1 = s_acc1 * 2 + sarr[i];
        s_acc2 = s_acc2 * 3 + sarr[i+1];
        sarr[i] = s_acc1;
        sarr[i+1] = s_acc2;
        
        /* Integer array operations */
        i_acc1 = (i_acc1 << 1) + iarr[i] * 5;
        i_acc2 = (i_acc2 << 2) + iarr[i+1] * 7;
        iarr[i] = i_acc1;
        iarr[i+1] = i_acc2;
        
        /* Long long array operations */
        l_acc1 = l_acc1 + larr[i] * 11;
        l_acc2 = l_acc2 + larr[i+1] * 13;
        larr[i] = l_acc1;
        larr[i+1] = l_acc2;
        
        /* Cross-type mixing */
        global_acc += (s_acc1 * i_acc1) + (s_acc2 * i_acc2) + (l_acc1 >> 4) + (l_acc2 >> 4);
    }
}

/* Test 5: Complex recurrence with variable dependency distance */
void test_variable_distance(int *arr, int *indices, int n) {
    int acc[4] = {arr[0], arr[1], arr[2], arr[3]};
    
    for (int i = 4; i < n; i++) {
        /* Variable dependency distance based on index array */
        int dist1 = indices[i] % 3 + 1;
        int dist2 = indices[i] % 2 + 1;
        
        /* Recurrence with variable distance */
        if (i >= dist1) {
            acc[0] = acc[0] + arr[i-dist1] * 3;
        }
        if (i >= dist2) {
            acc[1] = acc[1] + arr[i-dist2] * 5;
        }
        
        /* Fixed distance recurrences */
        acc[2] = acc[2] * 2 + arr[i-1];
        acc[3] = acc[3] * 3 - arr[i-2];
        
        /* Combine results */
        arr[i] = acc[0] + acc[1] + acc[2] + acc[3];
        
        /* Update accumulators */
        acc[0] = acc[0] ^ arr[i];
        acc[1] = acc[1] | arr[i];
        acc[2] = acc[2] & arr[i];
        acc[3] = acc[3] + arr[i];
        
        global_acc += arr[i];
    }
}

/* Test 6: PowerPC-specific double precision operations */
#ifdef __powerpc__
void test_powerpc_double(double *d1, double *d2, double *d3, double *d4, int n) {
    double acc1 = d1[0];
    double acc2 = d2[0];
    double acc3 = d3[0];
    double acc4 = d4[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple double precision recurrence chains */
        acc1 = acc1 * 1.1 + d1[i] * 2.2;
        d1[i] = acc1;
        
        acc2 = acc2 / 1.3 - d2[i] * 3.3;
        d2[i] = acc2;
        
        acc3 = acc3 * 2.5 + d3[i-1] * d3[i];
        d3[i] = acc3;
        
        acc4 = acc4 + d4[i] * 4.4 - d4[i-1] * 1.7;
        d4[i] = acc4;
        
        /* Use results to prevent elimination */
        double mix = acc1 * acc2 + acc3 * acc4;
        global_acc += (long long)mix;
    }
}
#endif

/* Test 7: Loop with if-converted operations */
void test_if_converted(int *a, int *b, int *c, int n) {
    int acc1 = a[0];
    int acc2 = b[0];
    int acc3 = c[0];
    
    for (int i = 1; i < n; i++) {
        /* Operations that could be if-converted */
        int diff = a[i] - b[i];
        int abs_diff = diff > 0 ? diff : -diff;
        
        /* Multiple recurrence chains */
        acc1 = acc1 + abs_diff * 3;
        acc2 = acc2 * 2 - diff;
        acc3 = acc3 ^ (abs_diff * diff);
        
        a[i] = acc1;
        b[i] = acc2;
        c[i] = acc3;
        
        global_acc += acc1 + acc2 + acc3;
    }
}

/* Initialize arrays with pattern */
void init_arrays(int *a, int *b, int *c, int *d, 
                 float *fa, float *fb, double *da, double *db,
                 short *sarr, long long *larr, int *indices, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 + 2;
        c[i] = i * 7 + 3;
        d[i] = i * 11 + 4;
        fa[i] = i * 1.3f;
        fb[i] = i * 2.7f;
        da[i] = i * 3.1;
        db[i] = i * 4.3;
        sarr[i] = i * 2;
        larr[i] = i * 1000LL;
        indices[i] = (i * 13) % 10;
    }
}

int main() {
    /* Allocate arrays */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    float *fa = malloc(SIZE * sizeof(float));
    float *fb = malloc(SIZE * sizeof(float));
    double *da = malloc(SIZE * sizeof(double));
    double *db = malloc(SIZE * sizeof(double));
    short *sarr = malloc(SIZE * sizeof(short));
    long long *larr = malloc(SIZE * sizeof(long long));
    int *indices = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c || !d || !fa || !fb || !da || !db || !sarr || !larr || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, c, d, fa, fb, da, db, sarr, larr, indices, SIZE);
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify inputs slightly each iteration */
        a[0] += iter;
        b[0] += iter * 2;
        
        /* Test 1: Multiple integer recurrence chains */
        test_multi_recurrence_int(a, b, c, d, SIZE);
        
        /* Test 2: Floating-point accumulation */
        test_float_recurrence(fa, fb, da, db, SIZE);
        
        /* Test 3: Pointer-chasing */
        test_pointer_chasing(a, SIZE, 4);
        
        /* Test 4: Mixed operations with unrolling */
        test_mixed_unrolled(sarr, b, larr, SIZE);
        
        /* Test 5: Variable distance recurrence */
        test_variable_distance(c, indices, SIZE);
        
        /* Test 6: PowerPC-specific if available */
        #ifdef __powerpc__
        test_powerpc_double(da, db, da, db, SIZE);
        #endif
        
        /* Test 7: If-converted operations */
        test_if_converted(a, b, d, SIZE);
    }
    
    printf("Tests completed. Global accumulator: %lld\n", global_acc);
    
    /* Clean up */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb); free(da); free(db);
    free(sarr); free(larr); free(indices);
    
    return 0;
}
