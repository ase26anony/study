/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int helper_compute(int x, int y);
extern void helper_init(void);
extern volatile int ext_volatile;

/* Global arrays for memory operations with potential aliasing */
int global_arr[1024];
unsigned short global_short_arr[2048];
float global_float_arr[512];

/* Struct for complex memory access patterns */
struct DataPoint {
    int index;
    unsigned value;
    short tag;
    char padding[2];
};

struct DataPoint data_points[256];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_multiply(int a, int b) {
    return a * b;
}

__attribute__((noinline)) unsigned noinline_checksum(unsigned a, unsigned b) {
    return (a ^ b) + (a & b);
}

/* Pure function for const attribute testing */
__attribute__((const)) int pure_transform(int x) {
    return (x * 3 + 7) & 0xFF;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) void warm_up_computation(int limit) {
    register int i;
    int acc = 0;
    
    for (i = 0; i < limit; ++i) {
        /* Mix of operations with different data types */
        acc += (i & 0xFF);
        acc ^= (i << 3);
        
        /* Conditional execution */
        if (__builtin_expect((i % 17) == 0, 0)) {
            acc = noinline_multiply(acc, 2);
        }
        
        /* Memory barrier to create scheduling boundaries */
        asm volatile ("" ::: "memory");
    }
    
    /* Use result to prevent optimization */
    global_arr[0] = acc;
}

/* Core computation with nested loops and varied operations */
__attribute__((noinline)) unsigned long core_computation(int outer_limit, 
                                                         int inner_limit,
                                                         int conditional_limit) {
    unsigned long total = 0;
    int i, j, k;
    unsigned temp;
    short s_temp;
    
    /* Outer loop with varying trip count */
    for (i = 0; i < outer_limit; ++i) {
        int loop_carried = i;  /* Loop-carried dependency */
        
        /* First inner loop - always executed */
        for (j = 0; j < inner_limit; ++j) {
            /* Arithmetic operations with different types */
            temp = (unsigned)i * (unsigned)j;
            s_temp = (short)(i + j);
            
            /* Memory operations with potential aliasing */
            global_arr[(i * 16 + j) & 1023] = temp;
            global_short_arr[(j * 2) & 2047] = s_temp;
            
            /* Loop-carried dependency */
            loop_carried += global_arr[(j * 3) & 1023];
            
            /* Function call with loop-variant arguments */
            total += pure_transform(loop_carried);
            
            /* Conditional branch with builtin expect */
            if (__builtin_expect((j % 13) == 0, 1)) {
                total += noinline_checksum(temp, total);
            }
            
            /* Optimization barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Second inner loop - conditionally executed */
        if (__builtin_expect(i > conditional_limit, 0)) {
            register int reg_acc = 0;  /* register keyword hint */
            
            for (k = 0; k < (i & 15); ++k) {
                /* Complex expression with multiple uses of same variable */
                int idx = (i + k) & 255;
                data_points[idx].value = data_points[idx].index * k;
                reg_acc += data_points[idx].value;
                
                /* Mixed-type computation */
                total += (unsigned long)reg_acc * (unsigned long)data_points[idx].tag;
                
                /* External function call */
                reg_acc = helper_compute(reg_acc, k);
                
                /* Another memory barrier */
                asm volatile ("" ::: "memory");
            }
            
            total += reg_acc;
        }
        
        /* Conditional with unpredictable pattern */
        if ((i % 7) == 0) {
            /* Nested conditional loop */
            for (int m = 0; m < 3; ++m) {
                total ^= global_short_arr[(i + m) & 2047];
                total += noinline_multiply(m, i);
            }
        } else if ((i % 5) == 0) {
            total += global_float_arr[(i / 2) & 511] * 1000;
        }
    }
    
    return total;
}

/* Additional computation with different loop structures */
__attribute__((noinline)) unsigned long secondary_computation(int n) {
    unsigned long result = 0;
    unsigned u_counter;
    short s_counter;
    
    /* Loop with unsigned counter */
    for (u_counter = 0; u_counter < (unsigned)n; u_counter += 2) {
        int inner;
        
        /* Mixed loop counter types */
        for (s_counter = 0; s_counter < 8; s_counter++) {
            /* Access different array types */
            result += global_arr[u_counter & 1023] * s_counter;
            result -= global_short_arr[s_counter * 2];
            
            /* Volatile-like access through external */
            result ^= ext_volatile;
            
            /* Complex conditional */
            if ((u_counter + s_counter) % 11 == 0) {
                result = noinline_checksum(result, u_counter);
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Variable scope test - different lifetime */
        {
            int temp_var = u_counter * 3;
            for (inner = 0; inner < 4; inner++) {
                temp_var = pure_transform(temp_var + inner);
                result += temp_var;
            }
        } /* temp_var goes out of scope here */
    }
    
    return result;
}

/* Initialize data with pseudo-random values */
void initialize_data(void) {
    unsigned seed = 42;  /* Fixed seed for reproducibility */
    int i;
    
    for (i = 0; i < 1024; i++) {
        /* Simple LCG for pseudo-random values */
        seed = seed * 1103515245 + 12345;
        global_arr[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    for (i = 0; i < 2048; i++) {
        seed = seed * 1103515245 + 12345;
        global_short_arr[i] = (short)(seed & 0xFFFF);
    }
    
    for (i = 0; i < 512; i++) {
        seed = seed * 1103515245 + 12345;
        global_float_arr[i] = (float)(seed & 0xFF) / 256.0f;
    }
    
    for (i = 0; i < 256; i++) {
        data_points[i].index = i;
        seed = seed * 1103515245 + 12345;
        data_points[i].value = seed & 0xFFFFFF;
        data_points[i].tag = (short)(seed >> 24);
    }
}

int main(int argc, char *argv[]) {
    unsigned long checksum1, checksum2, final_result;
    
    /* Use command line arguments to make loop bounds non-constant */
    int outer_lim = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_lim = (argc > 2) ? atoi(argv[2]) : 20;
    int cond_lim = (argc > 3) ? atoi(argv[3]) : 10;
    
    /* Initialize external helper */
    helper_init();
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up computation (executed once) */
    printf("Starting warm-up...\n");
    warm_up_computation(100);
    
    /* Main computation with nested loops */
    printf("Starting main computation...\n");
    checksum1 = core_computation(outer_lim, inner_lim, cond_lim);
    
    /* Secondary computation with different patterns */
    printf("Starting secondary computation...\n");
    checksum2 = secondary_computation(outer_lim * 2);
    
    /* Combine results */
    final_result = checksum1 ^ checksum2;
    
    /* Print verifiable result */
    printf("Final checksum: 0x%016lx\n", final_result);
    printf("Checksum1: 0x%016lx\n", checksum1);
    printf("Checksum2: 0x%016lx\n", checksum2);
    
    return 0;
}
