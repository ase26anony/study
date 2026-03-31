/* sel-sched-test.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization of dependencies */
static volatile int global_seed = 42;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict out, const int *in, 
                                float *farr, double *darr, int n) {
    int i;
    float f_acc = 1.0f;
    double d_acc = 1.0;
    int int_acc = global_seed;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Integer operations with carried dependency */
        int_acc = int_acc * 1103515245 + 12345;
        
        /* Floating-point operations */
        f_acc = f_acc * 1.5f + farr[i % 16];
        d_acc = d_acc / 1.7 + darr[i % 16] * 0.5;
        
        /* Memory operations with potential aliasing */
        int val = in[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            val = val * 3 + 1;
            f_acc = f_acc - 0.25f;
        } else if (i % 13 == 0) {
            val = val / 2;
            d_acc = d_acc + 1.0;
        } else {
            val = val + int_acc % 100;
        }
        
        /* More arithmetic diversity */
        if (i % 5 == 0) {
            f_acc = f_acc * 2.0f;
            d_acc = d_acc - 0.3;
        }
        
        /* Store result with pointer arithmetic */
        out[i] = val + (int)(f_acc + d_acc);
        
        /* Additional independent operations */
        int temp = i * i;
        farr[i % 16] = farr[i % 16] + 0.1f * temp;
        darr[i % 16] = darr[i % 16] - 0.01 * temp;
    }
    
    /* Store accumulated values to prevent dead code elimination */
    out[n] = int_acc;
    out[n + 1] = (int)(f_acc * 1000);
    out[n + 2] = (int)(d_acc * 1000);
}

/* Another computation function to encourage inlining */
static inline void process_chunk(int *restrict dest, const int *src, 
                                 float *work_f, double *work_d, int start, int end) {
    for (int i = start; i < end; i++) {
        /* Mixed operations */
        float f1 = work_f[i % 8] * 1.1f;
        double d1 = work_d[i % 8] / 1.2;
        
        /* Integer computation with dependency chain */
        int x = src[i];
        x = (x << 3) | (x >> 29);  /* Rotate */
        x = x ^ (x * 13);
        
        /* Memory store with computation */
        dest[i] = x + (int)(f1 + d1);
        
        /* Update working arrays */
        work_f[i % 8] = f1 * 0.9f;
        work_d[i % 8] = d1 + 0.1;
    }
}

int main(void) {
    const int N = 1024;
    const int ITERS = 100;
    
    /* Allocate and initialize arrays */
    int *data_in = malloc(N * sizeof(int));
    int *data_out = malloc((N + 3) * sizeof(int));
    float *farray = malloc(16 * sizeof(float));
    double *darray = malloc(16 * sizeof(double));
    
    if (!data_in || !data_out || !farray || !darray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        data_in[i] = (i * 137) % 7919;
    }
    
    for (int i = 0; i < 16; i++) {
        farray[i] = (i * 0.7f) - 0.3f;
        darray[i] = (i * 1.3) + 0.5;
    }
    
    /* Perform multiple iterations to create hot loop */
    int checksum = 0;
    
    for (int iter = 0; iter < ITERS; iter++) {
        /* Call the hot function multiple times */
        compute_loop(data_out, data_in, farray, darray, N);
        
        /* Process in chunks to create more scheduling opportunities */
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * (N / 4);
            int end = (chunk + 1) * (N / 4);
            process_chunk(data_out + start, data_in + start, 
                         farray, darray, start, end);
        }
        
        /* Update checksum to prevent optimization */
        for (int i = 0; i < N; i++) {
            checksum ^= data_out[i];
        }
        
        /* Modify input slightly for next iteration */
        for (int i = 0; i < N; i++) {
            data_in[i] = (data_in[i] * 97 + 1) % 65521;
        }
    }
    
    /* Use volatile to ensure computations aren't optimized away */
    volatile int final_checksum = checksum;
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", final_checksum);
    printf("Sample values: %d, %d, %d\n", 
           data_out[0], data_out[N/2], data_out[N-1]);
    
    /* Free allocated memory */
    free(data_in);
    free(data_out);
    free(farray);
    free(darray);
    
    return 0;
}
