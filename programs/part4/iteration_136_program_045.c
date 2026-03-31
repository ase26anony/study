/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Start with first element */
    float acc_float = *b;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence relation */
        acc_float = acc_float * b[i] + d[i];
        
        /* Mixed latency operations */
        int temp_int = acc_int;
        float temp_float = acc_float;
        
        /* Complex control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic with memory access */
                temp_int = (temp_int * 3) / 2;
                local_volatile = temp_int;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r"(temp_int) : : "memory");
                break;
            case 1:
                /* Floating point operation */
                temp_float = temp_float * 1.5f + 2.0f;
                /* Force memory store/load */
                float_sink = temp_float;
                temp_float = float_sink;
                break;
            case 2:
                /* Mixed operations */
                temp_int = temp_int ^ (temp_int >> 3);
                temp_float = temp_float + (float)temp_int;
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                break;
            case 3:
                /* Pointer chasing creating memory dependency */
                temp_int = a[temp_int % (n-1)];
                temp_float = b[temp_int % (n-1)];
                break;
            case 4:
                /* Complex expression with multiple dependencies */
                temp_int = (temp_int * 7 + a[i-1]) / 3;
                temp_float = (temp_float * 2.0f + b[i-1]) / 1.5f;
                /* Another asm to prevent optimization */
                asm volatile ("" : "+r"(temp_int), "+r"(temp_float));
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        for (int j = 0; j < 2; j++) {
            /* Simple operation that depends on outer loop */
            int nested_temp = temp_int + j;
            float nested_float = temp_float + (float)j;
            
            /* Conditional execution inside nested loop */
            if ((i + j) % 3 == 0) {
                nested_temp *= 2;
                nested_float *= 1.2f;
            } else {
                nested_temp /= 2;
                nested_float /= 1.2f;
            }
            
            /* Use results to prevent dead code elimination */
            asm volatile ("" : "+r"(nested_temp), "+r"(nested_float));
        }
        
        /* Store back to arrays with offset creating more dependencies */
        if (i < n - 1) {
            a[i+1] = temp_int % 1000;
            b[i+1] = temp_float * 0.5f;
        }
    }
    
    /* Final store to volatile global */
    global_sink = acc_int;
    float_sink = acc_float;
}

/* Irreducible control flow using computed goto */
__attribute__((noinline))
int irreducible_control(int x) {
    void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = x;
    
    /* Jump table to create irreducible flow */
    goto *labels[x % 4];
    
label0:
    result = result * 3 + 1;
    goto end;
label1:
    result = result / 2;
    goto end;
label2:
    result = result ^ 0xAAAA;
    goto end;
label3:
    result = result + result;
    goto end;
    
end:
    return result;
}

/* Main function with runtime-determined loop count */
int main(int argc, char *argv[]) {
    /* Use command line or random size to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Initialize with pattern to avoid simple analysis */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100 + 1;
        b[i] = (float)(rand() % 100) / 10.0f + 0.1f;
        c[i] = rand() % 50 + 1;
        d[i] = (float)(rand() % 50) / 5.0f + 0.1f;
    }
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, d, n);
        
        /* Also call irreducible control flow function */
        int temp = irreducible_control(iter + a[0]);
        global_sink += temp;
    }
    
    /* Print result to ensure computation happens */
    printf("Result: %d, %f\n", global_sink, float_sink);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
