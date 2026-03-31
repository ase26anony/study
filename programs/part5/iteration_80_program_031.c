/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern float global_float_array[1024];
static volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func1(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline)) float noinline_func2(float a, float b) {
    return a * b - a / (b + 1.0f);
}

/* Pure function for loop computations */
__attribute__((const)) int pure_multiply(int a, int b) {
    return a * b;
}

/* Helper with side effects */
__attribute__((noinline)) int side_effect_func(int* ptr) {
    int val = *ptr;
    *ptr = val ^ 0x5A5A5A5A;
    return val;
}

/* Complex nested loop structure */
void __attribute__((noinline)) compute_checksum(int N, int M, int K, int* result) {
    int acc_int = 0;
    float acc_float = 0.0f;
    unsigned short us_counter;
    register int reg_var;  /* Hint for register allocation */
    
    /* Outer loop with varying data types */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency */
        acc_int += i * (i % 7);
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > (N / 3), 0)) {
            /* First inner loop with short counter */
            for (us_counter = 0; us_counter < (unsigned short)M; ++us_counter) {
                int temp = pure_multiply(i, us_counter);
                
                /* Memory operation with potential aliasing */
                global_array[temp % 1024] ^= acc_int;
                
                /* Arithmetic mix */
                acc_float += global_float_array[temp % 1024] * 0.5f;
                
                /* Optimization barrier */
                asm volatile("" ::: "memory");
                
                /* Conditional branch inside innermost loop */
                if ((us_counter % K) == 0) {
                    acc_int += noinline_func1(i, us_counter);
                }
            }
        }
        
        /* Second inner loop with different characteristics */
        if (__builtin_expect(i < (2 * N / 3), 1)) {
            for (int j = 0; j < (M / 2); ++j) {
                /* Variable with different scope */
                {
                    int local_var = i * j + acc_int;
                    reg_var = local_var;  /* Multiple uses */
                    
                    /* Function call with loop-variant arguments */
                    float fval = noinline_func2(reg_var * 0.01f, j * 0.02f);
                    acc_float += fval;
                    
                    /* Another memory operation */
                    global_array[(i + j) % 1024] += reg_var;
                    
                    /* Use reg_var again */
                    if (reg_var % 5 == 0) {
                        acc_int -= side_effect_func(&global_array[j % 1024]);
                    }
                }
                
                /* Nested conditional */
                if (j % 3 == 0) {
                    /* Tiny loop inside */
                    for (int k = 0; k < 3; ++k) {
                        acc_int += (i << k) | (j >> k);
                    }
                }
            }
        }
        
        /* Third loop with volatile dependency */
        int limit = volatile_seed % 8;
        for (int l = 0; l < limit; ++l) {
            /* Complex expression with mixed types */
            acc_int += (int)(acc_float * 100.0f) ^ (i * l);
            
            /* Another optimization barrier */
            asm volatile("" ::: "memory");
        }
    }
    
    /* Final computation */
    *result = acc_int + (int)acc_float;
}

/* Warm-up function */
void __attribute__((noinline)) warmup_computation(void) {
    int dummy_result;
    int warm_N = volatile_seed % 10 + 5;
    int warm_M = volatile_seed % 8 + 3;
    
    /* Execute once to potentially trigger different compilation paths */
    compute_checksum(warm_N, warm_M, 2, &dummy_result);
}

/* Initialize arrays with pseudo-random values */
void init_arrays(void) {
    unsigned int lcg = 123456789;
    
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_array[i] = (int)(lcg % 1000);
        
        lcg = lcg * 1103515245 + 12345;
        global_float_array[i] = (float)(lcg % 1000) * 0.001f;
    }
}

int main(int argc, char* argv[]) {
    int N, M, K;
    int checksum;
    
    /* Use command line args for variability */
    if (argc >= 4) {
        N = atoi(argv[1]);
        M = atoi(argv[2]);
        K = atoi(argv[3]);
    } else {
        /* Default values that create interesting patterns */
        N = 100;
        M = 50;
        K = 7;
    }
    
    /* Ensure non-zero, reasonable bounds */
    if (N <= 0) N = 100;
    if (M <= 0) M = 50;
    if (K <= 0) K = 7;
    
    printf("Running selective scheduling test with N=%d, M=%d, K=%d\n", N, M, K);
    
    /* Initialize data */
    init_arrays();
    
    /* Warm-up run */
    printf("Warm-up computation...\n");
    warmup_computation();
    
    /* Main computation */
    printf("Main computation...\n");
    compute_checksum(N, M, K, &checksum);
    
    /* Additional variant with different types */
    {
        unsigned N2 = (unsigned)N * 2;
        int checksum2;
        
        /* Loop with unsigned counter */
        for (unsigned u = 0; u < N2; ++u) {
            if (u % 4 == 0) {
                for (short s = 0; s < (short)(M / 2); ++s) {
                    checksum += (int)(u * s) - (u % 16);
                }
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use result to prevent dead code elimination */
    if (checksum == 0x12345678) {  /* Unlikely */
        printf("Impossible condition\n");
    }
    
    return 0;
}
