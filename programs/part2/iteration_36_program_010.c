/* sel-sched-coverage.c
 * Designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;

/* Function with memory aliasing */
static inline void compute_loop(int *restrict arr_a, int *arr_b, 
                               float *restrict arr_f, double *arr_d,
                               int start, int end, int seed) {
    int i;
    int local_sum = seed;
    float local_fsum = seed * 0.5f;
    double local_dprod = 1.0;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        local_sum = local_sum * 1103515245 + 12345;
        
        /* Floating-point operations */
        local_fsum = local_fsum * 1.5f + arr_f[i % SIZE] * 0.3f;
        
        /* Double precision with potential division */
        if (arr_d[i % SIZE] != 0.0) {
            local_dprod = local_dprod / (arr_d[i % SIZE] + 1.0) * 2.0;
        }
        
        /* Memory operations with aliasing */
        arr_a[i % SIZE] = local_sum + arr_b[i % SIZE];
        arr_b[(i + 1) % SIZE] = arr_a[i % SIZE] * 2;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in taken branch */
            local_fsum = local_fsum - arr_f[(i + 3) % SIZE];
            g_volatile_counter++;
            
            /* Inline assembly with memory clobber to prevent optimization */
            asm volatile("" ::: "memory");
        } else if (i % 13 == 0) {
            /* Another basic block */
            local_dprod = local_dprod * 0.99;
            arr_b[i % SIZE] = local_sum >> 2;
        }
        
        /* More arithmetic diversity */
        if (i % 17 == 0) {
            local_sum = local_sum ^ (arr_b[(i + 5) % SIZE] * 3);
        }
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 19 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Store results back to prevent dead code elimination */
    arr_a[0] = local_sum;
    arr_f[0] = local_fsum;
    arr_d[0] = local_dprod;
}

/* Another variant with different operations */
static inline void compute_loop_variant(float *restrict farr, int *iarr,
                                       double *darr, int n, int offset) {
    int i;
    float f_acc = farr[offset];
    int i_acc = iarr[offset];
    double d_acc = darr[offset];
    
    for (i = 0; i < n; i++) {
        /* Mixed operations */
        f_acc = f_acc * 1.1f + (i % 256) * 0.01f;
        i_acc = (i_acc << 3) | (i_acc >> 29);  /* rotate */
        i_acc = i_acc ^ (iarr[(i + offset) % SIZE]);
        
        /* Memory operations */
        farr[(i + offset) % SIZE] = f_acc;
        iarr[(i + offset + 1) % SIZE] = i_acc;
        
        /* Complex double operation */
        d_acc = d_acc + (darr[(i + offset) % SIZE] * 0.5);
        d_acc = d_acc - (i % 11) * 0.01;
        
        /* Conditional with side effects */
        if ((i_acc & 0xF) == 0) {
            d_acc = d_acc * 0.999;
            asm volatile("" ::: "memory");
        }
        
        /* Periodic memory barrier */
        if (i % 23 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Store accumulated values */
    farr[offset] = f_acc;
    iarr[offset] = i_acc;
    darr[offset] = d_acc;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr_a = (int*)malloc(SIZE * sizeof(int));
    int *arr_b = (int*)malloc(SIZE * sizeof(int));
    float *arr_f = (float*)malloc(SIZE * sizeof(float));
    double *arr_d = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr_a || !arr_b || !arr_f || !arr_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr_a[i] = i * 3;
        arr_b[i] = i * 5 + 1;
        arr_f[i] = i * 0.7f;
        arr_d[i] = i * 0.3;
    }
    
    /* Perform multiple computations to create scheduling regions */
    for (int iter = 0; iter < 10; iter++) {
        compute_loop(arr_a, arr_b, arr_f, arr_d, 
                    0, ITERATIONS, iter * 100);
        
        compute_loop_variant(arr_f, arr_a, arr_d, 
                           ITERATIONS / 2, iter * 50);
        
        /* Alternate between different loop bounds */
        if (iter % 2 == 0) {
            compute_loop(arr_b, arr_a, arr_f, arr_d,
                        100, ITERATIONS - 100, iter * 77);
        }
    }
    
    /* Compute checksum to prevent optimization */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= (uint64_t)arr_a[i];
        checksum ^= (uint64_t)arr_b[i];
        checksum ^= *(uint64_t*)&arr_f[i];  /* Treat float as bits */
        checksum ^= *(uint64_t*)&arr_d[i];  /* Treat double as bits */
    }
    
    printf("Result checksum: 0x%016llx\n", (unsigned long long)checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(arr_a);
    free(arr_b);
    free(arr_f);
    free(arr_d);
    
    return 0;
}
