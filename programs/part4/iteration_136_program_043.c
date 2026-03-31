/* Test program for modulo scheduling debug output coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = 1;
    float acc_float = 1.0f;
    int i, j;
    
    /* Cross-iteration dependencies with mixed operations */
    for (i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence relation */
        acc_float = acc_float * b[i] + d[i];
        
        /* Complex control flow with switch */
        switch (i % 4) {
            case 0:
                /* Integer operations with artificial latency */
                {
                    int temp = acc_int * 3;
                    /* Inline asm to create register pressure */
                    asm volatile ("# case0 asm" : "+r" (temp));
                    a[i] = temp;
                    /* Memory load with potential latency */
                    local_volatile = c[i % 16];
                }
                break;
                
            case 1:
                /* Floating point operations */
                {
                    float ftemp = acc_float * 2.5f;
                    /* Force memory store */
                    d[i] = ftemp;
                    /* Another asm to prevent optimization */
                    asm volatile ("# case1 asm" : "+f" (ftemp));
                    /* Mixed type operation */
                    acc_int += (int)ftemp;
                }
                break;
                
            case 2:
                /* Pointer chasing creating memory dependencies */
                {
                    int *ptr = &a[i];
                    int val = *ptr;
                    /* Complex expression with multiple uses */
                    val = (val << 3) | (val >> 5);
                    /* Another asm barrier */
                    asm volatile ("# case2 asm" : "+r" (val));
                    c[i] = val;
                    /* Additional arithmetic */
                    acc_float -= (float)val * 0.1f;
                }
                break;
                
            case 3:
                /* Nested loop inside main loop for extra complexity */
                {
                    int sum = 0;
                    for (j = 0; j < 3; j++) {
                        sum += a[(i + j) % n] * j;
                    }
                    /* Conditional execution */
                    if (sum & 1) {
                        b[i] = acc_float * 0.5f;
                    } else {
                        b[i] = acc_float * 1.5f;
                    }
                    /* More inline asm */
                    asm volatile ("# case3 asm" : "+r" (sum));
                    acc_int ^= sum;
                }
                break;
        }
        
        /* Additional cross-iteration dependence */
        if (i > 0) {
            /* Use value from previous iteration */
            a[i] += a[i-1] & 0xFF;
            b[i] += b[i-1] * 0.25f;
        }
        
        /* Volatile access to prevent reordering */
        local_volatile = i;
    }
    
    /* Store results to global volatile to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_flow(int *arr, int n) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int i = 0;
    int state = 0;
    
    /* Computed goto creates irreducible flow */
    goto *labels[state];
    
label0:
    if (i >= n) goto end;
    arr[i] = arr[i] * 2 + 1;
    state = (state + 1) % 4;
    i++;
    goto *labels[state];
    
label1:
    if (i >= n) goto end;
    arr[i] = arr[i] * 3 - 2;
    state = (state + 2) % 4;
    i++;
    goto *labels[state];
    
label2:
    if (i >= n) goto end;
    arr[i] = (arr[i] << 1) | (arr[i] >> 31);
    state = (state + 3) % 4;
    i++;
    goto *labels[state];
    
label3:
    if (i >= n) goto end;
    arr[i] = arr[i] ^ 0xAAAAAAAA;
    state = (state + 1) % 4;
    i++;
    goto *labels[state];
    
end:
    return;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    const int N = 256;
    int *array1, *array3;
    float *array2, *array4;
    int i;
    
    /* Allocate and initialize arrays with pattern */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (float*)malloc(N * sizeof(float));
    array3 = (int*)malloc(N * sizeof(int));
    array4 = (float*)malloc(N * sizeof(float));
    
    srand(time(NULL));
    
    for (i = 0; i < N; i++) {
        array1[i] = rand() % 100 + 1;
        array2[i] = (float)(rand() % 100) / 10.0f + 0.1f;
        array3[i] = rand() % 50 + 1;
        array4[i] = (float)(rand() % 50) / 5.0f + 0.2f;
    }
    
    /* Call modulo scheduling test function */
    modulo_sched_stress(array1, array2, array3, array4, N);
    
    /* Call irreducible flow function */
    irreducible_flow(array1, N / 2);
    
    /* Print something to prevent complete optimization */
    printf("Result: %d %f\n", global_sink, global_float_sink);
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
