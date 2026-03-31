/* test-modulo-sched.c
 * Comprehensive test to trigger GCC's modulo scheduler register move logic
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test-modulo-sched.c -o test-modulo-sched
 * For ARM SVE: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve test-modulo-sched.c -o test-modulo-sched
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Volatile to prevent optimization */
volatile long long g_result = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent dependency chains */
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    for (i = 2; i < n; i++) {
        /* Chain 1: distance-1 dependency */
        x1 = x1 * 3 + a[i];
        /* Chain 2: distance-1 with different operation */
        x2 = (x2 << 2) ^ b[i];
        /* Chain 3: distance-1 with multiply */
        x3 = x3 * 7 + c[i] * 5;
        /* Chain 4: distance-1 with complex expression */
        x4 = (x4 & 0xFFFF) * 11 + d[i];
        
        /* Additional chains to increase register pressure */
        y1 = y1 + x1 * 2;
        y2 = y2 ^ (x2 >> 1);
        y3 = y3 * 3 - x3;
        y4 = y4 + (x4 & 0xFF);
        
        /* Cross-chain dependencies to create anti-dependencies */
        a[i-1] = x1 + y1;
        b[i-1] = x2 + y2;
        c[i-1] = x3 + y3;
        d[i-1] = x4 + y4;
    }
    
    /* Store results to volatile global */
    g_result += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 + a[i] * 1.5;
        sum2 = sum2 + b[i] * 2.5;
        sum3 = sum3 + c[i] * 3.5;
        
        /* Independent chains with different latencies */
        prod1 = prod1 * (a[i] + 0.1);
        prod2 = prod2 * (b[i] - 0.1);
        
        /* Cross dependencies requiring register moves */
        a[i-1] = sum1 * prod1;
        b[i-1] = sum2 * prod2;
        c[i-1] = sum3 + prod1 + prod2;
    }
    
    g_result += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer chains with carried dependencies */
        acc1 = acc1 * 3 + *ptr1;
        acc2 = acc2 * 5 + *ptr2;
        acc3 = acc3 * 7 + *ptr3;
        
        /* Update pointers - creates anti-dependencies */
        *ptr1 = acc1 >> 1;
        *ptr2 = acc2 >> 2;
        *ptr3 = acc3 >> 3;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
    }
    
    g_result += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with unrolling hint */
#pragma GCC unroll 4
void test_mixed_ops_unrolled(short *s, int *i, long *l, int n) {
    int j;
    short s_acc = s[0];
    int i_acc = i[0];
    long l_acc = l[0];
    
    for (j = 1; j < n; j++) {
        /* Operations with different register sizes */
        s_acc = (s_acc + s[j]) & 0x7FFF;
        i_acc = i_acc * 3 + i[j];
        l_acc = l_acc + (l[j] << 2);
        
        /* Cross-type operations to increase register pressure */
        s[j-1] = (short)(s_acc + (i_acc & 0xFFFF));
        i[j-1] = i_acc + (int)(l_acc & 0xFFFFFFFF);
        l[j-1] = l_acc + s_acc;
    }
    
    g_result += s_acc + i_acc + l_acc;
}

/* Test 5: PowerPC-specific double operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    int i;
    double d1 = a[0], d2 = b[0];
    double d3 = 1.0, d4 = 2.0;
    
    for (i = 1; i < n; i++) {
        /* FP operations that use multiple FP registers */
        d1 = d1 * 1.1 + a[i];
        d2 = d2 * 2.2 + b[i];
        d3 = d3 * 3.3 - a[i];
        d4 = d4 * 4.4 - b[i];
        
        /* Cross dependencies */
        a[i-1] = d1 + d3;
        b[i-1] = d2 + d4;
        
        /* Additional operations to increase pressure */
        d1 = d1 * 0.9;
        d2 = d2 * 0.8;
        d3 = d3 * 1.2;
        d4 = d4 * 1.1;
    }
    
    g_result += (long long)(d1 + d2 + d3 + d4);
}
#endif

/* Test 6: Variable distance dependencies */
void test_variable_distance(int *arr, int *pattern, int n) {
    int i;
    int acc[4] = {arr[0], arr[1], arr[2], arr[3]};
    
    for (i = 4; i < n; i++) {
        /* Variable dependency distances based on pattern */
        int dist = pattern[i % 16] & 3;
        if (dist == 0) dist = 1;
        
        /* Multiple accumulators with different distances */
        acc[0] = acc[0] + arr[i] * dist;
        acc[1] = acc[1] + arr[i-dist] * (dist + 1);
        acc[2] = acc[2] * 2 + arr[i-2];
        acc[3] = acc[3] ^ arr[i-3];
        
        /* Store back creating anti-dependencies */
        arr[i-1] = acc[0] + acc[1] + acc[2] + acc[3];
    }
    
    g_result += acc[0] + acc[1] + acc[2] + acc[3];
}

int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    double *darr1 = malloc(SIZE * sizeof(double));
    double *darr2 = malloc(SIZE * sizeof(double));
    double *darr3 = malloc(SIZE * sizeof(double));
    short *sarr = malloc(SIZE * sizeof(short));
    long *larr = malloc(SIZE * sizeof(long));
    int *pattern = malloc(16 * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !darr1 || !darr2 || !darr3 || 
        !sarr || !larr || !pattern) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
        darr1[i] = (double)(rand() % 100) / 10.0;
        darr2[i] = (double)(rand() % 100) / 10.0;
        darr3[i] = (double)(rand() % 100) / 10.0;
        sarr[i] = (short)(rand() % 100);
        larr[i] = (long)(rand() % 100);
    }
    
    for (i = 0; i < 16; i++) {
        pattern[i] = rand() % 100;
    }
    
    printf("Starting modulo scheduling tests...\n");
    start = clock();
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        /* Cycle through different tests to exercise various patterns */
        test_multi_recurrence_int(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulate(darr1, darr2, darr3, SIZE);
        test_pointer_chasing(arr1, SIZE, 4);
        test_mixed_ops_unrolled(sarr, arr2, larr, SIZE);
        test_variable_distance(arr3, pattern, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_double(darr1, darr2, SIZE);
        #endif
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Tests completed in %.2f seconds\n", cpu_time_used);
    printf("Final result (checksum): %lld\n", (long long)g_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(darr1);
    free(darr2);
    free(darr3);
    free(sarr);
    free(larr);
    free(pattern);
    
    return 0;
}
