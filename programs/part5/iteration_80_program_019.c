/* Test program to trigger selective scheduling dump logic */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_arr[1024];
extern short global_short_arr[2048];
volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func1(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline)) unsigned noinline_func2(unsigned a, unsigned b) {
    asm volatile("" ::: "memory");
    return (a << 3) | (b >> 2);
}

/* Pure function for loop computations */
__attribute__((const)) int pure_multiply(int a, int b) {
    return a * b;
}

/* Helper function with mixed operations */
__attribute__((noinline)) int complex_helper(int idx, int mod) {
    int result = idx;
    if (idx % 4 == 0) {
        result += mod * 2;
    } else if (idx % 3 == 0) {
        result -= mod;
    }
    asm volatile("" ::: "memory");
    return result;
}

/* Core computation with nested loops */
__attribute__((noinline)) 
long long compute_checksum(int outer_limit, int inner_limit, 
                          int conditional_limit, int *arr) {
    long long checksum = 0;
    register int reg_acc = 0;
    unsigned u_acc = 0;
    short s_acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_var = i;
        int temp = arr[i % 1024];
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > conditional_limit, 0)) {
            /* First inner loop with int counter */
            for (int j = 0; j < inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int calc = pure_multiply(loop_var, j);
                calc += noinline_func1(loop_var, j);
                
                /* Memory operation with potential aliasing */
                int mem_val = global_arr[(i + j) % 1024];
                calc ^= mem_val;
                
                /* Conditional branch creating control flow */
                if (j % 8 == 0) {
                    calc = complex_helper(calc, i);
                    asm volatile("" ::: "memory");
                }
                
                /* Loop-carried dependency */
                reg_acc = calc + reg_acc;
                
                /* Different data type computation */
                s_acc += global_short_arr[(i * j) % 2048];
                
                checksum += calc;
            }
        } else {
            /* Alternative path with different loop structure */
            for (unsigned k = 0; k < (unsigned)(inner_limit / 2); ++k) {
                unsigned u_calc = noinline_func2(k, i);
                u_acc += u_calc;
                
                /* Memory barrier */
                asm volatile("" ::: "memory");
                
                /* Another loop-carried dependency */
                if (k % 5 == 0) {
                    reg_acc -= arr[(i + k) % 1024];
                }
                
                checksum += u_calc;
            }
        }
        
        /* Second inner loop with short counter */
        for (short s = 0; s < (short)(inner_limit % 256); ++s) {
            int mixed_calc = loop_var * s;
            mixed_calc ^= s_acc;
            
            /* Function call with loop-variant arguments */
            mixed_calc = noinline_func1(mixed_calc, temp);
            
            /* Conditional execution within inner loop */
            if (s % 7 == 0) {
                mixed_calc = complex_helper(mixed_calc, s);
            }
            
            checksum += mixed_calc;
            temp = mixed_calc % 256;
        }
        
        /* Update checksum with accumulated values */
        checksum += reg_acc + u_acc + s_acc;
        
        /* Reset some accumulators periodically */
        if (i % 16 == 0) {
            u_acc = 0;
            asm volatile("" ::: "memory");
        }
    }
    
    return checksum;
}

/* Warm-up function */
__attribute__((noinline))
void warm_up_computation(int limit) {
    int local_arr[64];
    long long warm_sum = 0;
    
    /* Simple warm-up loop */
    for (int i = 0; i < limit; ++i) {
        for (int j = 0; j < 8; ++j) {
            int val = i * j + (i ^ j);
            local_arr[j % 64] = val;
            warm_sum += val;
            
            if (j % 4 == 0) {
                asm volatile("" ::: "memory");
                warm_sum += noinline_func1(i, j);
            }
        }
    }
    
    /* Prevent optimization */
    if (warm_sum == 0) {
        printf("Warm-up completed\n");
    }
}

/* Initialize arrays with pseudo-random values */
void init_arrays(void) {
    int lcg = volatile_seed;
    for (int i = 0; i < 1024; ++i) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        global_arr[i] = lcg % 1000;
    }
    
    lcg = volatile_seed ^ 0x5555;
    for (int i = 0; i < 2048; ++i) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        global_short_arr[i] = (short)(lcg % 10000);
    }
}

int main(int argc, char *argv[]) {
    /* Use arguments to make bounds non-constant */
    int outer_bound = (argc > 1) ? atoi(argv[1]) : 100;
    int inner_bound = (argc > 2) ? atoi(argv[2]) : 50;
    int cond_bound = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Ensure bounds are reasonable */
    if (outer_bound <= 0) outer_bound = 100;
    if (inner_bound <= 0) inner_bound = 50;
    if (cond_bound <= 0) cond_bound = 25;
    
    /* Initialize data */
    init_arrays();
    
    /* Warm-up execution */
    warm_up_computation(10);
    
    /* Main computation */
    long long result = compute_checksum(
        outer_bound, 
        inner_bound, 
        cond_bound,
        global_arr
    );
    
    /* Print verifiable result */
    printf("Computation checksum: %lld\n", result);
    printf("Result mod 1000000: %d\n", (int)(result % 1000000));
    
    return 0;
}
