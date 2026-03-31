/* sel-sched-trigger.c - Program to trigger selective scheduling dump logic */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays with different types to create varied RTL */
static int global_arr_int[1024];
static unsigned short global_arr_short[2048];
static volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline, const)) int pure_func(int x) {
    return (x * 3) / 2;
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* arr, int size) {
    int acc = 0;
    for (int i = 0; i < size; ++i) {
        acc = arr[i] + acc;
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            acc += pure_func(i);
        }
    }
    return acc;
}

/* Secondary computation with nested loops */
__attribute__((noinline)) int nested_loop_computation(int N, int M, int K) {
    int result = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < N; ++i) {
        int temp = i * K;
        
        /* Conditional inner loop execution */
        if (i > (N / 4)) {
            /* First inner loop with short counter */
            for (unsigned short j = 0; j < (unsigned short)M; ++j) {
                /* Mix of arithmetic operations */
                int val = global_arr_int[j] * 3;
                val += noinline_func(i, j);
                
                /* Memory operation with potential aliasing */
                global_arr_short[j] = (val & 0xFFFF);
                
                /* Loop-carried dependency */
                reg_acc += val;
                
                /* Optimization barrier */
                asm volatile ("" ::: "memory");
                
                /* Conditional branch inside innermost loop */
                if (__builtin_expect((j % 7) == 0, 1)) {
                    reg_acc -= pure_func(j);
                }
            }
        }
        
        /* Second inner loop with different data type */
        for (unsigned k = 0; k < (unsigned)(M / 2); ++k) {
            /* Complex expression with multiple uses of variables */
            int idx = (i * 31 + k * 17) % 1024;
            int a = global_arr_int[idx];
            int b = global_arr_short[k];
            
            /* Multiple operations on same variable */
            temp = temp * 3 + a;
            temp = temp - b / 2;
            temp = noinline_func(temp, k);
            
            /* Another optimization barrier */
            asm volatile ("" ::: "memory");
            
            result += temp;
        }
        
        /* Variable with different scope */
        {
            int scope_var = result & 0xFF;
            result += scope_var * pure_func(i);
        }
    }
    
    return result + reg_acc;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline)) void warmup_computation(void) {
    int warmup_acc = 0;
    for (int i = 0; i < 100; ++i) {
        warmup_acc += noinline_func(i, i + 1);
        asm volatile ("" ::: "memory");
    }
    /* Use result to prevent optimization */
    volatile_seed = warmup_acc & 1;
}

/* Initialize data with pseudo-random values */
void init_data(void) {
    unsigned lcg = 123456789;
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_arr_int[i] = (int)(lcg % 1000);
    }
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_arr_short[i] = (unsigned short)(lcg % 65535);
    }
}

/* Main computation function */
int core_computation(int outer_iter, int inner_iter, int mod_factor) {
    int total = 0;
    
    /* Triple nested loop structure */
    for (int a = 0; a < outer_iter; ++a) {
        int mid_loop_bound = inner_iter - (a % 3);
        
        for (int b = 0; b < mid_loop_bound; ++b) {
            /* Innermost loop with volatile-dependent bound */
            int inner_bound = (volatile_seed + b) % 8 + 4;
            
            for (int c = 0; c < inner_bound; ++c) {
                /* Complex computation mixing globals and locals */
                int idx = (a * 19 + b * 23 + c * 29) % 1024;
                int val = global_arr_int[idx];
                
                /* Multiple conditional paths */
                if (__builtin_expect((a % mod_factor) == 0, 0)) {
                    val = noinline_func(val, c);
                } else if ((b & 1) == 0) {
                    val = pure_func(val) * 2;
                } else {
                    val = val / 3;
                }
                
                /* Memory store with aliasing possibility */
                global_arr_short[c * 2] = (val & 0xFFFF);
                
                /* Accumulate with loop-carried dependency */
                total = total + val - (c % 2);
                
                /* Frequent optimization barrier */
                if ((c % 3) == 0) {
                    asm volatile ("" ::: "memory");
                }
            }
            
            /* Function call with loop-variant arguments */
            total += pure_func(b) * 2;
        }
        
        /* Periodic complex operation */
        if ((a % 5) == 0) {
            total -= nested_loop_computation(3, 10, 7);
        }
    }
    
    return total;
}

int main(int argc, char** argv) {
    /* Use arguments for loop bounds to prevent constant folding */
    int outer_iter = argc > 1 ? atoi(argv[1]) : 50;
    int inner_iter = argc > 2 ? atoi(argv[2]) : 100;
    int mod_factor = argc > 3 ? atoi(argv[3]) : 13;
    
    if (outer_iter < 10) outer_iter = 10;
    if (inner_iter < 20) inner_iter = 20;
    if (mod_factor < 2) mod_factor = 2;
    
    printf("Starting selective scheduling trigger program\n");
    printf("Parameters: outer=%d, inner=%d, mod=%d\n", 
           outer_iter, inner_iter, mod_factor);
    
    /* Initialize data */
    init_data();
    
    /* Warm-up execution */
    printf("Warm-up...\n");
    warmup_computation();
    
    /* Main computation */
    printf("Main computation...\n");
    int result = core_computation(outer_iter, inner_iter, mod_factor);
    
    /* Additional computation with arrays */
    int checksum = compute_checksum(global_arr_int, 
                      (outer_iter < 1024) ? outer_iter : 1024);
    
    /* Final result */
    int final_result = result + checksum;
    printf("Result: %d (checksum: %d)\n", final_result, checksum);
    
    return final_result != 0 ? 0 : 1;
}
