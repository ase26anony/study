/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent dead code elimination */
volatile long global_sink;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int v1 = seed;
    volatile float v2 = seed * 0.5f;
    volatile double v3 = seed * 0.25;
    
    /* Cross-iteration recurrence with distance-1 dependence */
    int acc_int = v1;
    float acc_float = v2;
    double acc_double = v3;
    
    /* Additional control flow variables */
    int branch_var = 0;
    void* label_table[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    for (int i = 0; i < n; i++) {
        /* Distance-1 recurrence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;
        acc_float = acc_float + b[i] * 0.7f;
        acc_double = acc_double * 1.1 + c[i];
        
        /* Complex control flow using computed goto */
        branch_var = (branch_var + i) % 5;
        goto *label_table[branch_var];
        
    L0:
        /* Integer operations with memory access */
        {
            volatile int temp = a[i];
            asm volatile ("" : "+r"(temp) : : "memory");
            acc_int += temp * 3;
        }
        continue;
        
    L1:
        /* Floating point operations */
        {
            volatile float ftemp = b[i];
            asm volatile ("" : "+f"(ftemp) : : "memory");
            acc_float = acc_float * ftemp + 1.0f;
        }
        continue;
        
    L2:
        /* Mixed operations with inline assembly for latency */
        {
            double dtemp;
            asm volatile ("movq %1, %0" : "=r"(dtemp) : "r"(c[i]) : );
            acc_double = acc_double + dtemp;
            asm volatile ("" : : "r"(acc_int), "r"(acc_float) : "memory");
        }
        continue;
        
    L3:
        /* Memory intensive operations */
        {
            volatile int load1 = a[(i + 1) % n];
            volatile float load2 = b[(i + 2) % n];
            acc_int ^= load1;
            acc_float += load2;
            
            /* Artificial dependency chain */
            for (int j = 0; j < 3; j++) {
                volatile int chain = acc_int + j;
                asm volatile ("" : "+r"(chain) : : );
                acc_int = chain;
            }
        }
        continue;
        
    L4:
        /* Conditional store with volatile */
        if (i % 7 == 0) {
            volatile int store_var = acc_int;
            asm volatile ("" : : "r"(store_var) : "memory");
            a[i] = store_var;
        }
        continue;
    }
    
    /* Prevent elimination of results */
    global_sink = acc_int + (long)acc_float + (long)acc_double;
}

/* Another function with different recurrence pattern */
__attribute__((optimize("no-unroll-loops")))
void second_loop(short *s, long *l, int n, int init) {
    volatile short vs = init;
    volatile long vl = init * 100L;
    
    /* Multiple recurrence variables */
    short rec1 = vs;
    long rec2 = vl;
    int rec3 = init;
    
    for (int i = 0; i < n; i++) {
        /* Multiple distance-1 dependences */
        rec1 = rec1 + s[i] - i;
        rec2 = rec2 * 2 + l[i];
        rec3 = (rec3 ^ s[i]) * 3;
        
        /* Switch-based control flow */
        switch (i % 6) {
            case 0:
                rec1 = rec1 << 2;
                asm volatile ("" : "+r"(rec1) : : );
                break;
            case 1:
                rec2 = rec2 >> 1;
                asm volatile ("" : "+r"(rec2) : : );
                break;
            case 2:
                rec3 = rec3 + (i & 0xFF);
                asm volatile ("" : "+r"(rec3) : : );
                break;
            case 3:
                /* Memory operation with barrier */
                {
                    volatile long barrier = rec2;
                    asm volatile ("" : : "r"(barrier) : "memory");
                    l[i] = barrier;
                }
                break;
            case 4:
                /* Nested loop for additional complexity */
                for (int k = 0; k < 2; k++) {
                    volatile int inner = rec3 + k;
                    asm volatile ("" : "+r"(inner) : : );
                    rec3 = inner;
                }
                break;
            case 5:
                /* Mixed operations */
                rec1 = rec1 * rec3;
                rec2 = rec2 - rec1;
                asm volatile ("" : : "r"(rec1), "r"(rec2) : "memory");
                break;
        }
    }
    
    global_sink += rec1 + rec2 + rec3;
}

int main(int argc, char **argv) {
    /* Use runtime values to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    if (n < 10) n = 1000;
    
    /* Allocate arrays with runtime size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    double *c = (double*)malloc(n * sizeof(double));
    short *s = (short*)malloc(n * sizeof(short));
    long *l = (long*)malloc(n * sizeof(long));
    
    if (!a || !b || !c || !s || !l) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    srand(seed);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (rand() % 100) * 0.1f;
        c[i] = (rand() % 100) * 0.01;
        s[i] = rand() % 32767;
        l[i] = rand() * 100L;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, n, seed);
    second_loop(s, l, n, seed);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %ld\n", global_sink);
    
    /* Cleanup */
    free(a); free(b); free(c); free(s); free(l);
    
    return 0;
}
