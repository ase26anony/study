/* sel_sched_test.c - Main test file for selective scheduling dump coverage */

#include <stdio.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int external_array[256];
extern volatile int g_volatile_seed;

/* Non-inlineable functions */
__attribute__((noinline)) int noinline_func(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline, const)) int pure_func(int x) {
    return x * 3 + 7;
}

/* Helper functions in separate compilation unit */
extern void init_external_data(void);
extern int compute_checksum(int *data, int size);

/* Memory barrier */
#define MEMORY_BARRIER() asm volatile("" ::: "memory")

/* Global arrays for memory operations */
int global_arr[1024];
short short_arr[512];
unsigned int uint_arr[256];

/* Core computation with nested loops */
__attribute__((noinline)) 
int nested_loop_computation(int outer_limit, int inner_limit, int threshold) {
    int result = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int stack_acc = 0;
    unsigned int u_acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_var = i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > threshold, 0)) {
            /* Inner loop with different data type */
            for (unsigned short j = 0; j < (unsigned short)inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int temp = loop_var * j;
                temp += pure_func(j);  /* Pure function call */
                
                /* Loop-carried dependency */
                reg_acc = global_arr[(i * 16 + j) % 1024] + reg_acc;
                
                /* Memory operation with potential aliasing */
                short_arr[j % 512] = (short)(temp % 256);
                
                /* Conditional branch */
                if (i % 8 == 0) {
                    result += noinline_func(temp, j);
                    MEMORY_BARRIER();  /* Scheduling boundary */
                }
                
                /* Multiple uses of same variable */
                u_acc = u_acc + (unsigned int)temp;
                u_acc = u_acc ^ (unsigned int)j;
                
                /* Another memory operation */
                uint_arr[j % 256] = u_acc;
            }
            
            /* Scheduling barrier inside conditional block */
            MEMORY_BARRIER();
        }
        
        /* Another inner loop with different structure */
        for (int k = 0; k < (i % 7) + 1; ++k) {
            /* Access external array */
            int ext_val = external_array[(i + k) % 256];
            
            /* Complex expression with mixed types */
            stack_acc += (ext_val * k) / (loop_var + 1);
            
            /* Function call with loop-variant arguments */
            if (k % 3 == 0) {
                result += noinline_func(stack_acc, ext_val);
            }
        }
        
        /* Variable with different scope */
        {
            int scope_var = result & 0xFF;
            global_arr[i % 1024] = scope_var + i;
        }
    }
    
    /* Combine accumulators */
    result = (result + reg_acc + stack_acc) ^ (int)u_acc;
    return result;
}

/* Warm-up function */
__attribute__((noinline))
void warm_up_computation(void) {
    volatile int warm_up = g_volatile_seed;
    int temp = 0;
    
    /* Simple warm-up loop */
    for (int i = 0; i < 100; ++i) {
        temp += i * warm_up;
        if (i % 10 == 0) {
            MEMORY_BARRIER();
        }
    }
    
    /* Prevent optimization */
    if (temp == 0) {
        printf("Warm-up completed\n");
    }
}

/* Main computation driver */
int compute_all(int seed) {
    int checksum = 0;
    
    /* Initialize data with simple LCG */
    for (int i = 0; i < 1024; ++i) {
        global_arr[i] = (seed * 1103515245 + 12345) % 65536;
        seed = global_arr[i];
    }
    
    for (int i = 0; i < 512; ++i) {
        short_arr[i] = (short)(seed % 32768);
        seed = (seed * 1103515245 + 12345) % 65536;
    }
    
    /* Make loop bounds depend on external factors */
    volatile int v1 = seed % 100 + 50;
    volatile int v2 = (seed / 100) % 50 + 20;
    volatile int v3 = (seed / 10000) % 30;
    
    /* Warm-up first */
    warm_up_computation();
    
    /* Main nested loop computation */
    checksum = nested_loop_computation(v1, v2, v3);
    
    /* Additional computation with different patterns */
    for (int i = 0; i < v2; ++i) {
        if (__builtin_expect(i < v3, 1)) {
            for (short s = 0; s < (short)(i + 5); ++s) {
                checksum ^= pure_func(s) * noinline_func(i, s);
                uint_arr[s % 256] = checksum;
            }
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize external data */
    init_external_data();
    
    /* Perform computation */
    int result = compute_all(seed);
    
    /* Compute final checksum */
    int final_checksum = compute_checksum(global_arr, 1024);
    final_checksum ^= result;
    
    printf("Result: %d\n", result);
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
