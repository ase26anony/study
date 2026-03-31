/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External declarations for multi-file compilation stress */
extern int global_array[1024];
extern volatile int g_volatile_seed;
extern int helper_compute(int a, int b) __attribute__((noinline));

/* Non-inlineable functions to create scheduling boundaries */
int __attribute__((noinline)) noinline_multiply(int a, int b) {
    return a * b;
}

int __attribute__((noinline)) noinline_modulo(int a, int b) {
    return a % b;
}

/* Pure function for varied RTL patterns */
int __attribute__((const)) pure_transform(int x) {
    return (x * 3) + 7;
}

/* Global variables for aliasing stress */
int global_data[256];
int global_counter = 0;
volatile int volatile_bound = 100;

/* Struct for complex memory access patterns */
struct DataBlock {
    int values[16];
    short shorts[32];
    unsigned char bytes[64];
};

/* Main computation with nested loops */
unsigned long long __attribute__((noinline)) 
compute_checksum(int outer_limit, int inner_limit, int step) {
    struct DataBlock blocks[4];
    int local_array[128];
    unsigned long long checksum = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    
    /* Initialize local data */
    for (int i = 0; i < 128; i++) {
        local_array[i] = i * 3 + 1;
    }
    
    /* Warm-up loop (executed once) */
    for (int warm = 0; warm < 1; ++warm) {
        int temp = 0;
        for (int i = 0; i < 16; ++i) {
            temp += blocks[warm % 4].values[i];
        }
        checksum += temp;
    }
    
    /* Main nested loop structure with varying trip counts */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_variant = i * step;
        short short_counter = (short)i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > outer_limit / 3, 0)) {
            /* Inner loop with different data types */
            for (unsigned j = 0; j < (unsigned)inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int idx = (i * 31 + j * 17) % 128;
                int val = local_array[idx] + loop_variant;
                
                /* Loop-carried dependency */
                reg_acc = val + reg_acc;
                
                /* Memory operation with potential aliasing */
                global_data[j % 256] = val;
                
                /* Conditional branch creating control flow */
                if (i % 4 == 0) {
                    /* Function call with loop-variant arguments */
                    int transformed = pure_transform(j);
                    checksum += transformed;
                    
                    /* Non-inlineable function call */
                    checksum += noinline_modulo(val, 7);
                }
                
                /* Optimization barrier creating scheduling boundary */
                asm volatile("" ::: "memory");
                
                /* Access struct with different element sizes */
                blocks[i % 4].shorts[j % 32] = (short)checksum;
                blocks[j % 4].bytes[idx % 64] = (unsigned char)val;
                
                /* External array access */
                checksum += global_array[(i + j) % 1024];
            }
        }
        
        /* Second inner loop with different characteristics */
        for (int j = 0; j < inner_limit / 2; ++j) {
            /* Different computation pattern */
            unsigned uval = (unsigned)i * (unsigned)j;
            
            /* Multiple uses of same variable */
            int temp = uval % 256;
            temp = temp * 3 - 1;
            temp = noinline_multiply(temp, 2);
            
            /* Helper function from another translation unit */
            temp = helper_compute(temp, j);
            
            /* Update checksum with complex expression */
            checksum += (unsigned long long)temp * (checksum & 0xFF);
            
            /* Another optimization barrier */
            asm volatile("" ::: "memory");
            
            /* Conditional execution based on multiple factors */
            if ((i ^ j) & 0x3) {
                checksum += blocks[0].values[j % 16];
            }
        }
        
        /* Update global volatile to prevent optimization */
        if (i % 8 == 0) {
            global_counter = reg_acc;
        }
    }
    
    /* Final reduction loop */
    for (int i = 0; i < 256; i += 4) {
        checksum += global_data[i];
    }
    
    return checksum + reg_acc;
}

/* Another computation function with different loop structure */
int __attribute__((noinline)) 
alternate_computation(int limit, int *results) {
    int sum = 0;
    unsigned short us_counter = 0;
    
    /* Loop with mixed-width computations */
    for (int i = 0; i < limit; ++i) {
        /* Type conversions creating varied RTL */
        long long wide_val = (long long)i * i;
        int narrow_val = (int)(wide_val >> 4);
        
        /* Store with different strides */
        results[i % 64] = narrow_val;
        
        /* Update short counter with wrap-around */
        us_counter = (us_counter + 1) & 0x7FFF;
        
        /* Complex condition with builtin expect */
        if (__builtin_expect((i & 0xF) == 0, 1)) {
            sum += noinline_multiply(narrow_val, us_counter);
        } else {
            sum += pure_transform(narrow_val);
        }
        
        /* Memory barrier every 16 iterations */
        if ((i & 0xF) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use arguments for variability */
    int outer_limit = argc > 1 ? atoi(argv[1]) : 50;
    int inner_limit = argc > 2 ? atoi(argv[2]) : 100;
    int step = argc > 3 ? atoi(argv[3]) : 3;
    
    /* Initialize global data with pseudo-random values */
    unsigned int lcg_state = 123456789;
    for (int i = 0; i < 1024; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        global_array[i] = (int)(lcg_state >> 16) & 0x7FFF;
    }
    
    for (int i = 0; i < 256; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        global_data[i] = (int)(lcg_state & 0xFF);
    }
    
    /* Volatile variable to prevent optimization */
    g_volatile_seed = outer_limit;
    
    /* Results array for alternate computation */
    int results[64];
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: outer=%d, inner=%d, step=%d\n", 
           outer_limit, inner_limit, step);
    
    /* Perform computations */
    unsigned long long checksum1 = compute_checksum(outer_limit, inner_limit, step);
    int checksum2 = alternate_computation(inner_limit * 2, results);
    
    /* Final result */
    unsigned long long final_result = checksum1 + checksum2;
    
    printf("Checksum 1: %llu\n", checksum1);
    printf("Checksum 2: %d\n", checksum2);
    printf("Final result: %llu\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
