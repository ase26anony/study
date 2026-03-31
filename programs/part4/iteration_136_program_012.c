/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Start with first element */
    float acc_float = *b;
    int i, j;
    
    /* Complex loop with cross-iteration dependencies */
    for (i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence relation */
        acc_float = acc_float + b[i] * d[i];
        
        /* Mixed latency operations */
        int temp_int = acc_int;
        float temp_float = acc_float;
        
        /* Irreducible control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer operations */
                temp_int = temp_int * 3;
                temp_int = temp_int / 2;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (temp_int));
                break;
            case 1:
                /* Floating point operations (higher latency) */
                temp_float = temp_float * 1.5f;
                temp_float = temp_float - 0.25f;
                /* Force memory barrier */
                asm volatile ("" : : : "memory");
                break;
            case 2:
                /* Memory operations */
                local_volatile = a[i] + c[i-1];  /* Distance-1 memory access */
                temp_int = local_volatile * 2;
                break;
            case 3:
                /* Mixed operations */
                temp_int = (int)(temp_float * 100.0f);
                temp_float = (float)temp_int / 50.0f;
                /* Another asm barrier */
                asm volatile ("" : "+r" (temp_int), "+r" (temp_float));
                break;
            case 4:
                /* Complex computation with multiple dependencies */
                temp_int = (temp_int << 2) | (temp_int >> 30);
                temp_float = temp_float + (float)(temp_int & 0xFF);
                /* Memory operation with volatile */
                *(volatile int *)&a[i] = temp_int;
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        for (j = 0; j < 2; j++) {
            /* Simple operation that depends on outer loop */
            int inner_temp = temp_int + j;
            /* Use computed goto for irreducible flow */
            void *labels[] = { &&label1, &&label2 };
            goto *labels[j];
            
        label1:
            inner_temp += 1;
            goto cont;
            
        label2:
            inner_temp *= 2;
            goto cont;
            
        cont:
            /* Use the result */
            temp_int += inner_temp;
        }
        
        /* Store results with volatile to prevent optimization */
        c[i] = temp_int;
        d[i] = temp_float;
    }
    
    /* Final store to global to prevent elimination */
    global_sink = acc_int + (int)acc_float;
}

/* Helper to initialize arrays */
void init_arrays(int *a, float *b, int *c, float *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 37) % 101;      /* Patterned but not constant */
        b[i] = (float)((i * 19) % 53) / 10.0f;
        c[i] = (i * 23) % 97;
        d[i] = (float)((i * 29) % 71) / 10.0f;
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Allocate arrays dynamically to avoid constant propagation */
    int *a = malloc(n * sizeof(int));
    float *b = malloc(n * sizeof(float));
    int *c = malloc(n * sizeof(int));
    float *d = malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant data */
    srand(time(NULL));
    init_arrays(a, b, c, d, n);
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, d, n);
        
        /* Modify inputs slightly between iterations */
        for (int i = 0; i < n; i++) {
            a[i] += (rand() % 5) - 2;
            b[i] += (float)((rand() % 5) - 2) / 10.0f;
        }
    }
    
    /* Print a result to prevent complete optimization */
    printf("Result: %d (from global_sink=%d)\n", 
           a[n-1] + (int)b[n-1], global_sink);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
