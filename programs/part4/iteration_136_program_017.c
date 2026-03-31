/* Test program for modulo scheduling debug output coverage */
#include <stdlib.h>
#include <stdio.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    float acc_f = 1.0f;
    int acc_i = 1;
    
    /* Cross-iteration dependencies with mixed operations */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_i depends on previous iteration */
        acc_i = acc_i * a[i] + c[i];
        
        /* Another distance-1 dependence with floating point */
        acc_f = acc_f * b[i] + d[i];
        
        /* Complex control flow using switch */
        switch (i & 3) {
            case 0: {
                /* Integer operations with inline asm to prevent optimization */
                int temp = acc_i + a[i];
                asm volatile ("" : "+r"(temp) : : "memory");
                c[i] = temp;
                break;
            }
            case 1: {
                /* Floating point operations */
                float ftemp = acc_f * 1.5f;
                /* Force memory barrier */
                asm volatile ("" : "+m"(ftemp) : : "memory");
                d[i] = ftemp;
                break;
            }
            case 2: {
                /* Mixed operations with memory access */
                int itemp = acc_i - c[i];
                float ftemp = acc_f + b[i];
                /* Inline asm with multiple outputs */
                asm volatile ("# dummy asm" : "+r"(itemp), "+r"(ftemp) : : "memory");
                a[i] = itemp;
                b[i] = ftemp;
                break;
            }
            case 3: {
                /* Pointer chasing creating complex dependencies */
                int *ptr = &a[i];
                int val = *ptr + acc_i;
                /* Memory barrier */
                asm volatile ("" : : "r"(ptr), "r"(val) : "memory");
                c[i] = val;
                
                /* Additional floating point operation */
                float *fptr = &b[i];
                float fval = *fptr * acc_f;
                asm volatile ("" : : "r"(fptr), "r"(fval) : "memory");
                d[i] = fval;
                break;
            }
        }
        
        /* Nested loop to create additional scheduling pressure */
        for (int j = 0; j < 2; j++) {
            local_volatile = a[i] + j;
            /* Memory operation with volatile */
            asm volatile ("" : "+m"(local_volatile) : : "memory");
        }
    }
    
    /* Store results to prevent dead code elimination */
    global_sink = acc_i;
    global_float_sink = acc_f;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_control_flow(int *arr, int n) {
    void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int sum = 0;
    int i = 0;
    
    /* Computed goto creating irreducible flow */
    goto *labels[i % 4];
    
label0:
    for (; i < n; ) {
        sum += arr[i] * 2;
        i++;
        if (i < n) goto *labels[i % 4];
    }
    goto end;
    
label1:
    for (; i < n; ) {
        sum -= arr[i];
        i++;
        if (i < n) goto *labels[i % 4];
    }
    goto end;
    
label2:
    for (; i < n; ) {
        sum ^= arr[i];
        i++;
        if (i < n) goto *labels[i % 4];
    }
    goto end;
    
label3:
    for (; i < n; ) {
        sum |= arr[i] << 1;
        i++;
        if (i < n) goto *labels[i % 4];
    }
    
end:
    global_sink = sum;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    /* Use command line or random seed for runtime values */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate arrays with dynamic size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    
    /* Initialize with pattern (not compile-time constant) */
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 7;
        b[i] = (float)((i * 5) % 11) * 0.1f;
        c[i] = (i * 7) % 13;
        d[i] = (float)((i * 11) % 17) * 0.01f;
    }
    
    /* Call the stress function */
    modulo_sched_stress(a, b, c, d, n);
    
    /* Also test irreducible control flow */
    irreducible_control_flow(a, n / 2);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return global_sink & 0xFF;
}
