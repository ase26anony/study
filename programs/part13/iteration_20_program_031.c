/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test_modulo_sched.c -o test_modulo_sched
 * For ARM SVE: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve test_modulo_sched.c -o test_modulo_sched
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multichain(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5, u = 6;
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent recurrence chains to increase register pressure */
        x = x * 13 + a[i];          /* Chain 1: distance 1 dependency */
        y = y * 17 + b[i] + x;      /* Chain 2: depends on x and previous y */
        z = z * 19 + c[i] + y;      /* Chain 3: depends on y and previous z */
        w = w * 23 + a[i-1] + z;    /* Chain 4: depends on z and previous w */
        v = v * 29 + b[i-1] + w;    /* Chain 5: depends on w and previous v */
        u = u * 31 + c[i-1] + v;    /* Chain 6: depends on v and previous u */
        
        /* Cross-chain operations to create anti-dependencies */
        a[i] = x + y;
        b[i] = y + z;
        c[i] = z + w;
    }
    
    global_acc += x + y + z + w + v + u;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i];      /* Distance 1 true dependency */
        sum2 = sum2 * 1.02 + b[i] * sum1;
        sum3 = sum3 * 1.03 + c[i] * sum2;
        
        /* Independent product chains */
        prod1 = prod1 * (1.0 + a[i] * 0.001);
        prod2 = prod2 * (1.0 + b[i] * 0.002);
        
        /* Cross-store operations */
        a[i] = sum1 + prod1;
        b[i] = sum2 + prod2;
        c[i] = sum3 + a[i-1];
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    
    while (ptr1 < end && ptr2 < end && ptr3 < end) {
        /* Multiple pointer-based recurrence chains */
        acc1 = acc1 * 7 + *ptr1 + tmp3;    /* Anti-dependency on tmp3 */
        acc2 = acc2 * 11 + *ptr2 + tmp1;   /* Anti-dependency on tmp1 */
        acc3 = acc3 * 13 + *ptr3 + tmp2;   /* Anti-dependency on tmp2 */
        
        /* Register moves needed for these swaps */
        tmp1 = acc1;
        tmp2 = acc2;
        tmp3 = acc3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
    }
    
    global_acc += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with shift and mask */
void test_mixed_operations(unsigned int *arr, int n) {
    unsigned int x = 0x12345678;
    unsigned int y = 0x9ABCDEF0;
    unsigned int z = 0x13579BDF;
    
    for (int i = 0; i < n; i++) {
        /* Complex operations with multiple uses of each variable */
        unsigned int t1 = (x >> 3) | (y << 5);
        unsigned int t2 = (y & 0xFF00FF00) ^ (z & 0x00FF00FF);
        unsigned int t3 = (z * 3) + (x & 0x0000FFFF);
        
        /* Recurrence with distance 1 */
        x = x * 0x9E3779B9 + t1 + arr[i];
        y = y * 0x9E3779B9 + t2 + x;
        z = z * 0x9E3779B9 + t3 + y;
        
        /* Store with anti-dependency */
        arr[i] = (x + y + z) & 0xFFFFFFFF;
    }
    
    global_acc += x + y + z;
}

/* Test 5: Nested loops with inner loop being modulo scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    for (int j = 0; j < m; j++) {
        int acc = a[j];
        int prod = 1;
        
        /* This inner loop should trigger modulo scheduling */
        for (int i = 1; i < n; i++) {
            /* Multiple operations with carried dependencies */
            int t1 = acc * 3 + b[i + j * n];
            int t2 = prod * 5 + c[i + j * n];
            int t3 = t1 ^ t2;
            
            acc = t1 + t3;
            prod = t2 * t3;
            
            /* Anti-dependency through array access */
            a[i + j * n] = acc + prod + a[i-1 + j * n];
        }
        
        global_acc += acc + prod;
    }
}

/* Test 6: PowerPC specific operations (if compiled for PowerPC) */
#ifdef __powerpc__
void test_powerpc_specific(double *a, double *b, int n) {
    double sum = 0.0;
    double prod = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* FP operations that use multiple FP registers */
        double t1 = sum * 1.1 + a[i];
        double t2 = prod * 1.2 + b[i];
        double t3 = t1 * t2 - sum;
        double t4 = t1 / t2 + prod;
        
        sum = t3 + a[i-1];
        prod = t4 * b[i-1];
        
        /* Force register moves */
        a[i] = sum;
        b[i] = prod;
    }
    
    global_acc += (long long)(sum + prod);
}
#endif

/* Test 7: Loop with pragma unroll to increase register pressure */
void test_unrolled_loop(int *a, int *b, int n) {
    int x = 1, y = 2, z = 3;
    
    #pragma GCC unroll 4
    for (int i = 1; i < n; i++) {
        /* Unrolled operations create more simultaneous live values */
        x = x * 11 + a[i];
        y = y * 13 + b[i] + x;
        z = z * 17 + a[i-1] + y;
        
        int t = x + y;
        x = y + z;
        y = z + t;
        z = t + x;
        
        a[i] = x;
        b[i] = y;
    }
    
    global_acc += x + y + z;
}

/* Main driver */
int main() {
    /* Allocate and initialize arrays */
    int *int_data1 = malloc(SIZE * sizeof(int));
    int *int_data2 = malloc(SIZE * sizeof(int));
    int *int_data3 = malloc(SIZE * sizeof(int));
    unsigned int *uint_data = malloc(SIZE * sizeof(unsigned int));
    double *double_data1 = malloc(SIZE * sizeof(double));
    double *double_data2 = malloc(SIZE * sizeof(double));
    double *double_data3 = malloc(SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        int_data1[i] = rand() % 100;
        int_data2[i] = rand() % 100;
        int_data3[i] = rand() % 100;
        uint_data[i] = rand();
        double_data1[i] = (double)rand() / RAND_MAX;
        double_data2[i] = (double)rand() / RAND_MAX;
        double_data3[i] = (double)rand() / RAND_MAX;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_recurrence_multichain(int_data1, int_data2, int_data3, SIZE);
        test_float_accumulate(double_data1, double_data2, double_data3, SIZE);
        test_pointer_chasing(int_data1, SIZE, 4);
        test_mixed_operations(uint_data, SIZE);
        test_nested_loops(int_data1, int_data2, int_data3, 64, 16);
        test_unrolled_loop(int_data2, int_data3, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_specific(double_data1, double_data2, SIZE);
        #endif
    }
    
    printf("Final accumulator value: %lld\n", global_acc);
    printf("Check dump files for '--(T,' pattern to verify coverage\n");
    
    /* Cleanup */
    free(int_data1);
    free(int_data2);
    free(int_data3);
    free(uint_data);
    free(double_data1);
    free(double_data2);
    free(double_data3);
    
    return 0;
}
