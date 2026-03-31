/* sel_sched_test.c - Main test program for selective scheduling dump coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern volatile int volatile_seed;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_multiply(int a, int b) {
    return a * b;
}

__attribute__((noinline)) int noinline_divide(int a, int b) {
    return b != 0 ? a / (b + 1) : 0;
}

/* Pure function for loop computations */
__attribute__((const)) int pure_compute(int x, int y) {
    return (x * x) + (y * y);
}

/* Helper function with mixed operations */
__attribute__((noinline)) int mixed_operations(int a, int b, int c) {
    int result = 0;
    result = a + b;
    result *= c;
    result -= a * b;
    return result;
}

/* Secondary file functions - declared here, defined in separate file */
extern void init_global_array(void);
extern int compute_checksum(int iterations);

/* Core computation with nested loops targeting selective scheduling */
__attribute__((noinline)) 
int nested_loop_computation(int outer_limit, int inner_limit, int threshold) {
    int acc = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    unsigned short us_counter;
    int i, j, k;
    
    /* Outer loop with varying data types */
    for (i = 0; i < outer_limit; ++i) {
        int temp = i * i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > threshold, 0)) {
            /* First inner loop with int counter */
            for (j = 0; j < inner_limit; ++j) {
                /* Loop-carried dependency */
                acc = global_array[j] + acc;
                
                /* Mixed operations creating scheduling opportunities */
                int val = noinline_multiply(i, j);
                val += pure_compute(i, j);
                
                /* Memory barrier to create scheduling boundary */
                asm volatile ("" ::: "memory");
                
                /* Conditional branch with modulo */
                if (j % 8 == 0) {
                    reg_acc += mixed_operations(i, j, val);
                } else {
                    reg_acc -= val;
                }
                
                /* Access volatile to prevent optimization */
                if (volatile_seed > 1000) {
                    acc += volatile_seed % 256;
                }
            }
        }
        
        /* Second inner loop with unsigned short counter */
        for (us_counter = 0; us_counter < (unsigned short)(inner_limit / 2); ++us_counter) {
            /* Different data type operations */
            short s_val = (short)us_counter;
            int idx = (i * 16 + us_counter) % 1024;
            
            /* Memory operation with potential aliasing */
            global_array[idx] = global_array[idx] + s_val;
            
            /* Function call with loop-variant arguments */
            int div_result = noinline_divide(acc, us_counter + 1);
            
            /* Another scheduling barrier */
            asm volatile ("" ::: "memory");
            
            /* Complex expression with multiple uses of same variable */
            temp = temp * 3 + div_result;
            temp = temp / 2 - s_val;
            
            /* Update accumulator */
            acc += temp;
        }
        
        /* Third level of nesting for deeper scheduling regions */
        if (i % 3 == 0) {
            for (k = 0; k < 5; ++k) {
                /* Create register pressure */
                int a = i + k;
                int b = i - k;
                int c = i * k;
                int d = a + b + c;
                
                /* Multiple operations on same variable */
                d = noinline_multiply(d, 7);
                d = pure_compute(d, k);
                d = mixed_operations(a, b, d);
                
                reg_acc += d;
                
                /* Conditional with __builtin_expect */
                if (__builtin_expect(k == 2, 1)) {
                    asm volatile ("" ::: "memory");
                    acc += global_array[k] * 2;
                }
            }
        }
    }
    
    return acc + reg_acc;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline))
void warm_up_computation(void) {
    int dummy = 0;
    for (int i = 0; i < 100; ++i) {
        dummy += pure_compute(i, i % 10);
        asm volatile ("" ::: "memory");
    }
    /* Use dummy to prevent optimization */
    volatile_seed = dummy & 1;
}

/* Main computation driver */
int compute_all(int seed) {
    int result = 0;
    
    /* Initialize with seed-dependent bounds */
    int outer = 50 + (seed % 50);
    int inner = 20 + (seed % 30);
    int threshold = 10 + (seed % 20);
    
    /* Execute warm-up */
    warm_up_computation();
    
    /* Main nested loop computation */
    result = nested_loop_computation(outer, inner, threshold);
    
    /* Additional computation with different patterns */
    for (int i = 0; i < outer; i += 2) {
        int local_acc = 0;
        for (int j = 0; j < inner; ++j) {
            /* Switch between operation types */
            if (j % 4 == 0) {
                local_acc += noinline_multiply(i, j);
            } else if (j % 4 == 1) {
                local_acc -= pure_compute(i, j);
            } else if (j % 4 == 2) {
                local_acc ^= mixed_operations(i, j, local_acc);
            } else {
                asm volatile ("" ::: "memory");
                local_acc = local_acc >> 1;
            }
        }
        result += local_acc;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize global data */
    init_global_array();
    volatile_seed = seed;
    
    /* Perform computation */
    int checksum = compute_all(seed);
    
    /* Additional cross-file computation */
    int secondary_checksum = compute_checksum(seed % 100);
    
    /* Final result */
    int final_result = checksum + secondary_checksum;
    
    printf("Selective Scheduling Test Result: %d\n", final_result);
    printf("Checksum: %d, Secondary: %d\n", checksum, secondary_checksum);
    
    return 0;
}
