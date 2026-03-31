/* test_modulo_sched.c
 * 
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test_modulo_sched.c -o test_modulo_sched
 * 
 * For ARM SVE: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve test_modulo_sched.c -o test_modulo_sched
 * 
 * For RISC-V: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=rv64gcv test_modulo_sched.c -o test_modulo_sched
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

volatile int global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multiple_recurrence_chains(int *a, int *b, int *c, int *d, int n) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1 = 5, y2 = 6, y3 = 7, y4 = 8;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent recurrence chains */
        x1 = x1 * 3 + a[i];      /* Chain 1: distance 1 */
        x2 = x2 * 5 + b[i];      /* Chain 2: distance 1 */
        x3 = x3 * 7 + c[i];      /* Chain 3: distance 1 */
        x4 = x4 * 11 + d[i];     /* Chain 4: distance 1 */
        
        /* Cross-chain dependencies to increase pressure */
        y1 = y1 + x1 * 2;
        y2 = y2 + x2 * 3;
        y3 = y3 + x3 * 4;
        y4 = y4 + x4 * 5;
        
        /* More operations to use registers */
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
        sum1 = sum1 * 1.1f + fa[i];      /* Distance 1 */
        sum2 = sum2 * 1.2f + fb[i];      /* Distance 1 */
        sum3 = sum3 * 1.3f + fc[i];      /* Distance 1 */
        
        /* Cross dependencies */
        prod1 = prod1 * (sum1 + 0.5f);
        prod2 = prod2 * (sum2 + 0.7f);
        
        /* More operations to increase register pressure */
        fa[i] = sum1 * prod1;
        fb[i] = sum2 * prod2;
        fc[i] = sum3 + prod1 + prod2;
    }
    
    global_sum += (int)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *arr, int n, int stride) {
    int *ptr1 = arr;
    int *ptr2 = arr + stride;
    int *ptr3 = arr + 2 * stride;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple pointer-chasing chains */
        acc1 = acc1 * 2 + *ptr1;
        acc2 = acc2 * 3 + *ptr2;
        acc3 = acc3 * 5 + *ptr3;
        
        /* Update pointers with stride */
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Additional operations */
        *ptr1 = acc1 & 0xFF;
        *ptr2 = acc2 & 0xFF;
        *ptr3 = acc3 & 0xFF;
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_operations(int *a, int *b, int *c, int n) {
    int t1 = a[0], t2 = b[0], t3 = c[0];
    int u1 = 1, u2 = 2, u3 = 3, u4 = 4;
    
    for (int i = 1; i < n; i++) {
        /* Different dependency distances */
        t1 = t1 + a[i];           /* Distance 1 */
        t2 = t2 * 3 + b[i-1];     /* Distance 1 */
        t3 = (t3 << 2) ^ c[i];    /* Distance 1 */
        
        /* More chains with different operations */
        u1 = u1 + (t1 & 0xF);
        u2 = u2 ^ (t2 | 0xAA);
        u3 = u3 * (t3 + 1);
        u4 = u4 - (t1 ^ t2 ^ t3);
        
        /* Store results */
        a[i] = t1 + u1;
        b[i] = t2 + u2;
        c[i] = t3 + u3 + u4;
    }
    
    global_sum += t1 + t2 + t3 + u1 + u2 + u3 + u4;
}

/* Test 5: Double precision floating point for PowerPC */
void test_double_precision(double *da, double *db, double *dc, int n) {
    double d1 = 0.1, d2 = 0.2, d3 = 0.3;
    double d4 = 1.0, d5 = 2.0, d6 = 3.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double precision chains */
        d1 = d1 * 1.01 + da[i];
        d2 = d2 * 1.02 + db[i];
        d3 = d3 * 1.03 + dc[i];
        
        /* Cross dependencies */
        d4 = d4 / (d1 + 0.5);
        d5 = d5 * (d2 - 0.3);
        d6 = d6 + (d3 * 0.7);
        
        /* Store back */
        da[i] = d1 * d4;
        db[i] = d2 * d5;
        dc[i] = d3 * d6;
    }
    
    global_sum += (int)(d1 + d2 + d3 + d4 + d5 + d6);
}

/* Test 6: Manual unrolling to increase operations per iteration */
void test_manual_unroll(int *a, int *b, int n) {
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    int p1 = 1, p2 = 1;
    
    /* Manual unrolling by 4 */
    for (int i = 0; i < n - 3; i += 4) {
        /* Unrolled iteration 1 */
        s1 = s1 * 3 + a[i];
        p1 = p1 * (s1 + 1);
        a[i] = s1 ^ p1;
        
        /* Unrolled iteration 2 */
        s2 = s2 * 5 + b[i+1];
        p2 = p2 * (s2 + 2);
        b[i+1] = s2 ^ p2;
        
        /* Unrolled iteration 3 */
        s1 = s1 * 7 + a[i+2];
        p1 = p1 * (s1 + 3);
        a[i+2] = s1 ^ p1;
        
        /* Unrolled iteration 4 */
        s2 = s2 * 11 + b[i+3];
        p2 = p2 * (s2 + 4);
        b[i+3] = s2 ^ p2;
        
        /* Cross dependencies between unrolled iterations */
        s3 = s3 + s1 * s2;
        s4 = s4 ^ (p1 | p2);
    }
    
    global_sum += s1 + s2 + s3 + s4 + p1 + p2;
}

/* Test 7: Array accumulation with multiple dependency patterns */
void test_array_accumulation(int *arr, int n) {
    int acc[4] = {1, 2, 3, 4};
    
    for (int i = 1; i < n; i++) {
        /* Multiple array-based recurrence chains */
        acc[0] = acc[0] + arr[i-1] * 2;      /* Distance 1 */
        acc[1] = acc[1] * 3 + arr[i];        /* Distance 0 */
        acc[2] = (acc[2] << 1) ^ acc[0];     /* Distance 0 from acc[0] */
        acc[3] = acc[3] - acc[1] + acc[2];   /* Distance 0 from acc[1], acc[2] */
        
        /* Store with transformation */
        arr[i] = (acc[0] + acc[1]) * (acc[2] - acc[3]);
    }
    
    global_sum += acc[0] + acc[1] + acc[2] + acc[3];
}

int main() {
    /* Initialize arrays with test data */
    int *int_arr1 = malloc(SIZE * sizeof(int));
    int *int_arr2 = malloc(SIZE * sizeof(int));
    int *int_arr3 = malloc(SIZE * sizeof(int));
    int *int_arr4 = malloc(SIZE * sizeof(int));
    
    float *float_arr1 = malloc(SIZE * sizeof(float));
    float *float_arr2 = malloc(SIZE * sizeof(float));
    float *float_arr3 = malloc(SIZE * sizeof(float));
    
    double *double_arr1 = malloc(SIZE * sizeof(double));
    double *double_arr2 = malloc(SIZE * sizeof(double));
    double *double_arr3 = malloc(SIZE * sizeof(double));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        int_arr1[i] = rand() % 100;
        int_arr2[i] = rand() % 100;
        int_arr3[i] = rand() % 100;
        int_arr4[i] = rand() % 100;
        
        float_arr1[i] = (float)(rand() % 100) / 10.0f;
        float_arr2[i] = (float)(rand() % 100) / 10.0f;
        float_arr3[i] = (float)(rand() % 100) / 10.0f;
        
        double_arr1[i] = (double)(rand() % 100) / 10.0;
        double_arr2[i] = (double)(rand() % 100) / 10.0;
        double_arr3[i] = (double)(rand() % 100) / 10.0;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_multiple_recurrence_chains(int_arr1, int_arr2, int_arr3, int_arr4, SIZE);
        test_float_accumulation(float_arr1, float_arr2, float_arr3, SIZE);
        test_pointer_chasing(int_arr1, SIZE, 4);
        test_mixed_operations(int_arr2, int_arr3, int_arr4, SIZE);
        test_double_precision(double_arr1, double_arr2, double_arr3, SIZE);
        test_manual_unroll(int_arr1, int_arr2, SIZE);
        test_array_accumulation(int_arr3, SIZE);
    }
    
    /* Output result to prevent dead code elimination */
    printf("Final result: %d\n", global_sum);
    
    /* Cleanup */
    free(int_arr1);
    free(int_arr2);
    free(int_arr3);
    free(int_arr4);
    free(float_arr1);
    free(float_arr2);
    free(float_arr3);
    free(double_arr1);
    free(double_arr2);
    free(double_arr3);
    
    return 0;
}
