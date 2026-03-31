/* test_modulo_sched.c - Test program for GCC modulo scheduling register moves */
/* Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    for (i = 2; i < n; i++) {
        /* Chain 1: x1 depends on previous x1 and x2 */
        x1 = (x1 * 3 + x2) ^ a[i];
        x2 = (x2 * 5 + x1) ^ b[i];
        
        /* Chain 2: y1 depends on previous y1 and y2 with distance 1 */
        y1 = (y1 << 2) + y2 + c[i];
        y2 = (y2 << 1) + y1 + d[i];
        
        /* Chain 3: Mixed operations creating more register pressure */
        x3 = (x3 & 0xFFFF) * 7 + x4;
        x4 = (x4 | 0xFF) * 11 + x3;
        
        /* Chain 4: Additional arithmetic chains */
        y3 = y3 * 13 + y4 * 17;
        y4 = y4 * 19 + y3 * 23;
        
        /* Store results to prevent optimization */
        a[i] = x1 + y1;
        b[i] = x2 + y2;
        c[i] = x3 + y3;
        d[i] = x4 + y4;
    }
    
    global_acc += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_recurrence(double *a, double *b, double *c, int n) {
    int i;
    double x = a[0], y = b[0], z = c[0];
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        x = x * 1.5 + a[i] * 2.0;
        y = y * 2.0 + b[i] * 1.5;
        z = z * 2.5 + c[i] * 0.5;
        
        /* Additional accumulation chains */
        acc1 = acc1 + x * y;
        acc2 = acc2 + y * z;
        acc3 = acc3 + z * x;
        
        /* Cross-chain dependencies */
        x = x + acc1 * 0.1;
        y = y + acc2 * 0.2;
        z = z + acc3 * 0.3;
    }
    
    global_acc += (long long)(x + y + z + acc1 + acc2 + acc3);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int i;
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (i = 0; i < n - 3 * stride; i++) {
        /* Multiple pointer chasing chains */
        sum1 = sum1 * 3 + *ptr1;
        sum2 = sum2 * 5 + *ptr2;
        sum3 = sum3 * 7 + *ptr3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Cross-dependencies between sums */
        sum1 = sum1 ^ sum2;
        sum2 = sum2 ^ sum3;
        sum3 = sum3 ^ sum1;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer/float with unrolled loops */
#pragma GCC unroll 4
void test_mixed_unrolled(float *fa, int *ia, double *da, int n) {
    int i;
    float f1 = fa[0], f2 = fa[1];
    int i1 = ia[0], i2 = ia[1];
    double d1 = da[0], d2 = da[1];
    
    for (i = 2; i < n; i++) {
        /* Mixed type operations creating register pressure */
        f1 = f1 * 1.1f + (float)ia[i];
        f2 = f2 * 1.2f + (float)ia[i-1];
        
        i1 = i1 * 3 + (int)fa[i];
        i2 = i2 * 5 + (int)fa[i-1];
        
        d1 = d1 * 1.5 + (double)ia[i];
        d2 = d2 * 2.0 + (double)ia[i-1];
        
        /* Cross-type dependencies */
        fa[i] = f1 + (float)d1;
        ia[i] = i1 + (int)f2;
        da[i] = d1 + (double)i2;
    }
    
    global_acc += (long long)(f1 + f2 + i1 + i2 + d1 + d2);
}

/* Test 5: PowerPC-specific double operations */
#ifdef __powerpc__ || __PPC__
void test_powerpc_double(double *a, double *b, int n) {
    int i;
    double x = a[0], y = b[0];
    double z = x * y;
    
    for (i = 1; i < n; i++) {
        /* Multiple double precision chains */
        x = x * 1.25 + a[i];
        y = y * 1.75 + b[i];
        z = z * 2.0 + x * y;
        
        /* Additional operations to increase register pressure */
        double t1 = x * 3.14159;
        double t2 = y * 2.71828;
        double t3 = z * 1.41421;
        
        x = x + t1 * t2;
        y = y + t2 * t3;
        z = z + t3 * t1;
    }
    
    global_acc += (long long)(x + y + z);
}
#endif

/* Test 6: Array accumulation with variable distance */
void test_variable_distance(int *a, int *b, int *c, int n, int dist) {
    int i;
    /* Multiple accumulators with carried dependencies */
    int acc1 = a[0], acc2 = b[0], acc3 = c[0];
    int tmp1 = a[1], tmp2 = b[1], tmp3 = c[1];
    
    for (i = dist; i < n; i++) {
        /* Dependencies with variable distance */
        acc1 = acc1 * 2 + a[i] - a[i-dist];
        acc2 = acc2 * 3 + b[i] - b[i-dist];
        acc3 = acc3 * 5 + c[i] - c[i-dist];
        
        /* Cross-accumulator operations */
        tmp1 = tmp1 + acc1 * acc2;
        tmp2 = tmp2 + acc2 * acc3;
        tmp3 = tmp3 + acc3 * acc1;
        
        /* Feedback into accumulators */
        acc1 = acc1 ^ tmp1;
        acc2 = acc2 ^ tmp2;
        acc3 = acc3 ^ tmp3;
    }
    
    global_acc += acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *ia1, int *ia2, int *ia3, int *ia4,
                 float *fa, double *da1, double *da2, int n) {
    int i;
    for (i = 0; i < n; i++) {
        ia1[i] = (i * 13) % 100;
        ia2[i] = (i * 17) % 100;
        ia3[i] = (i * 19) % 100;
        ia4[i] = (i * 23) % 100;
        fa[i] = (float)(i % 50) * 0.5f;
        da1[i] = (double)(i % 30) * 0.3;
        da2[i] = (double)(i % 40) * 0.4;
    }
}

int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate arrays */
    int *ia1 = malloc(SIZE * sizeof(int));
    int *ia2 = malloc(SIZE * sizeof(int));
    int *ia3 = malloc(SIZE * sizeof(int));
    int *ia4 = malloc(SIZE * sizeof(int));
    float *fa = malloc(SIZE * sizeof(float));
    double *da1 = malloc(SIZE * sizeof(double));
    double *da2 = malloc(SIZE * sizeof(double));
    
    if (!ia1 || !ia2 || !ia3 || !ia4 || !fa || !da1 || !da2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(ia1, ia2, ia3, ia4, fa, da1, da2, SIZE);
    
    start = clock();
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (i = 0; i < ITERS; i++) {
        /* Vary parameters slightly each iteration */
        int dist = 1 + (i % 3);
        int stride = 1 + (i % 4);
        
        test_multi_recurrence_int(ia1, ia2, ia3, ia4, SIZE);
        test_float_recurrence(da1, da2, da1, SIZE);  /* Use da1 for both a and c */
        test_pointer_chasing(ia1, SIZE, stride);
        test_mixed_unrolled(fa, ia1, da1, SIZE);
        test_variable_distance(ia1, ia2, ia3, SIZE, dist);
        
        #ifdef __powerpc__ || __PPC__
        test_powerpc_double(da1, da2, SIZE);
        #endif
        
        /* Modify array data slightly to prevent pattern optimization */
        if (i % 100 == 0) {
            ia1[0] += 1;
            da1[0] += 0.1;
        }
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Modulo scheduling test completed.\n");
    printf("Global accumulator: %lld\n", global_acc);
    printf("Time used: %.2f seconds\n", cpu_time_used);
    printf("Iterations: %d\n", ITERS);
    
    /* Clean up */
    free(ia1);
    free(ia2);
    free(ia3);
    free(ia4);
    free(fa);
    free(da1);
    free(da2);
    
    return 0;
}
