/* sel_sched_test.c - Test program for selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern short global_short_array[2048];
static unsigned int static_array[512];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int compute_value(int a, int b) {
    return (a * b) + (a >> 3) - (b << 2);
}

__attribute__((noinline)) short process_short(short x, short y) {
    return (x + y) * 2 - (x / 4);
}

/* Pure function for loop-invariant computations */
__attribute__((const)) int pure_multiply(int x, int y) {
    return x * y;
}

/* Function with side effects */
__attribute__((noinline)) void update_counter(volatile int* counter) {
    *counter += 1;
}

/* Core computation with nested loops */
__attribute__((noinline)) 
unsigned long long nested_loop_computation(int outer_limit, int inner_limit, 
                                          int conditional_limit) {
    volatile int warmup_counter = 0;
    register int reg_acc = 0;
    unsigned long long total_sum = 0;
    int local_array[64];
    
    /* Initialize local array */
    for (int k = 0; k < 64; ++k) {
        local_array[k] = k * 3 + 1;
    }
    
    /* Warm-up loop - executed once */
    for (int w = 0; w < 1; ++w) {
        int temp = 0;
        for (int i = 0; i < 16; ++i) {
            temp += compute_value(i, w);
            asm volatile ("" ::: "memory");  /* Scheduling barrier */
        }
        warmup_counter = temp;
    }
    
    /* Main nested loop structure */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_carried = i * 2;
        unsigned short us_i = (unsigned short)i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > conditional_limit, 0)) {
            for (int j = 0; j < inner_limit; ++j) {
                /* Mixed data type operations */
                int idx = (i * 32 + j) & 1023;
                short s_idx = (short)((i * 16 + j) & 2047);
                
                /* Loop-carried dependency */
                loop_carried = global_array[idx] + loop_carried;
                
                /* Memory operation with potential aliasing */
                int val = static_array[j & 511] + global_short_array[s_idx];
                
                /* Arithmetic with different types */
                reg_acc += val * (int)us_i;
                reg_acc -= pure_multiply(i, j);
                
                /* Conditional branch inside innermost loop */
                if ((i * j) % 7 == 0) {
                    total_sum += compute_value(reg_acc, val);
                    asm volatile ("" ::: "memory");  /* Scheduling barrier */
                }
                
                /* Function call with side effects */
                update_counter(&warmup_counter);
                
                /* More arithmetic with register variable */
                reg_acc = (reg_acc * 3) / 2;
            }
        } else {
            /* Alternative path with different loop structure */
            for (unsigned int j = 0; j < (unsigned int)inner_limit / 2; ++j) {
                /* Different data type for counter */
                unsigned int u_idx = (i * 64 + j) % 512;
                
                /* Memory access pattern */
                int temp = static_array[u_idx] - local_array[j % 64];
                
                /* Complex expression */
                total_sum += (unsigned long long)(temp * i * j);
                
                /* Conditional with __builtin_expect */
                if (__builtin_expect((i + j) % 11 == 0, 1)) {
                    short s_val = process_short((short)i, (short)j);
                    total_sum += s_val;
                }
            }
        }
        
        /* Outer loop computation */
        total_sum += loop_carried * reg_acc;
        
        /* Variable with different scope */
        {
            int scope_var = i * reg_acc;
            total_sum += scope_var;
        }
    }
    
    return total_sum + warmup_counter;
}

/* Secondary computation with different patterns */
__attribute__((noinline))
int triple_nested_loop(int dim1, int dim2, int dim3) {
    int result = 0;
    volatile int barrier = 0;
    
    for (int i = 0; i < dim1; ++i) {
        register int r1 = i;
        for (int j = 0; j < dim2; ++j) {
            int r2 = j * 2;
            for (int k = 0; k < dim3; ++k) {
                /* Complex expression with multiple operations */
                int temp = (r1 * r2 + k) * 3;
                temp -= (r1 / (k + 1)) * 2;
                temp += global_array[(i + j + k) & 1023];
                
                /* Memory barrier */
                asm volatile ("" ::: "memory");
                
                /* Conditional with function call */
                if (temp % 5 == 0) {
                    result += compute_value(temp, i + j);
                } else {
                    result -= pure_multiply(temp, k);
                }
                
                /* Update register variable */
                r2 = (r2 + temp) & 0xFFF;
            }
            
            /* Loop-carried update */
            r1 = (r1 + r2) % 100;
            barrier = r1;  /* Prevent optimization */
        }
    }
    
    return result;
}

/* Initialize arrays with pseudo-random values */
void initialize_data(void) {
    unsigned int seed = 42;  /* Simple LCG */
    for (int i = 0; i < 1024; ++i) {
        seed = seed * 1103515245 + 12345;
        global_array[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    for (int i = 0; i < 2048; ++i) {
        seed = seed * 1103515245 + 12345;
        global_short_array[i] = (short)(seed & 0xFFFF);
    }
    
    for (int i = 0; i < 512; ++i) {
        seed = seed * 1103515245 + 12345;
        static_array[i] = (unsigned int)seed;
    }
}

int main(int argc, char* argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int outer_lim = argc > 1 ? atoi(argv[1]) : 50;
    int inner_lim = argc > 2 ? atoi(argv[2]) : 100;
    int cond_lim = argc > 3 ? atoi(argv[3]) : 25;
    
    if (outer_lim <= 0) outer_lim = 50;
    if (inner_lim <= 0) inner_lim = 100;
    if (cond_lim <= 0) cond_lim = 25;
    
    /* Initialize data */
    initialize_data();
    
    /* Perform computations */
    unsigned long long result1 = nested_loop_computation(outer_lim, inner_lim, cond_lim);
    int result2 = triple_nested_loop(outer_lim / 2, inner_lim / 4, 10);
    
    /* Final checksum */
    unsigned long long final_checksum = result1 + result2;
    
    printf("Computation result: %llu\n", final_checksum);
    printf("Result1: %llu, Result2: %d\n", result1, result2);
    
    return 0;
}
