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
    int acc_int = *a;  /* Start with first element */
    float acc_float = *b;
    int i;
    
    /* Complex loop with cross-iteration dependencies */
    for (i = 1; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + c[i];
        
        /* Another distance-1 dependence */
        acc_float = acc_float * b[i] + d[i];
        
        /* Mixed operations with different latencies */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic */
                acc_int += (acc_int >> 3) | (acc_int << 29);  /* Rotate */
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (acc_int));
                break;
            case 1:
                /* Floating point operation */
                acc_float = acc_float * 1.01f + 0.5f;
                /* Memory operation */
                local_volatile = a[i-1];  /* Distance-1 memory access */
                break;
            case 2:
                /* Mixed integer/float */
                acc_int ^= (int)acc_float;
                acc_float += (float)(acc_int & 0xFF);
                break;
            case 3:
                /* Complex expression with multiple dependencies */
                acc_int = (acc_int * 1103515245 + 12345) & 0x7fffffff;
                acc_float = acc_float * 0.99f + (float)(acc_int % 100) * 0.01f;
                /* Another inline asm barrier */
                asm volatile ("" : "+r" (acc_int), "+f" (acc_float));
                break;
            case 4:
                /* Pointer chasing creating memory dependence */
                int temp = c[i] + a[i-1];  /* Distance-1 */
                acc_int += temp;
                /* Conditional store */
                if (acc_int % 7 == 0) {
                    local_volatile = acc_int;
                }
                break;
        }
        
        /* Irreducible control flow using computed goto */
        if (i % 13 == 0) {
            static void *labels[] = { &&label1, &&label2, &&label3 };
            goto *labels[i % 3];
        }
        
        /* Normal execution path */
        acc_int = (acc_int + i) & 0xFFF;
        continue;
        
    label1:
        acc_float = acc_float * 2.0f - 1.0f;
        continue;
        
    label2:
        acc_int = ~acc_int;
        continue;
        
    label3:
        acc_float = -acc_float;
        continue;
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
    
    /* Additional nested loop for scheduling pressure */
    {
        int j, k;
        volatile int nested_acc = 0;
        for (j = 0; j < 10; j++) {
            for (k = 0; k < 10; k++) {
                nested_acc += a[(j + k) % n] * c[(j * k) % n];
                /* Memory dependence in nested loop */
                if (k > 0) {
                    nested_acc += nested_acc >> 1;
                }
            }
            /* Cross-iteration in outer loop */
            if (j > 0) {
                nested_acc ^= nested_acc << 3;
            }
        }
        global_sink += nested_acc;
    }
}

/* Helper to initialize arrays */
void init_arrays(int *a, float *b, int *c, float *d, int n) {
    int i;
    for (i = 0; i < n; i++) {
        a[i] = rand() % 1000;
        b[i] = (float)(rand() % 1000) / 10.0f;
        c[i] = rand() % 1000;
        d[i] = (float)(rand() % 1000) / 20.0f;
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Dynamic allocation to prevent constant propagation */
    int *a = malloc(n * sizeof(int));
    float *b = malloc(n * sizeof(float));
    int *c = malloc(n * sizeof(int));
    float *d = malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    init_arrays(a, b, c, d, n);
    
    /* Call the stress function multiple times */
    int i;
    for (i = 0; i < 3; i++) {
        modulo_sched_stress(a, b, c, d, n);
    }
    
    printf("Result: %d %f\n", global_sink, global_float_sink);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
