/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimization of critical function */
__attribute__((noinline))
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n, int seed) {
    volatile int acc_int = seed;
    volatile float acc_float = (float)seed;
    volatile int temp;
    volatile float ftemp;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + b[i];
        
        /* Mixed latency operations */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic (low latency) */
                temp = a[i] * b[i] + i;
                /* Inline assembly to create artificial use */
                asm volatile ("" : "+r" (temp));
                a[i] = temp;
                break;
            case 1:
                /* Floating point (higher latency) */
                ftemp = c[i] * d[i] + acc_float;
                /* Cross-iteration float dependence */
                acc_float = ftemp * 0.5f;
                break;
            case 2:
                /* Memory operations with volatile */
                temp = *(volatile int *)&a[i];
                ftemp = *(volatile float *)&c[i];
                /* Complex expression with both int and float */
                acc_int = (acc_int + temp) ^ (int)ftemp;
                break;
            case 3:
                /* Nested control flow within loop */
                if (acc_int % 3 == 0) {
                    temp = b[i] << 2;
                    asm volatile ("" : "+r" (temp));
                    a[i] = temp;
                } else {
                    ftemp = d[i] * 3.14f;
                    c[i] = ftemp;
                }
                /* Another distance-1 dependence */
                acc_float = acc_float + c[i-1];
                break;
            case 4:
                /* Pointer chasing creating memory dependence */
                int *ptr = &a[i];
                float *fptr = &c[i];
                temp = *ptr + *(ptr-1);  /* Distance-1 memory access */
                ftemp = *fptr * *(fptr-1);
                asm volatile ("" : "+r" (temp), "+r" (ftemp));
                break;
        }
        
        /* Additional recurrence relation */
        if (i > 2) {
            /* Multiple distance dependences */
            a[i] = a[i] + a[i-1] + a[i-2];
            c[i] = c[i] + c[i-1] * 0.3f;
        }
        
        /* Irreducible control flow via computed goto */
        static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
        goto *labels[i % 4];
        
        L0:
            temp = acc_int & 0xFF;
            continue;
        L1:
            ftemp = acc_float * 2.0f;
            continue;
        L2:
            temp = b[i] | 0x1;
            asm volatile ("" : "+r" (temp));
            continue;
        L3:
            ftemp = d[i] / 2.0f;
            continue;
    }
    
    /* Force result to be used */
    volatile int *global_result = (volatile int *)malloc(sizeof(int));
    *global_result = acc_int + (int)acc_float;
}

/* Another function with different pattern */
__attribute__((noinline))
void another_loop(int *arr1, int *arr2, int n) {
    volatile int sum = 0;
    for (int i = 1; i < n; i++) {
        /* Different recurrence pattern */
        arr1[i] = arr1[i-1] * 3 + arr2[i];
        sum += arr1[i];
        
        /* Conditional with side effects */
        if (sum % 7 == 0) {
            asm volatile ("" : : "r" (sum));
            arr2[i] = sum;
        }
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    
    /* Dynamic allocation prevents compile-time optimization */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    float *c = (float *)malloc(n * sizeof(float));
    float *d = (float *)malloc(n * sizeof(float));
    
    /* Initialize with pattern (not constant) */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, d, n, rand());
        another_loop(a, b, n);
    }
    
    /* Use results to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + (int)c[i];
    }
    
    printf("Result: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
