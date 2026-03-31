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

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains to increase register pressure */
    int acc1 = a[0];
    int acc2 = a[1];
    int acc3 = a[2];
    int acc4 = a[3];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: Simple recurrence */
        acc1 = acc1 * 3 + b[i];
        a[i] = acc1;
        
        /* Chain 2: Recurrence with offset */
        acc2 = (acc2 + c[i]) * 5;
        b[i] = acc2;
        
        /* Chain 3: More complex recurrence */
        acc3 = (acc3 << 2) ^ d[i];
        c[i] = acc3;
        
        /* Chain 4: Mixed operations recurrence */
        acc4 = (acc4 & 0xFFFF) * 7 + (d[i] >> 2);
        d[i] = acc4;
        
        /* Additional operations to increase register pressure */
        a[i] += (b[i] & 0xFF) | (c[i] & 0xFF00);
        d[i] ^= (acc1 + acc2) & 0xFFFFFF;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Test 2: Floating-point accumulation with carried dependencies */
void test_float_accumulation(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0];
    double sum2 = b[0];
    double sum3 = c[0];
    double prod1 = 1.0;
    double prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i];
        sum2 = sum2 * 0.99 + b[i];
        sum3 = sum3 + c[i] * sum1;
        
        prod1 = prod1 * (1.0 + a[i] * 0.001);
        prod2 = prod2 * (1.0 - b[i] * 0.001);
        
        /* Cross-chain dependencies */
        a[i] = sum1 + prod1;
        b[i] = sum2 * prod2;
        c[i] = sum3 + a[i] * b[i];
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Mixed integer/float with strided access */
void test_mixed_strided(float *farr, int *iarr, long *larr, int n) {
    int i;
    float f_acc = farr[0];
    int i_acc = iarr[0];
    long l_acc = larr[0];
    
    /* Strided access pattern */
    for (i = 4; i < n; i += 2) {
        /* Integer recurrence chain */
        i_acc = i_acc * 13 + iarr[i-2];
        iarr[i] = i_acc;
        
        /* Float recurrence chain */
        f_acc = f_acc * 1.5f + farr[i-4];
        farr[i] = f_acc;
        
        /* Long integer recurrence with dependency on both */
        l_acc = l_acc + (long)(f_acc * i_acc);
        larr[i] = l_acc;
        
        /* Additional operations for register pressure */
        iarr[i-1] = (int)(f_acc) ^ i_acc;
        farr[i-1] = (float)(l_acc & 0xFFFF) * 0.01f;
    }
    
    global_acc += i_acc + (long long)f_acc + l_acc;
}

/* Test 4: Pointer chasing with arithmetic */
void test_pointer_chasing(int *data, int n) {
    int *ptr1 = data;
    int *ptr2 = data + 1;
    int *ptr3 = data + 2;
    int *end = data + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer chasing chains */
        sum1 = sum1 * 11 + *ptr1;
        sum2 = sum2 * 7 + *ptr2;
        sum3 = sum3 * 5 + *ptr3;
        
        /* Update pointers with different strides */
        *ptr1 = sum1 & 0xFF;
        *ptr2 = sum2 & 0xFF;
        *ptr3 = sum3 & 0xFF;
        
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computation between pointer accesses */
        sum1 ^= sum2;
        sum2 += sum3;
        sum3 -= sum1;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 5: Nested loops with inner loop carried dependency */
void test_nested_loops(int *mat, int rows, int cols) {
    int i, j;
    int row_acc = 0;
    
    for (i = 1; i < rows; i++) {
        int col_acc = mat[i * cols];
        
        /* Inner loop with carried dependency */
        for (j = 1; j < cols; j++) {
            int idx = i * cols + j;
            /* Dependency on previous column in same row */
            col_acc = col_acc * 3 + mat[idx - 1];
            mat[idx] = col_acc;
            
            /* Dependency on previous row same column */
            int prev_row_val = mat[(i-1) * cols + j];
            mat[idx] += prev_row_val * 2;
            
            /* Additional operations */
            mat[idx] ^= (col_acc << (j & 3));
        }
        
        row_acc += col_acc;
    }
    
    global_acc += row_acc;
}

/* Test 6: SIMD-style operations (triggers vectorization + modulo scheduling) */
void test_simd_style(short *src, int *dst, int n) {
    int i;
    int acc0 = 0, acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Process 4 elements at a time */
    for (i = 0; i < n - 3; i += 4) {
        /* Independent chains for each lane */
        acc0 = acc0 * 3 + src[i];
        acc1 = acc1 * 5 + src[i+1];
        acc2 = acc2 * 7 + src[i+2];
        acc3 = acc3 * 11 + src[i+3];
        
        /* Store with cross-lane mixing */
        dst[i] = acc0 + acc1;
        dst[i+1] = acc1 + acc2;
        dst[i+2] = acc2 + acc3;
        dst[i+3] = acc3 + acc0;
        
        /* Additional mixing */
        acc0 ^= acc1;
        acc1 += acc2;
        acc2 ^= acc3;
        acc3 += acc0;
    }
    
    global_acc += acc0 + acc1 + acc2 + acc3;
}

/* Test 7: PowerPC specific - double precision with FMA-like pattern */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    int i;
    double sum = a[0];
    double prod = b[0];
    
    for (i = 1; i < n; i++) {
        /* Pattern that encourages FMA usage */
        double t1 = sum * 1.5;
        double t2 = prod * 2.0;
        sum = t1 + a[i] * b[i];
        prod = t2 * c[i];
        
        /* Cross dependency */
        a[i] = sum + prod;
        b[i] = sum - prod;
        c[i] = sum * prod;
        
        /* Additional operations for register pressure */
        sum = sum * 0.99 + a[i] * 0.01;
        prod = prod * 1.01 - b[i] * 0.01;
    }
    
    global_acc += (long long)(sum + prod);
}
#endif

/* Test 8: Compile-time unrolled loop with dependencies */
#pragma GCC unroll 4
void test_unrolled_deps(int *a, int *b, int n) {
    int i;
    int acc1 = a[0], acc2 = b[0], acc3 = a[1], acc4 = b[1];
    
    for (i = 2; i < n - 2; i += 2) {
        /* Unrolled dependency chains */
        acc1 = acc1 * 2 + a[i];
        acc2 = acc2 * 3 + b[i];
        a[i] = acc1 ^ acc2;
        
        acc3 = acc3 * 5 + a[i+1];
        acc4 = acc4 * 7 + b[i+1];
        b[i+1] = acc3 & acc4;
        
        /* Cross dependencies between unrolled iterations */
        acc1 += acc3;
        acc2 ^= acc4;
        acc3 -= acc1;
        acc4 |= acc2;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Initialize arrays with pattern */
void init_arrays(int *a, int *b, int *c, int *d, 
                 double *fa, double *fb, double *fc,
                 float *ff, long *la, short *sa, int size) {
    int i;
    for (i = 0; i < size; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
        c[i] = (i * 5) % 100;
        d[i] = (i * 7) % 100;
        
        fa[i] = i * 0.1;
        fb[i] = i * 0.2;
        fc[i] = i * 0.3;
        
        ff[i] = i * 0.5f;
        la[i] = i * 10L;
        sa[i] = i % 32767;
    }
}

int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    
    double *fa = malloc(SIZE * sizeof(double));
    double *fb = malloc(SIZE * sizeof(double));
    double *fc = malloc(SIZE * sizeof(double));
    
    float *ff = malloc(SIZE * sizeof(float));
    long *la = malloc(SIZE * sizeof(long));
    short *sa = malloc(SIZE * sizeof(short));
    
    int *mat = malloc(100 * 100 * sizeof(int));
    
    if (!a || !b || !c || !d || !fa || !fb || !fc || !ff || !la || !sa || !mat) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, d, fa, fb, fc, ff, la, sa, SIZE);
    
    printf("Starting modulo scheduling tests...\n");
    start = clock();
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        test_int_recurrence_multi_chain(a, b, c, d, SIZE);
        test_float_accumulation(fa, fb, fc, SIZE);
        test_mixed_strided(ff, a, la, SIZE);
        test_pointer_chasing(a, SIZE);
        test_nested_loops(mat, 100, 100);
        test_simd_style(sa, a, SIZE);
        test_unrolled_deps(a, b, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_double(fa, fb, fc, SIZE);
        #endif
        
        /* Modify inputs slightly each iteration */
        a[0] = (a[0] + 1) % 100;
        fa[0] += 0.001;
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Tests completed in %.2f seconds\n", cpu_time_used);
    printf("Global accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb); free(fc);
    free(ff); free(la); free(sa);
    free(mat);
    
    return 0;
}
