/* sel_sched_test.c - Test program to trigger selective scheduling dump logic */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_arr[1024];
extern short global_short_arr[2048];
static unsigned int static_arr[512];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline)) short noinline_short_func(short a, short b) {
    return (short)(a + b * 2);
}

/* Pure function for loop computations */
__attribute__((const)) int pure_mult(int a, int b) {
    return a * b;
}

__attribute__((const)) unsigned int pure_sum(unsigned int a, unsigned int b) {
    return a + b;
}

/* External helper functions (defined in another file) */
extern void init_arrays(int seed);
extern int verify_result(int checksum);

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 100;
volatile short g_volatile_short = 50;

/* Main computation with nested loops targeting selective scheduling */
__attribute__((noinline)) 
int compute_nested_loops(int outer_bound, int inner_bound, int limit) {
    int result = 0;
    unsigned int acc = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_bound; ++i) {
        int temp = i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > limit, 0)) {
            /* Inner loop with different data type */
            for (unsigned int j = 0; j < (unsigned int)inner_bound; ++j) {
                /* Mix of arithmetic operations */
                int val = pure_mult(i, j);
                short s_val = noinline_short_func((short)i, (short)j);
                
                /* Loop-carried dependency */
                acc = pure_sum(acc, (unsigned int)val);
                
                /* Memory operation with potential aliasing */
                global_arr[(i * 16 + j) % 1024] = val;
                global_short_arr[(j * 2) % 2048] = s_val;
                
                /* Optimization barrier creating scheduling boundary */
                asm volatile ("" ::: "memory");
                
                /* Conditional branch inside innermost loop */
                if (j % 8 == 0) {
                    temp += noinline_func(val, s_val);
                    reg_acc += temp;
                } else {
                    temp -= val % 7;
                }
                
                /* Multiple uses of same variable */
                result += temp;
                result -= val / 3;
            }
        } else {
            /* Different loop structure when condition not met */
            for (short s = 0; s < g_volatile_short; ++s) {
                /* Different computation pattern */
                int prod = i * s * 3;
                result += prod;
                
                /* Access static array */
                static_arr[s % 512] = prod % 256;
                
                /* Another scheduling boundary */
                asm volatile ("" ::: "memory");
                
                /* Nested conditional */
                if (s % 5 == 0) {
                    result += noinline_func(prod, s);
                }
            }
        }
        
        /* Cross-iteration dependency */
        result += acc % 1024;
        reg_acc = reg_acc * 2 - i;
    }
    
    return result + reg_acc;
}

/* Another computation with different loop patterns */
__attribute__((noinline))
int compute_complex_loops(int n, int m) {
    int total = 0;
    unsigned long long big_acc = 0;
    
    /* Triple nested loop */
    for (int x = 0; x < n; x += 2) {
        int x_squared = x * x;
        
        for (int y = 1; y < m; y += 3) {
            register int reg_var = x_squared + y;  /* Register hint */
            
            for (int z = 0; z < 10; ++z) {
                /* Complex expression with multiple operations */
                int val = (reg_var * z + (x ^ y) - (z % 4)) * 3;
                
                /* Memory access pattern */
                int idx = (x * 31 + y * 7 + z) % 1024;
                global_arr[idx] = val;
                
                /* Conditional with __builtin_expect */
                if (__builtin_expect((val & 1) == 0, 1)) {
                    total += val;
                    big_acc += (unsigned long long)val * z;
                } else {
                    total -= val / 2;
                    big_acc >>= 1;
                }
                
                /* Function call with loop-variant arguments */
                total += pure_mult(val, z);
                
                /* Scheduling barrier */
                if (z % 3 == 0) {
                    asm volatile ("" ::: "memory");
                }
            }
            
            /* Update register variable */
            reg_var = noinline_func(reg_var, y);
            total += reg_var;
        }
        
        /* Access volatile to prevent optimization */
        total += g_volatile_bound % 17;
    }
    
    return total + (int)(big_acc % 0x7FFFFFFF);
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline))
void warm_up_computation(void) {
    int warm_result = 0;
    
    /* Simple warm-up loop */
    for (int i = 0; i < 100; ++i) {
        warm_result += i * i;
        asm volatile ("" ::: "memory");
    }
    
    /* Prevent unused variable warning */
    if (warm_result > 0) {
        printf("Warm-up complete\n");
    }
}

int main(int argc, char *argv[]) {
    /* Use arguments for variability, but provide defaults */
    int outer_bound = (argc > 1) ? atoi(argv[1]) : 73;
    int inner_bound = (argc > 2) ? atoi(argv[2]) : 89;
    int limit = (argc > 3) ? atoi(argv[3]) : 30;
    
    /* Initialize data */
    init_arrays(42);
    
    /* Fill static array */
    for (int i = 0; i < 512; ++i) {
        static_arr[i] = i * 3 + 1;
    }
    
    /* Warm-up execution */
    warm_up_computation();
    
    /* Main computations */
    int result1 = compute_nested_loops(outer_bound, inner_bound, limit);
    printf("Result 1: %d\n", result1);
    
    int result2 = compute_complex_loops(outer_bound / 2 + 5, inner_bound / 3 + 7);
    printf("Result 2: %d\n", result2);
    
    /* Combine results */
    int final_result = result1 + result2 * 3;
    printf("Final result: %d\n", final_result);
    
    /* Verification */
    if (verify_result(final_result)) {
        printf("Verification passed\n");
    } else {
        printf("Verification failed\n");
    }
    
    return 0;
}
