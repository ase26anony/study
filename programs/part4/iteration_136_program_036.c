/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent loop unrolling and vectorization */
__attribute__((optimize("no-unroll-loops")))
__attribute__((noinline))
static void modulo_sched_stress(int *a, float *b, volatile int *c, 
                                volatile float *d, int n, int seed) {
    volatile int acc_int = seed;
    volatile float acc_float = (float)seed;
    int temp_int;
    float temp_float;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 0; i < n; i++) {
        /* Cross-iteration recurrence: distance-1 dependence */
        acc_int = acc_int * a[i] + i;  /* Carried scalar dependence */
        
        /* Mixed latency operations */
        temp_int = a[i] * 3;           /* Integer multiply */
        temp_float = b[i] + 1.5f;      /* Float addition */
        
        /* Volatile memory access with artificial latency */
        *c = temp_int;
        temp_int = *c;                 /* Memory load with potential stall */
        
        /* Complex control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer path */
                acc_int += temp_int * 2;
                /* Inline assembly to create register pressure */
                asm volatile ("" : "+r" (acc_int) : : "memory");
                break;
            case 1:
                /* Float path */
                acc_float = acc_float * 0.9f + temp_float;
                asm volatile ("" : "+r" (acc_float) : : "memory");
                break;
            case 2:
                /* Mixed path with pointer chasing */
                acc_int = (acc_int & 0xFF) + a[acc_int & 0xF];
                acc_float = acc_float * b[i & 0x3];
                break;
            case 3:
                /* Memory intensive path */
                *d = acc_float;
                temp_float = *d;
                acc_int ^= (int)temp_float;
                break;
            default:
                /* Complex computation with multiple dependencies */
                temp_int = (acc_int >> 3) | (acc_int << 29);  /* Rotate */
                acc_int = temp_int * a[i] - i;
                asm volatile ("" : "+r" (temp_int) : : "memory");
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        for (int j = 0; j < 2; j++) {
            volatile int inner = acc_int + j;
            /* Create artificial dependency chain */
            asm volatile ("addl $1, %0" : "+r" (inner) : : "cc");
            acc_int ^= inner;
        }
        
        /* Store to volatile to prevent elimination */
        *c = acc_int;
        *d = acc_float;
    }
    
    /* Final store to global volatile */
    volatile int final_result __attribute__((unused)) = acc_int;
}

/* Global volatile to prevent optimization */
volatile int global_sink;

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    
    /* Dynamic allocation to prevent constant propagation */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    volatile int c;
    volatile float d;
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        a[i] = (i * 13) % 97;
        b[i] = (float)((i * 17) % 101) * 0.1f;
    }
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, &c, &d, n, iter * 12345);
    }
    
    /* Use results to prevent dead code elimination */
    global_sink = a[0] + (int)b[0];
    
    free(a);
    free(b);
    
    printf("Test completed. Check dump files for modulo scheduling debug output.\n");
    printf("Look for lines with format: '%%11d %%11d %%5d %%d --(T,%%d,%%d)--> %%d\\n'\n");
    
    return 0;
}
